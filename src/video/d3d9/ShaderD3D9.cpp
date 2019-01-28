#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "ShaderD3D9.h"
#include "platform/StrippedD3D9X.h"
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

///////////////////////////////////////////////////////////////////////////////

// TODO: Provide Shader includes in a better way.

namespace
{

class luxD3DXShaderIncludes : public ID3DXInclude, core::Uncopyable
{
public:
	virtual ~luxD3DXShaderIncludes()
	{
	}

	STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
	{
		if(!ppData || !pBytes)
			return E_FAIL;

		if(IncludeType == D3DXINC_LOCAL) {
			if(io::FileSystem::Instance()->ExistFile(pFileName)) {
				auto file = io::FileSystem::Instance()->OpenFile(pFileName);
				*pBytes = core::SafeCast<UINT>(file->GetSize());
				m_Allocated.EmplaceBack(*pBytes);
				auto& data = m_Allocated.Back();
				if(file->ReadBinaryPart(*pBytes, data) != *pBytes)
					return E_FAIL;
				*ppData = data;
				return S_OK;
			} else {
				return Open(D3DXINC_SYSTEM, pFileName, pParentData, ppData, pBytes);
			}
		} else if(IncludeType == D3DXINC_SYSTEM) {
			core::StringView data;
			if(ShaderFactory::Instance()->GetShaderInclude(EShaderLanguage::HLSL, core::StringView(pFileName, std::strlen(pFileName)), data)) {
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
		for(int i = 0; i < m_Allocated.Size(); ++i) {
			if(m_Allocated[i] == pData) {
				m_Allocated.Erase(i);
				break;
			}
		}

		return S_OK;
	}

private:
	core::Array<core::RawMemory> m_Allocated;
};

luxD3DXShaderIncludes g_luxD3DXShaderIncludes;

bool ParseD3DXShaderError(core::StringView input, EShaderType shaderType, ShaderCompileMessage& out)
{
	// File(line, col): error number: error-string
	//     ^bracket   ^colon2       ^colon
	auto colon = input.FindReverse(":");
	if(colon == -1)
		return false;
	auto colon2 = input.BeginSubString(colon).FindReverse(":");
	if(colon2 == -1)
		return false;
	auto bracket = input.BeginSubString(colon2).FindReverse("(");
	if(bracket == -1)
		return false;
	core::StringConverter::EParseError error;
	auto line = core::StringConverter::ParseInt(input.SubString(bracket + 1, colon2 - bracket + 1), -1, nullptr, &error);
	if(error != core::StringConverter::EParseError::OK)
		return false;

	out.shaderType = shaderType;
	out.level = EShaderCompileMessageLevel::Error;
	out.text = input.EndSubString(colon + 2);
	out.line = line;
	return true;
}

void ParseErrors(core::Array<ShaderCompileMessage>& outList, UnknownRefCounted<ID3DXBuffer> errors, EShaderType shaderType)
{
	const char* perr = (const char*)errors->GetBufferPointer();
	core::StringView str(perr, std::strlen(perr));
	str.BasicSplit("\n", -1, true, [&](core::StringView str) {
		if(!str.IsWhitespace()) {
			ShaderCompileMessage msg;
			if(!ParseD3DXShaderError(str, shaderType, msg)) {
				msg.level = EShaderCompileMessageLevel::Error;
				msg.shaderType = shaderType;
				msg.line = -1;
				msg.text = str;
			}
			outList.PushBack(msg);
		}
	});
}

void CastTypeToShader(ShaderD3D9::EType type, const void* in, void* out)
{
	switch(type) {
	case ShaderD3D9::EType::Boolean:
		*(u32*)out = *(bool*)in;
		break;
	case ShaderD3D9::EType::Texture:
		break;
	case ShaderD3D9::EType::Integer:
		*(u32*)out = *(int*)in;
		break;
	case ShaderD3D9::EType::ColorF:    ((float*)out)[3] = ((float*)in)[3];
	case ShaderD3D9::EType::Vector3:      ((float*)out)[2] = ((float*)in)[2];
	case ShaderD3D9::EType::Vector2:      ((float*)out)[1] = ((float*)in)[1];
	case ShaderD3D9::EType::Float:     ((float*)out)[0] = ((float*)in)[0];
		break;
	case ShaderD3D9::EType::Matrix:
		std::memcpy(out, in, 16 * sizeof(float));
		break;
	case ShaderD3D9::EType::Matrix_ColMajor:
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

void CastShaderToType(ShaderD3D9::EType type, const void* in, void* out)
{
	switch(type) {
	case ShaderD3D9::EType::Boolean:
		((bool*)out)[0] = ((*(BOOL*)in) == TRUE);
		break;
	case ShaderD3D9::EType::Integer:
		((int*)out)[0] = *(int*)in;
		break;
	case ShaderD3D9::EType::ColorF:
		((float*)out)[3] = ((float*)in)[3];
	case ShaderD3D9::EType::Vector3:
		((float*)out)[2] = ((float*)in)[2];
	case ShaderD3D9::EType::Vector2:
		((float*)out)[1] = ((float*)in)[1];
	case ShaderD3D9::EType::Float:
		((float*)out)[0] = ((float*)in)[0];
		break;
	case ShaderD3D9::EType::Matrix:
		std::memcpy(out, in, 16 * sizeof(float));
		break;
	case ShaderD3D9::EType::Matrix_ColMajor:
	{
		float* pf = (float*)in;
		float* f = (float*)out;
		f[0] = pf[0];    f[1] = pf[4];   f[2] = pf[8];   f[3] = pf[12];
		f[4] = pf[1];    f[5] = pf[5];    f[6] = pf[9];   f[7] = pf[13];
		f[8] = pf[2];   f[9] = pf[6];    f[10] = pf[10];    f[11] = pf[14];
		f[12] = pf[3];    f[13] = pf[7];  f[14] = pf[11]; f[15] = pf[15];
	}
	break;
	case ShaderD3D9::EType::Texture:
		break;
	default:
		lxAssertNeverReach("Unsupported shader variable type.");
	}
}

core::Type GetCoreType(ShaderD3D9::EType type)
{
	switch(type) {
	case ShaderD3D9::EType::Unknown: return core::Type::Unknown;
	case ShaderD3D9::EType::Integer: return core::Types::Integer();
	case ShaderD3D9::EType::Float: return core::Types::Float();
	case ShaderD3D9::EType::Boolean: return core::Types::Boolean();
	case ShaderD3D9::EType::U32: return core::Types::UInteger();
	case ShaderD3D9::EType::Texture: return core::Types::Texture();
	case ShaderD3D9::EType::Color: return core::Types::Color();
	case ShaderD3D9::EType::ColorF: return core::Types::ColorF();
	case ShaderD3D9::EType::Vector2: return core::Types::Vector2F();
	case ShaderD3D9::EType::Vector3: return core::Types::Vector3F();
	case ShaderD3D9::EType::Vector2Int: return core::Types::Vector2I();
	case ShaderD3D9::EType::Vector3Int: return core::Types::Vector3I();
	case ShaderD3D9::EType::Matrix: return core::Types::Matrix();
	case ShaderD3D9::EType::Matrix_ColMajor: return core::Types::Matrix();
	case ShaderD3D9::EType::Structure: return core::Types::Unknown();
	default: return core::Type::Unknown;
	}
}

ShaderD3D9::EType GetTypeFromD3DXDesc(const D3DXCONSTANT_DESC& desc)
{
	if(desc.Class == D3DXPC_SCALAR) {
		if(desc.Type == D3DXPT_BOOL)
			return ShaderD3D9::EType::Boolean;
		else if(desc.Type == D3DXPT_INT)
			return ShaderD3D9::EType::Integer;
		else if(desc.Type == D3DXPT_FLOAT)
			return ShaderD3D9::EType::Float;
	} else if(desc.Class == D3DXPC_VECTOR) {
		if(desc.Type == D3DXPT_FLOAT) {
			if(desc.Columns == 2)
				return ShaderD3D9::EType::Vector2;
			else if(desc.Columns == 3)
				return ShaderD3D9::EType::Vector3;
			else if(desc.Columns == 4)
				return ShaderD3D9::EType::ColorF;
		}
	} else if(desc.Class == D3DXPC_MATRIX_ROWS && desc.Rows == 4 && desc.Columns == 4) {
		if(desc.Type == D3DXPT_FLOAT)
			return ShaderD3D9::EType::Matrix;
	} else if(desc.Class == D3DXPC_MATRIX_COLUMNS && desc.Rows == 4 && desc.Columns == 4) {
		if(desc.Type == D3DXPT_FLOAT)
			return ShaderD3D9::EType::Matrix_ColMajor;
	} else if(desc.Class == D3DXPC_OBJECT) {
		if(desc.Type == D3DXPT_SAMPLER || desc.Type == D3DXPT_SAMPLER2D || desc.Type == D3DXPT_SAMPLER3D || desc.Type == D3DXPT_SAMPLERCUBE) {
			return ShaderD3D9::EType::Texture;
		}
	}
	return ShaderD3D9::EType::Unknown;
}

///////////////////////////////////////////////////////////////////////////////

class HLSLShaderCompilerD3DX9
{
	struct Param
	{
		ShaderD3D9::ParamLocation vsLocation;
		ShaderD3D9::ParamLocation psLocation;

		ShaderD3D9::EType type = ShaderD3D9::EType::Unknown;

		ShaderD3D9::EParamType paramType = ShaderD3D9::EParamType::Other;

		u32 samplerStage;

		core::StringView name;
		const void* defaultValue = nullptr;
	};

public:
	StrongRef<ShaderD3D9> Compile(
		DeviceStateD3D9& deviceState,
		IDirect3DDevice9* device,
		core::StringView vsCode, core::StringView vsEntryPoint, core::StringView vsProfile,
		core::StringView psCode, core::StringView psEntryPoint, core::StringView psProfile,
		core::Array<ShaderCompileMessage>& outMessages)
	{
		UnknownRefCounted<ID3DXConstantTable> vertexShaderConstants;
		UnknownRefCounted<ID3DXConstantTable> pixelShaderConstants;

		bool compileError = false;
		auto vertexShaderAsm = CompileShader(EShaderType::Vertex, vsCode, vsEntryPoint, vsProfile, outMessages, vertexShaderConstants);
		if(!vertexShaderAsm)
			compileError = true;

		UnknownRefCounted<ID3DXBuffer> pixelShaderAsm;
		if(!psCode.IsEmpty()) {
			pixelShaderAsm = CompileShader(EShaderType::Pixel, psCode, psEntryPoint, psProfile, outMessages, pixelShaderConstants);
			if(!pixelShaderAsm)
				compileError = true;
		}

		if(compileError)
			return nullptr;

		// Load all shader parameters.
		u32 nameMemoryNeeded = 0;
		core::Array<Param> unsortedParams;
		bool errorWhileLoadingParams = false;
		errorWhileLoadingParams |= !LoadAllParams(EShaderType::Vertex, vertexShaderConstants, unsortedParams, nameMemoryNeeded, &outMessages);
		errorWhileLoadingParams |= !LoadAllParams(EShaderType::Pixel, pixelShaderConstants, unsortedParams, nameMemoryNeeded, &outMessages);

		if(errorWhileLoadingParams)
			return nullptr;

		core::ParamPackageBuilder ppb;

		core::Array<ShaderD3D9::Param> params;
		core::Array<ShaderD3D9::SceneParam> sceneParams;

		// Create param package and fill in additional data.
		for(int i = 0; i < unsortedParams.Size(); ++i) {
			auto& p = unsortedParams[i];
			if(p.paramType == ShaderD3D9::EParamType::Param) {
				// Add to param package.
				u8 tempMemory[sizeof(float) * 16]; // Matrix is the biggest type.
				if(p.defaultValue)
					CastShaderToType(p.type, p.defaultValue, tempMemory);
				else
					std::memset(tempMemory, 0, sizeof(tempMemory)); // Sets integers and float to zero.
				ppb.AddParam(p.name, GetCoreType(p.type), tempMemory);
			}

			// Place the value in the correct list.
			if(p.paramType == ShaderD3D9::EParamType::Scene) {
				ShaderD3D9::SceneParam param;
				param.psLocation = p.psLocation;
				param.vsLocation = p.vsLocation;
				param.type = p.type;
				param.name = p.name;
				sceneParams.PushBack(param);
			} else {
				ShaderD3D9::Param param;
				param.psLocation = p.psLocation;
				param.vsLocation = p.vsLocation;
				param.type = p.type;
				params.PushBack(param);
			}
		}

		auto paramPackage = ppb.BuildAndReset();

		// Create hardware shaders
		HRESULT hr;
		UnknownRefCounted<IDirect3DVertexShader9> vsShader;
		hr = device->CreateVertexShader((DWORD*)vertexShaderAsm->GetBufferPointer(), vsShader.Access());
		if(FAILED(hr))
			throw core::D3D9Exception(hr);
		UnknownRefCounted<IDirect3DPixelShader9> psShader;
		hr = device->CreatePixelShader((DWORD*)pixelShaderAsm->GetBufferPointer(), psShader.Access());
		if(FAILED(hr))
			throw core::D3D9Exception(hr);

		return LUX_NEW(ShaderD3D9)(
			deviceState,
			vsShader, psShader,
			paramPackage, params, sceneParams);

	}

private:
	UnknownRefCounted<ID3DXBuffer> CompileShader(
		EShaderType shaderType,
		core::StringView code, core::StringView entryPoint, core::StringView profile,
		core::Array<ShaderCompileMessage>& messages,
		UnknownRefCounted<ID3DXConstantTable>& outTable)
	{
		UnknownRefCounted<ID3DXBuffer> output;
		UnknownRefCounted<ID3DXBuffer> errors;

		HRESULT hr = D3DXLibraryLoader::Instance().GetD3DXCompileShader()(code.Data(), (UINT)code.Size(),
			NULL, &g_luxD3DXShaderIncludes, core::NulterminatedStringViewWrapper(entryPoint),
			core::NulterminatedStringViewWrapper(profile),
			0, output.Access(), errors.Access(),
			outTable.Access());

		if(FAILED(hr) && errors == nullptr)
			throw core::D3D9Exception(hr);

		if(errors)
			ParseErrors(messages, errors, shaderType);

		return output;
	}

	bool LoadAllParams(
		EShaderType shaderType,
		ID3DXConstantTable* table,
		core::Array<Param>& outParams,
		u32& outStringSize,
		core::Array<ShaderCompileMessage>* errorList)
	{
		D3DXCONSTANTTABLE_DESC tableDesc;
		HRESULT hr;
		if(FAILED(hr = table->GetDesc(&tableDesc)))
			throw core::D3D9Exception(hr);

		bool error = false;
		for(UINT i = 0; i < tableDesc.Constants; ++i) {
			D3DXHANDLE handle = table->GetConstant(NULL, i);
			if(handle == NULL)
				continue;

			ShaderD3D9::ParamLocation location;
			ShaderD3D9::EType type;
			core::StringView name;
			const void* defaultValue;
			ShaderD3D9::EParamType paramType;
			if(!GetParamInfo(shaderType, handle, table, type, location, name, defaultValue, paramType))
				continue;
			if(paramType == ShaderD3D9::EParamType::Other)
				continue;

			if(type == ShaderD3D9::EType::Unknown) {
				if(errorList)
					errorList->PushBack(
						ShaderCompileMessage(shaderType, EShaderCompileMessageLevel::Error,
							core::StringConverter::Format("Shader has unsupported parameter type. (param: {}).", name)));
				error = true;
				continue;
			}

			Param* foundEntry = nullptr;
			for(auto& p : outParams) {
				if(p.paramType == paramType && p.name.Equal(name)) {
					foundEntry = &p;
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
					errorList->PushBack(
						ShaderCompileMessage(EShaderType::Unknown, EShaderCompileMessageLevel::Error,
							core::StringConverter::Format("Shader param in pixelshader and vertex shader has diffrent types. (param: {}).", name)));
				error = true;
				continue;
			}

			if(shaderType == EShaderType::Vertex)
				foundEntry->vsLocation = location;
			else if(shaderType == EShaderType::Pixel)
				foundEntry->psLocation = location;
			else
				lxAssert(false);
		}

		return !error;
	}

	bool GetParamInfo(
		EShaderType shaderType,
		D3DXHANDLE handle,
		ID3DXConstantTable* table,
		ShaderD3D9::EType& outType,
		ShaderD3D9::ParamLocation& location,
		core::StringView& name,
		const void*& defaultValue,
		ShaderD3D9::EParamType& paramType)
	{
		D3DXCONSTANT_DESC desc;
		UINT count = 1;
		if(FAILED(table->GetConstantDesc(handle, &desc, &count)))
			return false;

		outType = GetTypeFromD3DXDesc(desc);
		if(outType == ShaderD3D9::EType::Texture) {
			DWORD baseId = 0;
			if(shaderType == EShaderType::Vertex)
				baseId = D3DVERTEXTEXTURESAMPLER0;
			else if(shaderType == EShaderType::Pixel)
				baseId = 0;
			else
				lxAssert(false);
			location.id = baseId + table->GetSamplerIndex(handle);
			location.count = 1;
		} else {
			location.id = desc.RegisterIndex;
			location.count = desc.RegisterCount;
		}
		name = core::StringView(desc.Name, std::strlen(desc.Name));
		defaultValue = desc.DefaultValue;
		if(name.StartsWith("param_")) {
			name = name.EndSubString(6);
			paramType = ShaderD3D9::EParamType::Param;
		} else if(name.StartsWith("scene_")) {
			name = name.EndSubString(6);
			paramType = ShaderD3D9::EParamType::Scene;
		} else {
			paramType = ShaderD3D9::EParamType::Other;
		}
		return true;
	}
};

}

StrongRef<Shader> Compile_HLSL_D3DX(
	DeviceStateD3D9& deviceState,
	IDirect3DDevice9* device,
	const ShaderCompileRequest_HLSL_D3DX& req,
	core::Array<ShaderCompileMessage>& outMessages)
{
	HLSLShaderCompilerD3DX9 compiler;
	return compiler.Compile(
		deviceState, device,
		req.vsCode, "mainVS", req.vsProfile,
		req.psCode, "mainPS", req.psProfile,
		outMessages);
}

///////////////////////////////////////////////////////////////////////////////

ShaderD3D9::ShaderD3D9(
	DeviceStateD3D9& deviceState,
	UnknownRefCounted<IDirect3DVertexShader9> vsShader,
	UnknownRefCounted<IDirect3DPixelShader9> psShader,
	const core::ParamPackage& paramPackage,
	const core::Array<Param>& params,
	const core::Array<SceneParam>& sceneParams) :
	m_DeviceState(deviceState),
	m_VertexShader(vsShader),
	m_PixelShader(psShader),
	m_Params(params),
	m_SceneValues(sceneParams),
	m_ParamPackage(paramPackage),
	m_CurAttributes(nullptr)
{
	m_SceneValueAttributeCache.Resize(sceneParams.Size());
}

ShaderD3D9::~ShaderD3D9()
{
}

void ShaderD3D9::Enable()
{
	m_DeviceState.EnableFixedFog(false);
	m_DeviceState.EnableLight(false);
	m_DeviceState.EnableHardwareShader(m_VertexShader, m_PixelShader);
}

void ShaderD3D9::SetParam(int paramId, const void* data)
{
	SetShaderValue(m_Params.At(paramId), data);
}

void ShaderD3D9::LoadSceneParams(core::AttributeList sceneAttributes, const Pass& pass)
{
	if(m_CurAttributes != sceneAttributes) {
		// Link with scene values.
		int i = 0;
		for(auto& sv : m_SceneValues) {
			core::AttributePtr ptr;
			ptr = sceneAttributes.Pointer(sv.name);

			if(ptr && GetCoreType(sv.type) != ptr->GetType())
				ptr = nullptr;

			m_SceneValueAttributeCache[i] = ptr;
			++i;
		}
		m_CurAttributes = sceneAttributes;
	}

	LUX_UNUSED(pass);
	for(int i = 0; i < m_SceneValues.Size(); ++i) {
		if(m_SceneValueAttributeCache[i])
			SetShaderValue(m_SceneValues[i], m_SceneValueAttributeCache[i]->GetAccess().Pointer());
	}
}

void ShaderD3D9::SetShaderValue(const BasicParam& param, const void* data)
{
	if(param.vsLocation.count == 0 && param.psLocation.count == 0)
		return;

	static u32 v[16];

	if(param.type != EType::Texture)
		CastTypeToShader(param.type, data, v);

	auto device = m_DeviceState.GetLowLevelDevice();
	HRESULT hr;
	if(param.vsLocation.count != 0) {
		u32 id = param.vsLocation.id;
		switch(param.type) {
		case EType::Boolean:
			hr = device->SetVertexShaderConstantB(id, (BOOL*)v, 1);
			break;
		case EType::Texture:
			m_DeviceState.EnableTextureLayer(id, *(const video::TextureLayer*)data);
			break;
		case EType::Integer:
			hr = device->SetVertexShaderConstantI(id, (int*)v, 1);
			break;
		case EType::Float:
		case EType::Vector2:
		case EType::Vector3:
		case EType::ColorF:
			hr = device->SetVertexShaderConstantF(id, (float*)v, 1);
			break;
		case EType::Matrix:
		case EType::Matrix_ColMajor:
			hr = device->SetVertexShaderConstantF(id, (float*)v, param.vsLocation.count);
			break;
		default:
			lxAssertNeverReach("Unsupported shader variable type.");
		}
	}
	if(param.psLocation.count != 0) {
		u32 id = param.psLocation.id;
		switch(param.type) {
		case EType::Boolean:
			hr = device->SetPixelShaderConstantB(id, (BOOL*)v, 1);
			break;
		case EType::Texture:
			m_DeviceState.EnableTextureLayer(id, *(const video::TextureLayer*)data);
			break;
		case EType::Integer:
			hr = device->SetPixelShaderConstantI(id, (int*)v, 1);
			break;
		case EType::Float:
		case EType::Vector2:
		case EType::Vector3:
		case EType::ColorF:
			hr = device->SetPixelShaderConstantF(id, (float*)v, 1);
			break;
		case EType::Matrix:
		case EType::Matrix_ColMajor:
			hr = device->SetPixelShaderConstantF(id, (float*)v, param.psLocation.count);
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

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
