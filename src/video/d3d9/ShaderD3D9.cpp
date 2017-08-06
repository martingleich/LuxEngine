#ifdef LUX_COMPILE_WITH_D3D9
#include "ShaderD3D9.h"

#include "core/Logger.h"
#include "video/VideoDriver.h"
#include "video/Renderer.h"
#include "video/Material.h"
#include "video/RenderSettings.h"
#include "video/MaterialRenderer.h"

#include "video/d3d9/D3DHelper.h"
#include "video/d3d9/D3D9Exception.h"

namespace lux
{
namespace video
{

ShaderD3D9::ShaderD3D9(VideoDriver* driver) :
	m_D3DDevice((IDirect3DDevice9*)driver->GetLowLevelDevice()),
	m_Renderer(driver->GetRenderer())
{
}

void ShaderD3D9::Init(
	const char* vsCode, const char* vsEntryPoint, size_t vsLength, const char* vsProfile,
	const char* psCode, const char* psEntryPoint, size_t psLength, const char* psProfile,
	core::Array<String>* errorList)
{
	LX_CHECK_NULL_ARG(vsCode);
	LX_CHECK_NULL_ARG(psCode);
	LX_CHECK_NULL_ARG(vsProfile);
	LX_CHECK_NULL_ARG(psProfile);

	if(vsLength == 0)
		vsLength = strlen(vsCode);
	if(psLength == 0)
		psLength = strlen(psCode);

	if(!vsEntryPoint)
		vsEntryPoint = "mainVS";
	if(!psEntryPoint)
		psEntryPoint = "mainPS";

	UnknownRefCounted<ID3DXConstantTable> vertexShaderConstants;
	UnknownRefCounted<ID3DXConstantTable> pixelShaderConstants;

	m_HasTextureSceneParam = false;
	m_VertexShader = CreateVertexShader(vsCode, vsEntryPoint, vsLength, vsProfile, errorList, vertexShaderConstants);

	if(psCode)
		m_PixelShader = CreatePixelShader(psCode, psEntryPoint, psLength, psProfile, errorList, pixelShaderConstants);

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
			u32 sceneParamId = 0;
			core::VariableAccess param;
			bool jumpToNext = false;
			try {
				param.Set(m_Renderer->GetParam(h.name, &sceneParamId));
			} catch(core::ObjectNotFoundException&) {
				if(errorList)
					errorList->PushBack(core::StringConverter::Format("Warning: Unknown scene value in shader: ~s.", h.name));
				jumpToNext = true;
			}

			if(jumpToNext)
				continue;

			if(GetCoreType(h.type) != param.GetType()) {
				if(errorList)
					errorList->PushBack(core::StringConverter::Format("Warning: Incompatible scene value type in shader: ~s.", h.name));
				continue;
			}

			entry.index = sceneParamId;
			entry.paramType = ParamType_Scene;
			if(entry.type == EType::Texture)
				m_HasTextureSceneParam = true;
		}
		break;
		}

		// Put name into namelist.
		memcpy((char*)m_Names + nameCursor, helper[i].name, helper[i].nameLength + 1);

		entry.registerPS = h.registerPS;
		entry.registerPSCount = h.registerPSCount;
		entry.registerVS = h.registerVS;
		entry.registerVSCount = h.registerVSCount;
		entry.type = h.type;
		if(entry.paramType == ParamType_Scene)
			m_SceneValues.PushBack(std::move(entry));
		else
			m_Params.PushBack(std::move(entry));

		nameCursor += h.nameLength + 1;
	}
}

void ShaderD3D9::LoadAllParams(bool isVertex, ID3DXConstantTable* table, core::Array<HelperEntry>& outParams, u32& outStringSize, core::Array<String>* errorList)
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

		u32 size, regId, regCount;
		EType type;
		const char* name;
		const void* defaultValue;
		bool isValidType;
		if(!GetStructureElemType(handle, table, type, size, regId, regCount, name, defaultValue, isValidType))
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
	const char* code, const char* entryPoint, size_t length, const char* profile,
	core::Array<String>* errorList,
	UnknownRefCounted<ID3DXConstantTable>& outTable)
{
	UnknownRefCounted<ID3DXBuffer> output;
	UnknownRefCounted<ID3DXBuffer> errors;
	UnknownRefCounted<IDirect3DVertexShader9> shader = nullptr;

	HRESULT hr = D3DXCompileShader(code, (UINT)length,
		NULL, NULL, entryPoint,
		profile,
		0, output.Access(), errors.Access(),
		outTable.Access());
	if(FAILED(hr)) {
		if(errors) {
			if(errorList) {
				String err = (const char*)errors->GetBufferPointer();
				auto err2 = err.Split('\n');
				errorList->PushBackm(err2.Data(), err2.Size());
			}
		}
		throw ShaderCompileException();
	}

	// Warnings.
	if(errors) {
		if(errorList) {
			String err = (const char*)errors->GetBufferPointer();
			auto err2 = err.Split('\n');
			errorList->PushBackm(err2.Data(), err2.Size());
		}
	}

	hr = m_D3DDevice->CreateVertexShader((DWORD*)output->GetBufferPointer(), shader.Access());
	if(FAILED(hr))
		throw core::D3D9Exception(hr);

	return shader;
}

UnknownRefCounted<IDirect3DPixelShader9>  ShaderD3D9::CreatePixelShader(
	const char* code, const char* entryPoint, size_t length, const char* profile,
	core::Array<String>* errorList,
	UnknownRefCounted<ID3DXConstantTable>& outTable)
{
	UnknownRefCounted<ID3DXBuffer> output;
	UnknownRefCounted<ID3DXBuffer> errors;
	UnknownRefCounted<IDirect3DPixelShader9> shader = nullptr;

	HRESULT hr;
	hr = D3DXCompileShader(code, (UINT)length,
		NULL, NULL, entryPoint,
		profile,
		0, output.Access(), errors.Access(),
		outTable.Access());
	if(FAILED(hr)) {
		if(errors) {
			if(errorList) {
				String err = (const char*)errors->GetBufferPointer();
				auto err2 = err.Split('\n');
				errorList->PushBackm(err2.Data(), err2.Size());
			}
		}
		throw ShaderCompileException();
	}

	// Warnings.
	if(errors) {
		if(errorList) {
			String err = (const char*)errors->GetBufferPointer();
			auto err2 = err.Split('\n');
			errorList->PushBackm(err2.Data(), err2.Size());
		}
	}

	DWORD* data = (DWORD*)output->GetBufferPointer();
	hr = m_D3DDevice->CreatePixelShader(data, shader.Access());
	if(FAILED(hr))
		throw core::D3D9Exception(hr);

	return shader;
}

size_t ShaderD3D9::GetSceneParamCount() const
{
	return m_SceneValues.Size();
}

u32 ShaderD3D9::GetSceneParam(size_t id) const
{
	return m_SceneValues.At(id).index;
}

bool ShaderD3D9::HasTextureSceneParam() const
{
	return m_HasTextureSceneParam;
}

void ShaderD3D9::Enable()
{
	HRESULT hr;

	if(FAILED(hr = m_D3DDevice->SetVertexShader(m_VertexShader)))
		throw core::D3D9Exception(hr);

	if(FAILED(hr = m_D3DDevice->SetPixelShader(m_PixelShader)))
		throw core::D3D9Exception(hr);
}

void ShaderD3D9::SetParam(const void* data, u32 paramId)
{
	bool found = false;
	u32 realId = 0;
	for(auto it = m_Params.First(); it != m_Params.End(); ++it) {
		if(it->index == paramId + DefaultParam_COUNT) {
			found = true;
			break;
		}
		++realId;
	}

	if(!found)
		throw core::ObjectNotFoundException("paramId");

	SetShaderValue(m_Params[realId], data);
}

void ShaderD3D9::LoadSceneParams(const RenderSettings& settings, u32 baseLayer)
{
	u32 layerId = baseLayer;

	for(auto it = m_SceneValues.First(); it != m_SceneValues.End(); ++it) {
		if(it->type == EType::Texture) {
			SetShaderValue(*it, &layerId);
			++layerId;
		} else {
			SetShaderValue(*it, m_Renderer->GetParam(it->index).Pointer());
		}
	}

	for(auto it = m_Params.First(); it != m_Params.End(); ++it) {
		if(it->paramType == ParamType_DefaultMaterial) {
			float f;
			video::Colorf c;
			switch(it->index) {
			case DefaultParam_Shininess:
				f = settings.material.GetShininess();
				SetShaderValue(*it, &f); break;
			case DefaultParam_Diffuse:
				c = settings.material.GetDiffuse();
				SetShaderValue(*it, &c); break;
			case DefaultParam_Emissive:
				c = settings.material.GetEmissive();
				SetShaderValue(*it, &c); break;
			case DefaultParam_Specular:
				c = settings.material.GetSpecular();
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

bool ShaderD3D9::GetStructureElemType(D3DXHANDLE handle, ID3DXConstantTable* table, EType& outType, u32& outSize, u32& registerID, u32& regCount, const char*& name, const void*& defaultValue, bool& isValid)
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
				outType = EType::Colorf;
		}
	} else if(desc.Class == D3DXPC_MATRIX_ROWS && desc.Rows == 4 && desc.Columns == 4) {
		if(desc.Type == D3DXPT_FLOAT)
			outType = EType::Matrix;
	} else if(desc.Class == D3DXPC_MATRIX_COLUMNS && desc.Rows == 4 && desc.Columns == 4) {
		if(desc.Type == D3DXPT_FLOAT)
			outType = EType::Matrix_ColMajor;
	} else if(desc.Class == D3DXPC_OBJECT) {
		if(desc.Type == D3DXPT_SAMPLER || desc.Type == D3DXPT_SAMPLER2D || desc.Type == D3DXPT_SAMPLER3D || desc.Type == D3DXPT_SAMPLERCUBE)
			outType = EType::Texture;
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
		break;
	case EType::Texture:
	case EType::Integer:
		break;
	case EType::Colorf:    ((float*)out)[3] = ((float*)in)[3];
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
	case EType::Texture:
	case EType::Integer:
		((int*)out)[0] = *(int*)in;
		break;
	case EType::Colorf:
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
	default:
		lxAssertNeverReach("Unsupported shader variable type.");
	}
}

void ShaderD3D9::SetShaderValue(const Param& param, const void* data)
{
	if((param.registerVS == 0xFFFFFFFF && param.registerPS == 0xFFFFFFFF) || param.type == EType::Unknown)
		return;

	static u32 v[16];

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
		case EType::Integer:
			hr = m_D3DDevice->SetVertexShaderConstantI(regId, (int*)v, 1);
			break;
		case EType::Float:
		case EType::Vector2:
		case EType::Vector3:
		case EType::Colorf:
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
		case EType::Integer:
			hr = m_D3DDevice->SetPixelShaderConstantI(regId, (int*)v, 1);
			break;
		case EType::Float:
		case EType::Vector2:
		case EType::Vector3:
		case EType::Colorf:
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
	static const char* NAMES[] = {
		"shininess",
		"diffuse",
		"emissive",
		"specular"
		"ambient",
		"power",
	};

	for(u32 i = 0; i < sizeof(NAMES) / sizeof(*NAMES); ++i) {
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
	case DefaultParam_Power:
		return EType::Float;
	case DefaultParam_Diffuse:
	case DefaultParam_Emissive:
	case DefaultParam_Specular:
		return EType::Colorf;
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
	case EType::U32: return core::Types::U32();
	case EType::Texture: return core::Types::Texture();
	case EType::Color: return core::Types::Color();
	case EType::Colorf: return core::Types::Colorf();
	case EType::Vector2: return core::Types::Vector2f();
	case EType::Vector3: return core::Types::Vector3f();
	case EType::Vector2Int: return core::Types::Vector2i();
	case EType::Vector3Int: return core::Types::Vector3i();
	case EType::Matrix: return core::Types::Matrix();
	case EType::Matrix_ColMajor: return core::Types::Matrix();
	case EType::Structure: return core::Types::Unknown();
	default: return core::Type::Unknown;
	}
}

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
