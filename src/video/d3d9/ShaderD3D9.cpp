#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "ShaderD3D9.h"
#include "video/d3d9/DeviceStateD3D9.h"

#include "core/Logger.h"
#include "core/SafeCast.h"

#include "video/VideoDriver.h"
#include "video/Renderer.h"

#include "io/FileSystem.h"
#include "io/File.h"
#include "video/MaterialLibrary.h"

#include "video/d3d9/D3DHelper.h"
#include "platform/D3D9Exception.h"

namespace lux
{
namespace video
{

class luxD3DXShaderIncludes : public ID3DXInclude
{
public:
	luxD3DXShaderIncludes()
	{
	}

	virtual ~luxD3DXShaderIncludes()
	{
	}

	STDMETHOD(Open)(THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
	{
		if(!ppData || !pBytes)
			return E_FAIL;

		if(IncludeType == D3DXINC_LOCAL) {
			if(io::FileSystem::Instance()->ExistFile(pFileName)) {
				auto file = io::FileSystem::Instance()->OpenFile(pFileName);
				*pBytes = core::SafeCast<UINT>(file->GetSize());
				void* data = LUX_NEW_RAW(*pBytes);
				m_Allocated.PushBack(data);
				if(file->ReadBinaryPart(*pBytes, data) != *pBytes)
					return E_FAIL;
				*ppData = data;
				return S_OK;
			} else {
				return Open(D3DXINC_SYSTEM, pFileName, pParentData, ppData, pBytes);
			}
		} else if(IncludeType == D3DXINC_SYSTEM) {
			const void* data;
			int bytes;
			if(MaterialLibrary::Instance()->GetShaderInclude(EShaderLanguage::HLSL, pFileName, data, bytes)) {
				*ppData = data;
				*pBytes = (UINT)bytes;
				return S_OK;
			} else {
				return E_FAIL;
			}
		} else {
			return E_FAIL;
		}
	}

	STDMETHOD(Close)(THIS_ LPCVOID pData)
	{
		auto it = core::LinearSearch(pData, m_Allocated);
		if(it != m_Allocated.End()) {
			LUX_FREE_RAW(*it);
			m_Allocated.Erase(it);
		}

		return S_OK;
	}
private:
	core::Array<const void*> m_Allocated;
};

static luxD3DXShaderIncludes g_luxD3DXShaderIncludes;

static core::String FormatD3DXShaderError(const core::String& input, bool isVertex)
{
	// File(line, col): error number: error-string
	auto colon = input.FindReverse(":");
	colon = input.FindReverse(":", input.First(), colon);
	auto base_name = input.FindReverse("\\", input.First(), colon);
	if(base_name == colon)
		base_name = input.First();
	else
		base_name++;

	return (isVertex ? "vs: " : "ps: ") + input.SubString(base_name, input.End());
}

ShaderD3D9::ShaderD3D9(VideoDriver* driver, DeviceStateD3D9& state) :
	m_D3DDevice((IDirect3DDevice9*)driver->GetLowLevelDevice()),
	m_Renderer(driver->GetRenderer()),
	m_DeviceState(state)
{
}

ShaderD3D9::~ShaderD3D9()
{
}

void ShaderD3D9::Init(
	const char* vsCode, const char* vsEntryPoint, int vsLength, const char* vsProfile,
	const char* psCode, const char* psEntryPoint, int psLength, const char* psProfile,
	core::Array<core::String>* errorList)
{
	LX_CHECK_NULL_ARG(vsCode);
	LX_CHECK_NULL_ARG(psCode);
	LX_CHECK_NULL_ARG(vsProfile);
	LX_CHECK_NULL_ARG(psProfile);

	if(vsLength == 0)
		vsLength = (int)strlen(vsCode);
	if(psLength == 0)
		psLength = (int)strlen(psCode);

	if(!vsEntryPoint)
		vsEntryPoint = "mainVS";
	if(!psEntryPoint)
		psEntryPoint = "mainPS";

	UnknownRefCounted<ID3DXConstantTable> vertexShaderConstants;
	UnknownRefCounted<ID3DXConstantTable> pixelShaderConstants;

	bool compileError = false;
	m_VertexShader = CreateVertexShader(vsCode, vsEntryPoint, vsLength, vsProfile, errorList, vertexShaderConstants);
	if(!m_VertexShader)
		compileError = true;

	if(psCode) {
		m_PixelShader = CreatePixelShader(psCode, psEntryPoint, psLength, psProfile, errorList, pixelShaderConstants);
		if(!m_PixelShader)
			compileError = true;
	}

	if(compileError)
		throw video::ShaderCompileException();

	// Load all shader parameters.
	core::Array<HelperEntry> helper;
	u32 nameMemoryNeeded = 0;
	LoadAllParams(true, vertexShaderConstants, helper, nameMemoryNeeded, errorList);
	LoadAllParams(false, pixelShaderConstants, helper, nameMemoryNeeded, errorList);

	u32 materialParamCount = (u32)helper.Size();
	// Param ids are saved as 16-Bit integer
	lxAssert(materialParamCount <= 0xFFFF);

	u32 nameCursor = 0;
	m_Params.Reserve(helper.Size());
	m_Names.SetMinSize(nameMemoryNeeded);
	u32 paramId = 0;
	for(u32 i = 0; i < materialParamCount; ++i) {
		Param entry;
		const HelperEntry& h = helper[i];
		switch(h.paramType) {
		case ParamType_DefaultMaterial:
		case ParamType_ParamMaterial:
		{
			int defId = GetDefaultId(h.name);
			if(defId >= 0) {
				if(GetDefaultType((u32)defId) != h.type) {
					if(errorList)
						errorList->PushBack(core::StringConverter::Format("Warning: Wrong type for default material param in shader: ~s.", h.name));
					continue;
				}
				entry.index = (u32)defId;
				entry.paramType = ParamType_DefaultMaterial;
			} else {
				u8 tempMemory[sizeof(float) * 16]; // Matrix is the biggest type.
				if(h.defaultValue)
					CastShaderToType(h.type, h.defaultValue, tempMemory);
				else
					memset(tempMemory, 0, sizeof(tempMemory)); // Set's integers and float to zero.
				entry.index = DefaultParam_COUNT + m_ParamPackage.AddParam(GetCoreType(h.type), h.name, tempMemory);
				entry.paramType = ParamType_ParamMaterial;
			}

			++paramId;
		}
		break;
		case ParamType_Scene:
		{
			core::AttributePtr ptr;
			bool jumpToNext = false;
			ptr = m_Renderer->GetParam(h.name);
			if(!ptr) {
				if(errorList)
					errorList->PushBack(core::StringConverter::Format("Warning: Unknown scene value in shader: ~s.", h.name));
				jumpToNext = true;
			}

			if(jumpToNext)
				continue;

			if(GetCoreType(h.type) != ptr->GetType()) {
				if(errorList)
					errorList->PushBack(core::StringConverter::Format("Warning: Incompatible scene value type in shader: ~s.", h.name));
				continue;
			}

			entry.sceneValue = ptr;
			entry.paramType = ParamType_Scene;
		}
		break;
		}

		// Put name into namelist.
		memcpy((char*)m_Names + nameCursor, helper[i].name, helper[i].nameLength + 1);

		entry.registerPS = h.registerPS;
		entry.registerPSCount = h.registerPSCount;
		entry.registerVS = h.registerVS;
		entry.registerVSCount = h.registerVSCount;
		entry.samplerStage = h.samplerStage;
		entry.type = h.type;
		if(entry.paramType == ParamType_Scene)
			m_SceneValues.PushBack(std::move(entry));
		else
			m_Params.PushBack(std::move(entry));

		nameCursor += h.nameLength + 1;
	}
}

void ShaderD3D9::LoadAllParams(bool isVertex, ID3DXConstantTable* table, core::Array<HelperEntry>& outParams, u32& outStringSize, core::Array<core::String>* errorList)
{
	D3DXCONSTANTTABLE_DESC tableDesc;
	HRESULT hr;
	hr = table->GetDesc(&tableDesc);
	if(FAILED(hr))
		throw core::D3D9Exception(hr);

	for(UINT i = 0; i < tableDesc.Constants; ++i) {
		D3DXHANDLE handle = table->GetConstant(NULL, i);
		if(handle == NULL)
			continue;

		u32 size, regId, regCount, samplerStage;
		EType type;
		const char* name;
		const void* defaultValue;
		bool isValidType;
		if(!GetStructureElemType(handle, table, samplerStage, type, size, regId, regCount, name, defaultValue, isValidType))
			continue;

		bool isParam = false;
		bool isScene = false;
		if(strncmp(name, "param_", 6) == 0) {
			name += 6;
			isParam = true;
		} else if(strncmp(name, "scene_", 6) == 0) {
			name += 6;
			isScene = true;
		}

		if((isParam || isScene) && !isValidType) {
			if(errorList)
				errorList->PushBack(core::StringConverter::Format("Shader has unsupported parameter type. (param: ~s).", name));
			throw ShaderCompileException();
		}

		if(!isParam && !isScene)
			continue;

		HelperEntry* foundEntry = nullptr;
		for(auto it = outParams.First(); it != outParams.End(); ++it) {
			bool isSameStruct =
				((it->paramType == ParamType_DefaultMaterial || it->paramType == ParamType_ParamMaterial) && isParam) ||
				(it->paramType == ParamType_Scene && !isParam);
			if(isSameStruct) {
				if(strcmp(it->name, name) == 0) {
					foundEntry = &*it;
					break;
				}
			}
		}

		if(foundEntry) {
			// If entry already available just update it's data
			if(isVertex) {
				foundEntry->registerVS = regId;
				foundEntry->registerVSCount = regCount;
			} else {
				foundEntry->registerPS = regId;
				foundEntry->registerPSCount = regCount;
			}
			foundEntry->samplerStage = samplerStage;
			if(foundEntry->type != type) {
				if(errorList)
					errorList->PushBack(core::StringConverter::Format("Shader param in pixelshader and vertex shader has diffrent types. (param: ~s).", name));
				throw ShaderCompileException();
			}
		} else {
			// Otherwise, create a new entry.
			HelperEntry HEntry;
			HEntry.nameLength = (u32)strlen(name);
			HEntry.name = name;
			HEntry.type = type;
			HEntry.typeSize = (u8)size;
			HEntry.defaultValue = defaultValue;
			HEntry.samplerStage = samplerStage;
			if(isVertex) {
				HEntry.registerVS = regId;
				HEntry.registerVSCount = regCount;
			} else {
				HEntry.registerPS = regId;
				HEntry.registerPSCount = regCount;
			}
			HEntry.paramType = isParam ? ParamType_ParamMaterial : ParamType_Scene;
			outParams.PushBack(HEntry);
			outStringSize += HEntry.nameLength + 1;
		}
	}
}

UnknownRefCounted<IDirect3DVertexShader9> ShaderD3D9::CreateVertexShader(
	const char* code, const char* entryPoint, int length, const char* profile,
	core::Array<core::String>* errorList,
	UnknownRefCounted<ID3DXConstantTable>& outTable)
{
	UnknownRefCounted<ID3DXBuffer> output;
	UnknownRefCounted<ID3DXBuffer> errors;
	UnknownRefCounted<IDirect3DVertexShader9> shader;

	HRESULT hr = D3DXCompileShader(code, (UINT)length,
		NULL, &g_luxD3DXShaderIncludes, entryPoint,
		profile,
		0, output.Access(), errors.Access(),
		outTable.Access());
	if(errors) {
		if(errorList) {
			core::String err = (const char*)errors->GetBufferPointer();
			for(auto& str : err.Split('\n', true))
				if(!str.IsWhitespace())
					errorList->PushBack(FormatD3DXShaderError(str, true));
		}
	}

	if(SUCCEEDED(hr)) {
		hr = m_D3DDevice->CreateVertexShader((DWORD*)output->GetBufferPointer(), shader.Access());
		if(FAILED(hr))
			throw core::D3D9Exception(hr);
	}

	return shader;
}

UnknownRefCounted<IDirect3DPixelShader9>  ShaderD3D9::CreatePixelShader(
	const char* code, const char* entryPoint, int length, const char* profile,
	core::Array<core::String>* errorList,
	UnknownRefCounted<ID3DXConstantTable>& outTable)
{
	UnknownRefCounted<ID3DXBuffer> output;
	UnknownRefCounted<ID3DXBuffer> errors;
	UnknownRefCounted<IDirect3DPixelShader9> shader;

	HRESULT hr;
	hr = D3DXCompileShader(code, (UINT)length,
		NULL, &g_luxD3DXShaderIncludes, entryPoint,
		profile,
		0, output.Access(), errors.Access(),
		outTable.Access());

	if(errors) {
		if(errorList) {
			core::String err = (const char*)errors->GetBufferPointer();
			for(auto& str : err.Split('\n', true))
				if(!str.IsWhitespace())
					errorList->PushBack(FormatD3DXShaderError(str, false));
		}
	}

	if(SUCCEEDED(hr)) {
		DWORD* data = (DWORD*)output->GetBufferPointer();
		hr = m_D3DDevice->CreatePixelShader(data, shader.Access());
		if(FAILED(hr))
			throw core::D3D9Exception(hr);
	}

	return shader;
}

int ShaderD3D9::GetSceneParamCount() const
{
	return m_SceneValues.Size();
}

core::AttributePtr ShaderD3D9::GetSceneParam(int id) const
{
	return m_SceneValues.At(id).sceneValue;
}

void ShaderD3D9::Enable()
{
	HRESULT hr;

	if(FAILED(hr = m_D3DDevice->SetVertexShader(m_VertexShader)))
		throw core::D3D9Exception(hr);

	if(FAILED(hr = m_D3DDevice->SetPixelShader(m_PixelShader)))
		throw core::D3D9Exception(hr);
}

void ShaderD3D9::SetParam(int paramId, const void* data)
{
	int realId = -1;
	for(auto it = m_Params.First(); it != m_Params.End(); ++it) {
		++realId;
		if(it->index == paramId + DefaultParam_COUNT)
			break;
	}

	if(realId < 0)
		throw core::ObjectNotFoundException("paramId");

	SetShaderValue(m_Params[realId], data);
}

int ShaderD3D9::GetParamId(const core::String& name) const
{
	return m_ParamPackage.GetParamId(name);
}

void ShaderD3D9::LoadSceneParams(const Pass& pass)
{
	for(auto it = m_SceneValues.First(); it != m_SceneValues.End(); ++it) {
		if(it->sceneValue)
			SetShaderValue(*it, (*it->sceneValue).Pointer());
	}

	for(auto it = m_Params.First(); it != m_Params.End(); ++it) {
		if(it->paramType == ParamType_DefaultMaterial) {
			float f;
			video::ColorF c;
			switch(it->index) {
			case DefaultParam_Shininess:
				f = pass.shininess;
				SetShaderValue(*it, &f); break;
			case DefaultParam_Ambient:
				f = pass.ambient;
				SetShaderValue(*it, &f); break;
			case DefaultParam_Diffuse:
				c = pass.diffuse;
				SetShaderValue(*it, &c); break;
			case DefaultParam_Emissive:
				c = pass.emissive;
				SetShaderValue(*it, &c); break;
			case DefaultParam_Specular:
				c = pass.specular;
				SetShaderValue(*it, &c); break;
			default: continue;
			}
		}
	}
}

void ShaderD3D9::Disable()
{
	m_D3DDevice->SetVertexShader(NULL);
	m_D3DDevice->SetPixelShader(NULL);
}

bool ShaderD3D9::GetStructureElemType(D3DXHANDLE handle, ID3DXConstantTable* table, u32& samplerStage, EType& outType, u32& outSize, u32& registerID, u32& regCount, const char*& name, const void*& defaultValue, bool& isValid)
{
	D3DXCONSTANT_DESC desc;
	UINT count = 1;
	if(FAILED(table->GetConstantDesc(handle, &desc, &count)))
		return false;

	outType = EType::Unknown;
	if(desc.Class == D3DXPC_SCALAR) {
		if(desc.Type == D3DXPT_BOOL)
			outType = EType::Boolean;
		else if(desc.Type == D3DXPT_INT)
			outType = EType::Integer;
		else if(desc.Type == D3DXPT_FLOAT)
			outType = EType::Float;
	} else if(desc.Class == D3DXPC_VECTOR) {
		if(desc.Type == D3DXPT_FLOAT) {
			if(desc.Columns == 2)
				outType = EType::Vector2;
			else if(desc.Columns == 3)
				outType = EType::Vector3;
			else if(desc.Columns == 4)
				outType = EType::ColorF;
		}
	} else if(desc.Class == D3DXPC_MATRIX_ROWS && desc.Rows == 4 && desc.Columns == 4) {
		if(desc.Type == D3DXPT_FLOAT)
			outType = EType::Matrix;
	} else if(desc.Class == D3DXPC_MATRIX_COLUMNS && desc.Rows == 4 && desc.Columns == 4) {
		if(desc.Type == D3DXPT_FLOAT)
			outType = EType::Matrix_ColMajor;
	} else if(desc.Class == D3DXPC_OBJECT) {
		if(desc.Type == D3DXPT_SAMPLER || desc.Type == D3DXPT_SAMPLER2D || desc.Type == D3DXPT_SAMPLER3D || desc.Type == D3DXPT_SAMPLERCUBE) {
			samplerStage = table->GetSamplerIndex(handle);
			outType = EType::Texture;
		}
	}

	outSize = desc.Bytes;
	registerID = desc.RegisterIndex;
	regCount = desc.RegisterCount;
	name = desc.Name;
	defaultValue = desc.DefaultValue;

	isValid = (outType != EType::Unknown);
	return true;
}

void ShaderD3D9::CastTypeToShader(EType type, const void* in, void* out)
{
	switch(type) {
	case EType::Boolean:
		*(u32*)out = *(bool*)in;
		break;
	case EType::Texture:
		break;
	case EType::Integer:
		*(u32*)out = *(int*)in;
		break;
	case EType::ColorF:    ((float*)out)[3] = ((float*)in)[3];
	case EType::Vector3:      ((float*)out)[2] = ((float*)in)[2];
	case EType::Vector2:      ((float*)out)[1] = ((float*)in)[1];
	case EType::Float:     ((float*)out)[0] = ((float*)in)[0];
		break;
	case EType::Matrix:
		memcpy(out, in, 16 * sizeof(float));
		break;
	case EType::Matrix_ColMajor:
	{
		float* pf = (float*)in;
		float* f = (float*)out;
		f[0] = pf[0];    f[1] = pf[4];   f[2] = pf[8];   f[3] = pf[12];
		f[4] = pf[1];    f[5] = pf[5];    f[6] = pf[9];   f[7] = pf[13];
		f[8] = pf[2];   f[9] = pf[6];    f[10] = pf[10];    f[11] = pf[14];
		f[12] = pf[3];    f[13] = pf[7];  f[14] = pf[11]; f[15] = pf[15];
	}
	break;
	default:
		lxAssertNeverReach("Unsupported shader variable type.");
	}
}

void ShaderD3D9::CastShaderToType(EType type, const void* in, void* out)
{
	switch(type) {
	case EType::Boolean:
		((bool*)out)[0] = ((*(BOOL*)in) == TRUE);
		break;
	case EType::Integer:
		((int*)out)[0] = *(int*)in;
		break;
	case EType::ColorF:
		((float*)out)[3] = ((float*)in)[3];
	case EType::Vector3:
		((float*)out)[2] = ((float*)in)[2];
	case EType::Vector2:
		((float*)out)[1] = ((float*)in)[1];
	case EType::Float:
		((float*)out)[0] = ((float*)in)[0];
		break;
	case EType::Matrix:
		memcpy(out, in, 16 * sizeof(float));
		break;
	case EType::Matrix_ColMajor:
	{
		float* pf = (float*)in;
		float* f = (float*)out;
		f[0] = pf[0];    f[1] = pf[4];   f[2] = pf[8];   f[3] = pf[12];
		f[4] = pf[1];    f[5] = pf[5];    f[6] = pf[9];   f[7] = pf[13];
		f[8] = pf[2];   f[9] = pf[6];    f[10] = pf[10];    f[11] = pf[14];
		f[12] = pf[3];    f[13] = pf[7];  f[14] = pf[11]; f[15] = pf[15];
	}
	break;
	case EType::Texture:
		break;
	default:
		lxAssertNeverReach("Unsupported shader variable type.");
	}
}

void ShaderD3D9::SetShaderValue(const Param& param, const void* data)
{
	if((param.registerVS == 0xFFFFFFFF && param.registerPS == 0xFFFFFFFF) || param.type == EType::Unknown)
		return;

	static u32 v[16];

	if(param.type != EType::Texture)
		CastTypeToShader(param.type, data, v);

	u32 regId;
	HRESULT hr;
	if(param.registerVS != 0xFFFFFFFF) {
		regId = param.registerVS;
		switch(param.type) {
		case EType::Boolean:
			hr = m_D3DDevice->SetVertexShaderConstantB(regId, (BOOL*)v, 1);
			break;
		case EType::Texture:
		{
			u32 sampler = D3DVERTEXTEXTURESAMPLER0 + param.samplerStage;
			m_DeviceState.EnableTextureLayer(sampler, *(const video::TextureLayer*)data);
		}
		break;
		case EType::Integer:
			hr = m_D3DDevice->SetVertexShaderConstantI(regId, (int*)v, 1);
			break;
		case EType::Float:
		case EType::Vector2:
		case EType::Vector3:
		case EType::ColorF:
			hr = m_D3DDevice->SetVertexShaderConstantF(regId, (float*)v, 1);
			break;
		case EType::Matrix:
		case EType::Matrix_ColMajor:
			hr = m_D3DDevice->SetVertexShaderConstantF(regId, (float*)v, param.registerVSCount);
			break;

		default:
			lxAssertNeverReach("Unsupported shader variable type.");
		}
	}
	if(param.registerPS != 0xFFFFFFFF) {
		regId = param.registerPS;
		switch(param.type) {
		case EType::Boolean:
			hr = m_D3DDevice->SetPixelShaderConstantB(regId, (BOOL*)v, 1);
			break;
		case EType::Texture:
			m_DeviceState.EnableTextureLayer(param.samplerStage, *(const video::TextureLayer*)data);
			break;
		case EType::Integer:
			hr = m_D3DDevice->SetPixelShaderConstantI(regId, (int*)v, 1);
			break;
		case EType::Float:
		case EType::Vector2:
		case EType::Vector3:
		case EType::ColorF:
			hr = m_D3DDevice->SetPixelShaderConstantF(regId, (float*)v, 1);
			break;

		case EType::Matrix:
		case EType::Matrix_ColMajor:
			hr = m_D3DDevice->SetPixelShaderConstantF(regId, (float*)v, param.registerPSCount);
			break;

		default:
			lxAssertNeverReach("Unsupported shader variable type.");
		}
	}
}

const core::ParamPackage& ShaderD3D9::GetParamPackage() const
{
	return m_ParamPackage;
}

int ShaderD3D9::GetDefaultId(const char* name)
{
	static const char* NAMES[DefaultParam_COUNT] = {
		"shininess",
		"diffuse",
		"emissive",
		"specular",
		"ambient"
	};

	for(int i = 0; i < sizeof(NAMES) / sizeof(*NAMES); ++i) {
		if(strcmp(NAMES[i], name) == 0)
			return (int)i;
	}

	return -1;
}

ShaderD3D9::EType ShaderD3D9::GetDefaultType(u32 id)
{
	switch(id) {
	case DefaultParam_Shininess:
	case DefaultParam_Ambient:
	case DefaultParam_Diffuse:
	case DefaultParam_Emissive:
	case DefaultParam_Specular:
		return EType::ColorF;
	default:
		return EType::Unknown;
	}
}

bool ShaderD3D9::IsTypeCompatible(EType a, EType b)
{
	if(a == b)
		return true;
	if((a == EType::Matrix_ColMajor && b == EType::Matrix) || (a == EType::Matrix && b == EType::Matrix_ColMajor))
		return true;

	return false;
}

core::Type ShaderD3D9::GetCoreType(EType type)
{
	switch(type) {
	case EType::Unknown: return core::Type::Unknown;
	case EType::Integer: return core::Types::Integer();
	case EType::Float: return core::Types::Float();
	case EType::Boolean: return core::Types::Boolean();
	case EType::U32: return core::Types::UInteger();
	case EType::Texture: return core::Types::Texture();
	case EType::Color: return core::Types::Color();
	case EType::ColorF: return core::Types::ColorF();
	case EType::Vector2: return core::Types::Vector2F();
	case EType::Vector3: return core::Types::Vector3F();
	case EType::Vector2Int: return core::Types::Vector2I();
	case EType::Vector3Int: return core::Types::Vector3I();
	case EType::Matrix: return core::Types::Matrix();
	case EType::Matrix_ColMajor: return core::Types::Matrix();
	case EType::Structure: return core::Types::Unknown();
	default: return core::Type::Unknown;
	}
}

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
