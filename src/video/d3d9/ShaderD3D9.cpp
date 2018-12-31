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

namespace
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
			core::StringView data;
			if(MaterialLibrary::Instance()->GetShaderInclude(EShaderLanguage::HLSL, core::StringView(pFileName, strlen(pFileName)), data)) {
				*ppData = data.Data();
				*pBytes = (UINT)data.Size();
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

luxD3DXShaderIncludes g_luxD3DXShaderIncludes;

core::String FormatD3DXShaderError(core::StringView input, bool isVertex)
{
	// File(line, col): error number: error-string
	auto colon = input.FindReverse(":");
	colon = colon + input.BeginSubString(colon).FindReverse(":");
	auto base_name = colon + input.BeginSubString(colon).FindReverse("\\");
	if(base_name == colon)
		base_name = 0;
	else
		base_name++;

	return (isVertex ? "vs: " : "ps: ") + input.EndSubString(base_name);
}

void WriteErrors(core::Array<core::String>* outList, UnknownRefCounted<ID3DXBuffer> errors, bool isVertex)
{
	if(errors) {
		if(outList) {
			const char* perr = (const char*)errors->GetBufferPointer();
			core::StringView str(perr, std::strlen(perr));
			str.BasicSplit("\n", -1, true, [&](core::StringView str) {
				if(!str.IsWhitespace())
					outList->PushBack(FormatD3DXShaderError(str, isVertex));
			});
		}
	}
}
}

ShaderD3D9::ShaderD3D9(VideoDriver* driver, DeviceStateD3D9& state) :
	m_D3DDevice((IDirect3DDevice9*)driver->GetLowLevelDevice()),
	m_DeviceState(state),
	m_CurAttributes(nullptr)
{
}

ShaderD3D9::~ShaderD3D9()
{
}

bool ShaderD3D9::Init(
	core::StringView vsCode, core::StringView vsEntryPoint, core::StringView vsProfile,
	core::StringView psCode, core::StringView psEntryPoint, core::StringView psProfile,
	core::Array<core::String>* errorList)
{
	UnknownRefCounted<ID3DXConstantTable> vertexShaderConstants;
	UnknownRefCounted<ID3DXConstantTable> pixelShaderConstants;

	bool compileError = false;
	m_VertexShader = CreateVertexShader(vsCode, vsEntryPoint, vsProfile, errorList, vertexShaderConstants);
	if(!m_VertexShader)
		compileError = true;

	if(!psCode.IsEmpty()) {
		m_PixelShader = CreatePixelShader(psCode, psEntryPoint, psProfile, errorList, pixelShaderConstants);
		if(!m_PixelShader)
			compileError = true;
	}

	if(compileError)
		return false;

	// Load all shader parameters.
	u32 nameMemoryNeeded = 0;
	core::Array<Param> unsortedParams;
	LoadAllParams(true, vertexShaderConstants, unsortedParams, nameMemoryNeeded, errorList);
	LoadAllParams(false, pixelShaderConstants, unsortedParams, nameMemoryNeeded, errorList);

	u32 nameCursor = 0;
	core::ParamPackageBuilder ppb;
	m_Names.SetMinSize(nameMemoryNeeded);

	// Create param package and fill in additional data.
	for(int i = 0; i < unsortedParams.Size(); ++i) {
		Param& p = unsortedParams[i];
		switch(p.paramType) {
		case EParamType::Param:
		{
			// Add to param package.
			u8 tempMemory[sizeof(float) * 16]; // Matrix is the biggest type.
			if(p.defaultValue)
				CastShaderToType(p.type, p.defaultValue, tempMemory);
			else
				std::memset(tempMemory, 0, sizeof(tempMemory)); // Sets integers and float to zero.
			ppb.AddParam(p.name, GetCoreType(p.type), tempMemory);
		}
		break;
		case EParamType::Scene:
		{
		}
		break;
		default:
			lxAssertNeverReach("");
		}

		// Backup name into namelist, and change the name location.
		// The namelist is used, so the D3DXConstantTable can be released.
		std::memcpy((char*)m_Names + nameCursor, p.name.Data(), p.name.Size());
		((char*)m_Names)[nameCursor + p.name.Size()] = 0;
		core::StringView newName((char*)m_Names + nameCursor, p.name.Size());
		p.name = newName;

		nameCursor += p.name.Size() + 1;

		// Release the defaultValue as well.
		p.defaultValue = nullptr;

		// Place the value in the correct list.
		if(p.paramType == EParamType::Scene)
			m_SceneValues.PushBack(std::move(p));
		else
			m_Params.PushBack(std::move(p));
	}

	m_ParamPackage = ppb.BuildAndReset();

	return true;
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
	SetShaderValue(m_Params.At(paramId), data);
}

void ShaderD3D9::LoadSceneParams(core::AttributeList sceneAttributes, const Pass& pass)
{
	if(m_CurAttributes != sceneAttributes) {
		m_CurAttributes = sceneAttributes;
		// Link with scene values.
		for(auto& sv : m_SceneValues) {
			core::AttributePtr ptr;
			ptr = sceneAttributes.Pointer(sv.name);

			if(ptr && GetCoreType(sv.type) != ptr->GetType())
				ptr = nullptr;

			sv.sceneValue = ptr;
		}
	}

	LUX_UNUSED(pass);
	for(auto it = m_SceneValues.First(); it != m_SceneValues.End(); ++it) {
		if(it->sceneValue)
			SetShaderValue(*it, (*it->sceneValue).Pointer());
	}
}

void ShaderD3D9::Disable()
{
	m_D3DDevice->SetVertexShader(NULL);
	m_D3DDevice->SetPixelShader(NULL);
}

UnknownRefCounted<IDirect3DVertexShader9> ShaderD3D9::CreateVertexShader(
	core::StringView code, core::StringView entryPoint, core::StringView profile,
	core::Array<core::String>* errorList,
	UnknownRefCounted<ID3DXConstantTable>& outTable)
{
	UnknownRefCounted<ID3DXBuffer> output;
	UnknownRefCounted<ID3DXBuffer> errors;
	UnknownRefCounted<IDirect3DVertexShader9> shader;

	HRESULT hr = D3DXCompileShader(code.Data(), (UINT)code.Size(),
		NULL, &g_luxD3DXShaderIncludes, core::NulterminatedStringViewWrapper(entryPoint),
		core::NulterminatedStringViewWrapper(profile),
		0, output.Access(), errors.Access(),
		outTable.Access());

	WriteErrors(errorList, errors, true);

	if(SUCCEEDED(hr)) {
		hr = m_D3DDevice->CreateVertexShader((DWORD*)output->GetBufferPointer(), shader.Access());
		if(FAILED(hr))
			throw core::D3D9Exception(hr);
	}

	return shader;
}

UnknownRefCounted<IDirect3DPixelShader9>  ShaderD3D9::CreatePixelShader(
	core::StringView code, core::StringView entryPoint, core::StringView profile,
	core::Array<core::String>* errorList,
	UnknownRefCounted<ID3DXConstantTable>& outTable)
{
	UnknownRefCounted<ID3DXBuffer> output;
	UnknownRefCounted<ID3DXBuffer> errors;
	UnknownRefCounted<IDirect3DPixelShader9> shader;

	HRESULT hr;
	hr = D3DXCompileShader(code.Data(), (UINT)code.Size(),
		NULL, &g_luxD3DXShaderIncludes, core::NulterminatedStringViewWrapper(entryPoint),
		core::NulterminatedStringViewWrapper(profile),
		0, output.Access(), errors.Access(),
		outTable.Access());

	WriteErrors(errorList, errors, false);

	if(SUCCEEDED(hr)) {
		DWORD* data = (DWORD*)output->GetBufferPointer();
		hr = m_D3DDevice->CreatePixelShader(data, shader.Access());
		if(FAILED(hr))
			throw core::D3D9Exception(hr);
	}

	return shader;
}

void ShaderD3D9::LoadAllParams(bool isVertex, ID3DXConstantTable* table, core::Array<Param>& outParams, u32& outStringSize, core::Array<core::String>* errorList)
{
	D3DXCONSTANTTABLE_DESC tableDesc;
	HRESULT hr;
	if(FAILED(hr = table->GetDesc(&tableDesc)))
		throw core::D3D9Exception(hr);

	for(UINT i = 0; i < tableDesc.Constants; ++i) {
		D3DXHANDLE handle = table->GetConstant(NULL, i);
		if(handle == NULL)
			continue;

		RegisterLocation location;
		u32 samplerStage;
		EType type;
		core::StringView name;
		const void* defaultValue;
		EParamType paramType;
		if(!GetParamInfo(handle, table, samplerStage, type, location, name, defaultValue, paramType))
			continue;

		if(paramType == EParamType::Other || type == EType::Unknown) {
			if(errorList)
				errorList->PushBack(core::StringConverter::Format("Shader has unsupported parameter type. (param: {}).", name));
			throw UnhandledShaderCompileErrorException();
		}

		Param* foundEntry = nullptr;
		for(auto it = outParams.First(); it != outParams.End(); ++it) {
			if(it->paramType == paramType && it->name.Equal(name)) {
				foundEntry = &*it;
				break;
			}
		}

		if(!foundEntry) {
			// If the entry doesn't exist, create it.
			Param entry;
			entry.name = name;
			entry.type = type;
			entry.defaultValue = defaultValue;
			entry.paramType = paramType;

			outParams.PushBack(entry);
			outStringSize += entry.name.Size() + 1;
			foundEntry = &outParams.Back();
		}

		if(foundEntry->type != type) {
			if(errorList)
				errorList->PushBack(core::StringConverter::Format("Shader param in pixelshader and vertex shader has diffrent types. (param: {}).", name));
			throw UnhandledShaderCompileErrorException();
		}

		if(isVertex)
			foundEntry->vsLocation = location;
		else
			foundEntry->psLocation = location;
		foundEntry->samplerStage = samplerStage;
	}
}

bool ShaderD3D9::GetParamInfo(
	D3DXHANDLE handle,
	ID3DXConstantTable* table,
	u32& samplerStage,
	EType& outType,
	RegisterLocation& location,
	core::StringView& name,
	const void*& defaultValue,
	EParamType& paramType)
{
	D3DXCONSTANT_DESC desc;
	UINT count = 1;
	if(FAILED(table->GetConstantDesc(handle, &desc, &count)))
		return false;

	outType = GetTypeFromD3DXDesc(desc);
	if(outType == EType::Texture)
		samplerStage = table->GetSamplerIndex(handle);

	location.id = desc.RegisterIndex;
	location.count = desc.RegisterCount;
	name = core::StringView(desc.Name, strlen(desc.Name));
	defaultValue = desc.DefaultValue;
	if(name.StartsWith("param_")) {
		name = name.EndSubString(6);
		paramType = EParamType::Param;
	} else if(name.StartsWith("scene_")) {
		name = name.EndSubString(6);
		paramType = EParamType::Scene;
	} else {
		paramType = EParamType::Other;
	}
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
	if(param.vsLocation.count == 0 && param.psLocation.count == 0)
		return;

	static u32 v[16];

	if(param.type != EType::Texture)
		CastTypeToShader(param.type, data, v);

	u32 regId;
	HRESULT hr;
	if(param.vsLocation.count != 0) {
		regId = param.vsLocation.id;
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
			hr = m_D3DDevice->SetVertexShaderConstantF(regId, (float*)v, param.vsLocation.count);
			break;
		default:
			lxAssertNeverReach("Unsupported shader variable type.");
		}
	}
	if(param.psLocation.count != 0) {
		regId = param.psLocation.id;
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
			hr = m_D3DDevice->SetPixelShaderConstantF(regId, (float*)v, param.psLocation.count);
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

ShaderD3D9::EType ShaderD3D9::GetTypeFromD3DXDesc(const D3DXCONSTANT_DESC& desc)
{
	if(desc.Class == D3DXPC_SCALAR) {
		if(desc.Type == D3DXPT_BOOL)
			return EType::Boolean;
		else if(desc.Type == D3DXPT_INT)
			return EType::Integer;
		else if(desc.Type == D3DXPT_FLOAT)
			return EType::Float;
	} else if(desc.Class == D3DXPC_VECTOR) {
		if(desc.Type == D3DXPT_FLOAT) {
			if(desc.Columns == 2)
				return EType::Vector2;
			else if(desc.Columns == 3)
				return EType::Vector3;
			else if(desc.Columns == 4)
				return EType::ColorF;
		}
	} else if(desc.Class == D3DXPC_MATRIX_ROWS && desc.Rows == 4 && desc.Columns == 4) {
		if(desc.Type == D3DXPT_FLOAT)
			return EType::Matrix;
	} else if(desc.Class == D3DXPC_MATRIX_COLUMNS && desc.Rows == 4 && desc.Columns == 4) {
		if(desc.Type == D3DXPT_FLOAT)
			return EType::Matrix_ColMajor;
	} else if(desc.Class == D3DXPC_OBJECT) {
		if(desc.Type == D3DXPT_SAMPLER || desc.Type == D3DXPT_SAMPLER2D || desc.Type == D3DXPT_SAMPLER3D || desc.Type == D3DXPT_SAMPLERCUBE) {
			return EType::Texture;
		}
	}
	return EType::Unknown;
}

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
