#ifdef LUX_COMPILE_WITH_D3D9
#include "core/Logger.h"
#include "video/VideoDriver.h"
#include "video/Material.h"

#include "ShaderD3D9.h"

namespace lux
{
namespace video
{


ShaderD3D9::ShaderD3D9(VideoDriver* driver) :
	m_D3DDevice((IDirect3DDevice9*)driver->GetDevice()),
	m_SceneValues(driver->GetSceneValues()),
	m_VertexShader(nullptr), m_PixelShader(nullptr),
	m_VertexShaderConstants(nullptr), m_PixelShaderConstants(nullptr),
	m_Names(nullptr),
	m_InvalidParam(this, core::Type::Unknown, 0, nullptr, 0xFFFFFFFF, 0xFFFFFFFF)
{
}

ShaderD3D9::~ShaderD3D9()
{
	if(m_VertexShader)
		m_VertexShader->Release();

	if(m_PixelShader)
		m_PixelShader->Release();

	LUX_FREE_ARRAY(m_Names);
}

bool ShaderD3D9::Init(
	const char* vsCode, const char* vsEntryPoint, size_t vsLength, const char* vsProfile,
	const char* psCode, const char* psEntryPoint, size_t psLength, const char* psProfile)
{
	if(!vsCode || !psCode)
		return false;

	if(vsLength == 0)
		vsLength = strlen(vsCode);
	if(psLength == 0)
		psLength = strlen(psCode);

	if(!vsEntryPoint)
		vsEntryPoint = "mainVS";
	if(!psEntryPoint)
		psEntryPoint = "mainPS";

	if(!CreateVertexShader(vsCode, vsEntryPoint, vsLength, vsProfile))
		return false;

	if(psCode) {
		if(!CreatePixelShader(psCode, psEntryPoint, psLength, psProfile))
			return false;
	}

	// Load all shader parameters.
	core::array<HelperEntry> helper;
	u32 nameMemoryNeeded = 0;
	bool result = true;
	if(result && m_VertexShader)
		result = LoadAllParams(m_VertexShaderConstants, helper, nameMemoryNeeded);
	if(result && m_PixelShader)
		result = LoadAllParams(m_PixelShaderConstants, helper, nameMemoryNeeded);

	if(!result) {
		if(m_VertexShader) {
			m_VertexShaderConstants->Release();
			m_VertexShaderConstants = nullptr;
		}

		if(m_PixelShader) {
			m_PixelShaderConstants->Release();
			m_PixelShaderConstants = nullptr;
		}

		return false;
	}

	u32 materialParamCount = (u32)helper.Size();
	// Param ids are saved as 16-Bit integer
	if(materialParamCount > 0xFFFF) {
		lxAssertNeverReach("Helper array overflow");
		materialParamCount = 0xFFFF;
	}

	u32 nameCursor = 0;
	m_Params.Reserve(helper.Size());
	m_Names = LUX_NEW_ARRAY(char, nameMemoryNeeded);
	for(u32 i = 0; i < materialParamCount; ++i) {
		ParamEntry entry;
		const HelperEntry& h = helper[i];
		switch(h.paramType) {
		case ParamType_DefaultMaterial:
		case ParamType_ParamMaterial:
		{
			int defId = GetDefaultId(h.name);
			if(defId >= 0) {
				if(GetDefaultType((u32)defId) != h.type) {
					log::Error("Wrong type for default material param in shader: ~s.", h.name);
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
				m_ParamPackage.AddParam(h.type, h.name, tempMemory, (u16)i);

				entry.index = 0;
				entry.paramType = ParamType_ParamMaterial;
			}
		}
		break;
		case ParamType_Scene:
			u32 idx = m_SceneValues->GetParamID(h.name);
			if(idx == 0xFFFFFFFF) {
				log::Warning("Unknown scene value in shader: ~s.", h.name);
				continue;
			}

			if(!IsTypeCompatible(h.type, m_SceneValues->GetParamType(idx))) {
				log::Warning("Incompatible scene value type in shader: ~s.", h.name);
				continue;
			}

			entry.index = idx;
			entry.paramType = ParamType_Scene;

			break;
		}

		// Put name into namelist.
		memcpy(m_Names + nameCursor, helper[i].name, helper[i].nameLength+1);

		entry.param = ShaderParam(this, h.type, h.typeSize, m_Names + nameCursor, h.registerVS, h.registerPS);
		m_Params.Push_Back(std::move(entry));

		nameCursor += h.nameLength+1;
	}

	// Free constant data.
	if(m_VertexShader) {
		m_VertexShaderConstants->Release();
		m_VertexShaderConstants = nullptr;
	}

	if(m_PixelShader) {
		m_PixelShaderConstants->Release();
		m_PixelShaderConstants = nullptr;
	}

	return true;
}

bool ShaderD3D9::LoadAllParams(ID3DXConstantTable* table, core::array<HelperEntry>& outParams, u32& outStringSize)
{
	D3DXCONSTANTTABLE_DESC tableDesc;
	if(FAILED(table->GetDesc(&tableDesc)))
		return false;

	for(UINT i = 0; i < tableDesc.Constants; ++i) {
		D3DXHANDLE handle = table->GetConstant(NULL, i);
		if(handle == NULL)
			continue;

		u32 size, regId;
		core::Type type;
		const char* name;
		const void* defaultValue;
		bool isValidType;
		if(!GetStructureElemType(handle, table, type, size, regId, name, defaultValue, isValidType))
			continue;

		bool isParam = false;
		bool isScene = false;
		if(strncmp(name, "param_", 6) == 0) {
			name += 6;
			isParam = true;
		} else if(strncmp(name, "tex_", 4) == 0) {
			name += 4;
			isParam = true;
		} else if(strncmp(name, "scene_", 6) == 0) {
			name += 6;
			isScene = true;
		}

		if((isParam || isScene) && !isValidType) {
			log::Error("Shader has unsupported parameter type. (param: ~s).", name);
			return false;
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
			if(table == m_VertexShaderConstants)
				foundEntry->registerVS = regId;
			else
				foundEntry->registerPS = regId;
			if(foundEntry->type != type) {
				log::Error("Shader param in pixelshader and vertex shader has diffrent types. (param: ~s).", name);
				return false;
			}
		} else {
			// Otherwise, create a new entry.
			HelperEntry HEntry;
			HEntry.nameLength = (u32)strlen(name);
			HEntry.name = name;
			HEntry.type = type;
			HEntry.typeSize = (u8)size;
			HEntry.defaultValue = defaultValue;
			if(table == m_VertexShaderConstants)
				HEntry.registerVS = regId;
			else
				HEntry.registerPS = regId;
			HEntry.paramType = isParam ? ParamType_ParamMaterial : ParamType_Scene;
			outParams.Push_Back(HEntry);
			outStringSize += HEntry.nameLength + 1;
		}
	}

	return true;
}

bool ShaderD3D9::CreateVertexShader(const char* code, const char* entryPoint, size_t length, const char* profile)
{
	ID3DXBuffer* output = 0;
	ID3DXBuffer* errors = 0;

	HRESULT hr = D3DXCompileShader(code, (UINT)length,
		NULL, NULL, entryPoint,
		profile,
		0, &output, &errors,
		&m_VertexShaderConstants);
	if(FAILED(hr)) {
		log::Error("Failed to compile vertexshader.");

		if(errors) {
			log::Error((const char*)(errors->GetBufferPointer()));
			errors->Release();
		}
		return false;
	}

	if(errors) {
		log::Warning((const char*)(errors->GetBufferPointer()));
		errors->Release();
		errors = 0;
	}

	if(FAILED(m_D3DDevice->CreateVertexShader((DWORD*)(output->GetBufferPointer()), &m_VertexShader))) {
		log::Error("Failed to compile vertexshader.");
		return false;
	}

	output->Release();
	output = 0;

	return true;
}

bool ShaderD3D9::CreatePixelShader(const char* code, const char* entryPoint, size_t length, const char* profile)
{
	ID3DXBuffer* output = 0;
	ID3DXBuffer* errors = 0;

	if(FAILED(D3DXCompileShader(code, (UINT)length,
		NULL, NULL, entryPoint,
		profile,
		0, &output, &errors,
		&m_PixelShaderConstants))) {
		log::Error("Failed to compile pixelshader.");

		if(errors) {
			log::Error((const char*)(errors->GetBufferPointer()));
			errors->Release();
		}
		return false;
	}

	if(errors) {
		log::Warning((const char*)(errors->GetBufferPointer()));
		errors->Release();
		errors = 0;
	}


	if(FAILED(m_D3DDevice->CreatePixelShader((DWORD*)(output->GetBufferPointer()), &m_PixelShader))) {
		log::Error("Failed to create pixelshader.");
		output->Release();
		return false;
	}

	output->Release();

	return true;
}

const ShaderParam& ShaderD3D9::GetParam(const char* name)
{
	for(auto it = m_Params.First(); it != m_Params.End(); ++it) {
		if(strcmp(it->param.GetName(), name) == 0)
			return it->param;
	}

	return m_InvalidParam;
}

const ShaderParam& ShaderD3D9::GetParam(u32 index)
{
	if(index < m_Params.Size())
		return m_Params[index].param;

	return m_InvalidParam;
}

u32 ShaderD3D9::GetParamCount() const
{
	return (u32)m_Params.Size();
}

void ShaderD3D9::Enable()
{
	HRESULT hr = m_D3DDevice->SetVertexShader(m_VertexShader);
	hr = m_D3DDevice->SetPixelShader(m_PixelShader);
}

void ShaderD3D9::LoadParams(const core::PackagePuffer& Puffer, const RenderData* renderData)
{
	const core::ParamPackage* pack = Puffer.GetType();
	if(pack != nullptr) {
		core::ParamPackage::ParamDesc desc;
		for(u32 i = 0; i < pack->GetParamCount(); ++i) {
			if(pack->GetParamDesc(i, desc)) {
				if(desc.reserved != 0xFFFF) {
					// Its a shader param
					ShaderParam& param = m_Params[desc.reserved].param;
					param.SetShaderValue(Puffer.FromID(i, true).Pointer());
				}
			}
		}
	}

	const Material* mat = dynamic_cast<const Material*>(renderData);

	if(mat) {
		for(auto it = m_Params.First(); it != m_Params.End(); ++it) {
			if(it->paramType == ParamType_DefaultMaterial) {
				ShaderParam& param = it->param;
				switch(it->index) {
				case DefaultParam_Shininess: param.SetShaderValue(&mat->shininess); break;
				case DefaultParam_Diffuse: param.SetShaderValue(&mat->diffuse); break;
				case DefaultParam_Emissive: param.SetShaderValue(&mat->emissive); break;
				case DefaultParam_Specular: param.SetShaderValue(&mat->specular); break;
				default: continue;
				}
			}
		}
	}

}

void ShaderD3D9::LoadSceneValues()
{
	for(auto it = m_Params.First(); it != m_Params.End(); ++it) {
		if(it->paramType == ParamType_Scene) {
			it->param.SetShaderValue(m_SceneValues->GetParamValue(it->index));
		}
	}
}

void ShaderD3D9::Disable()
{
	m_D3DDevice->SetVertexShader(NULL);
	m_D3DDevice->SetPixelShader(NULL);
}

bool ShaderD3D9::GetStructureElemType(D3DXHANDLE handle, ID3DXConstantTable* table, core::Type& outType, u32& outSize, u32& registerID, const char*& name, const void*& defaultValue, bool& isValid)
{
	D3DXCONSTANT_DESC desc;
	UINT count = 1;
	if(FAILED(table->GetConstantDesc(handle, &desc, &count)))
		return false;

	outType = core::Type::Unknown;
	if(desc.Class == D3DXPC_STRUCT)
		outType = core::Type::Internal_Composed;
	else if(desc.Class == D3DXPC_SCALAR) {
		if(desc.Type == D3DXPT_BOOL)
			outType = core::Type::Bool;
		else if(desc.Type == D3DXPT_INT)
			outType = core::Type::Integer;
		else if(desc.Type == D3DXPT_FLOAT)
			outType = core::Type::Float;
	} else if(desc.Class == D3DXPC_VECTOR) {
		if(desc.Type == D3DXPT_FLOAT) {
			if(desc.Columns == 2)
				outType = core::Type::Vector2;
			else if(desc.Columns == 3)
				outType = core::Type::Vector3;
			else if(desc.Columns == 4)
				outType = core::Type::ColorF;
		}
	} else if(desc.Class == D3DXPC_MATRIX_ROWS && desc.Rows == 4 && desc.Columns == 4) {
		if(desc.Type == D3DXPT_FLOAT)
			outType = core::Type::Matrix;
	} else if(desc.Class == D3DXPC_MATRIX_COLUMNS && desc.Rows == 4 && desc.Columns == 4) {
		if(desc.Type == D3DXPT_FLOAT)
			outType = core::Type::Internal_MatrixCol;
	} else if(desc.Class == D3DXPC_OBJECT) {
		if(desc.Type == D3DXPT_SAMPLER)
			outType = core::Type::Texture;
	}

	outSize = desc.Bytes;
	registerID = desc.RegisterIndex;
	name = desc.Name;
	defaultValue = desc.DefaultValue;

	isValid = (outType != core::Type::Unknown);
	return true;
}

void ShaderD3D9::CastTypeToShader(core::Type type, const void* in, void* out)
{
	switch((core::Type::EType)type) {
	case core::Type::Bool:
		((BOOL*)out)[0] = (*(bool*)in) ? TRUE : FALSE;
		break;
	case core::Type::Texture:
	case core::Type::Integer:
		((int*)out)[0] = *(int*)in;
		break;
	case core::Type::ColorF:    ((float*)out)[3] = ((float*)in)[3];
	case core::Type::Vector3:      ((float*)out)[2] = ((float*)in)[2];
	case core::Type::Vector2:      ((float*)out)[1] = ((float*)in)[1];
	case core::Type::Float:     ((float*)out)[0] = ((float*)in)[0];
		break;
	case core::Type::Matrix:
		memcpy(out, in, 16 * sizeof(float));
		break;
	case core::Type::Internal_MatrixCol:
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

void ShaderD3D9::CastShaderToType(core::Type type, const void* in, void* out)
{
	switch((core::Type::EType)type) {
	case core::Type::Bool:
		((bool*)out)[0] = ((*(BOOL*)in) == TRUE);
		break;
	case core::Type::Texture:
	case core::Type::Integer:
		((int*)out)[0] = *(int*)in;
		break;
	case core::Type::ColorF:
		((float*)out)[3] = ((float*)in)[3];
	case core::Type::Vector3:
		((float*)out)[2] = ((float*)in)[2];
	case core::Type::Vector2:
		((float*)out)[1] = ((float*)in)[1];
	case core::Type::Float:
		((float*)out)[0] = ((float*)in)[0];
		break;
	case core::Type::Matrix:
		memcpy(out, in, 16 * sizeof(float));
		break;
	case core::Type::Internal_MatrixCol:
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

void ShaderD3D9::GetShaderValue(u32 registerVS, u32 registerPS, core::Type type, u32 size, void* out)
{
	LUX_UNUSED(size);

	if((registerVS == 0xFFFFFFFF && registerPS == 0xFFFFFFFF) || type == core::Type::Unknown)
		return;

	int i[4];
	float f[16];
	BOOL b[4];
	u32 regId;
	if(registerVS != -1) {
		regId = registerVS;
		switch((core::Type::EType)type) {
		case core::Type::Bool:
		{
			m_D3DDevice->GetVertexShaderConstantB(regId, b, 1);
			*(bool*)out = (*b == TRUE);
		}
		break;
		case core::Type::Texture:
		case core::Type::Integer:
			m_D3DDevice->GetVertexShaderConstantI(regId, i, 1);
			*((int*)out) = *i;
			break;
		case core::Type::Float:
			m_D3DDevice->GetVertexShaderConstantF(regId, f, 1);
			*((float*)out) = *f;
			break;
		case core::Type::Vector2:
			m_D3DDevice->GetVertexShaderConstantF(regId, f, 1);
			((float*)out)[0] = f[0];
			((float*)out)[1] = f[1];
			break;
		case core::Type::Vector3:
			m_D3DDevice->GetVertexShaderConstantF(regId, f, 1);
			((float*)out)[0] = f[0];
			((float*)out)[1] = f[1];
			((float*)out)[2] = f[2];
			break;
		case core::Type::ColorF:
			m_D3DDevice->GetVertexShaderConstantF(regId, (float*)out, 1);
			break;
		case core::Type::Matrix:
			m_D3DDevice->GetVertexShaderConstantF(regId, (float*)out, 4);
			break;
		case core::Type::Internal_MatrixCol:
			m_D3DDevice->GetVertexShaderConstantF(regId, f, 4);
			{
				float* pf = (float*)out;
				pf[1] = f[4];   pf[2] = f[8];   pf[3] = f[12];
				pf[4] = f[1];                    pf[6] = f[9];   pf[7] = f[13];
				pf[8] = f[2];   pf[9] = f[6];                    pf[11] = f[14];
				pf[12] = f[3];    pf[13] = f[7]; pf[14] = f[11];
			}
			break;
		default:
			lxAssertNeverReach("Unsupported shader variable type.");
		}
	} else {
		regId = registerPS;
		switch((core::Type::EType)type) {
		case core::Type::Bool:
		{
			m_D3DDevice->GetPixelShaderConstantB(regId, b, 1);
			*(bool*)out = (*b == TRUE);
		}
		break;
		case core::Type::Texture:
		case core::Type::Integer:
			m_D3DDevice->GetPixelShaderConstantI(regId, i, 1);
			*((int*)out) = *i;
			break;
		case core::Type::Float:
			m_D3DDevice->GetPixelShaderConstantF(regId, f, 1);
			*((float*)out) = *f;
			break;
		case core::Type::Vector2:
			m_D3DDevice->GetPixelShaderConstantF(regId, f, 1);
			((float*)out)[0] = f[0];
			((float*)out)[1] = f[1];
			break;
		case core::Type::Vector3:
			m_D3DDevice->GetPixelShaderConstantF(regId, f, 1);
			((float*)out)[0] = f[0];
			((float*)out)[1] = f[1];
			((float*)out)[2] = f[2];
			break;
		case core::Type::ColorF:
			m_D3DDevice->GetPixelShaderConstantF(regId, (float*)out, 1);
			break;
		case core::Type::Matrix:
			m_D3DDevice->GetPixelShaderConstantF(regId, (float*)out, 4);
			break;
		case core::Type::Internal_MatrixCol:
			m_D3DDevice->GetPixelShaderConstantF(regId, f, 4);
			{
				float* pf = (float*)out;
				pf[1] = f[4];   pf[2] = f[8];   pf[3] = f[12];
				pf[4] = f[1];                    pf[6] = f[9];   pf[7] = f[13];
				pf[8] = f[2];   pf[9] = f[6];                    pf[11] = f[14];
				pf[12] = f[3];    pf[13] = f[7]; pf[14] = f[11];
			}
			break;

		default:
			lxAssertNeverReach("Unsupported shader variable type.");
		}
	}
}

void ShaderD3D9::SetShaderValue(u32 registerVS, u32 registerPS, core::Type type, u32 size, const void* data)
{
	LUX_UNUSED(size);

	if((registerVS == -1 && registerPS == -1) || type == core::Type::Unknown)
		return;

	static u32 v[16];

	CastTypeToShader(type, data, v);

	u32 regId;
	HRESULT hr;
	if(registerVS != -1) {
		regId = registerVS;
		switch((core::Type::EType)type) {
		case core::Type::Bool:
			hr = m_D3DDevice->SetVertexShaderConstantB(regId, (BOOL*)v, 1);
			break;
		case core::Type::Texture:
		case core::Type::Integer:
			hr = m_D3DDevice->SetVertexShaderConstantI(regId, (int*)v, 1);
			break;
		case core::Type::Float:
		case core::Type::Vector2:
		case core::Type::Vector3:
		case core::Type::ColorF:
			hr = m_D3DDevice->SetVertexShaderConstantF(regId, (float*)v, 1);
			break;
		case core::Type::Matrix:
		case core::Type::Internal_MatrixCol:
			hr = m_D3DDevice->SetVertexShaderConstantF(regId, (float*)v, 4);
			break;

		default:
			lxAssertNeverReach("Unsupported shader variable type.");
		}
	}
	if(registerPS != -1) {
		regId = registerPS;
		switch((core::Type::EType)type) {
		case core::Type::Bool:
			hr = m_D3DDevice->SetPixelShaderConstantB(regId, (BOOL*)v, 1);
			break;
		case core::Type::Texture:
		case core::Type::Integer:
			hr = m_D3DDevice->SetPixelShaderConstantI(regId, (int*)v, 1);
			break;
		case core::Type::Float:
		case core::Type::Vector2:
		case core::Type::Vector3:
		case core::Type::ColorF:
			hr = m_D3DDevice->SetPixelShaderConstantF(regId, (float*)v, 1);
			break;

		case core::Type::Matrix:
		case core::Type::Internal_MatrixCol:
			hr = m_D3DDevice->SetPixelShaderConstantF(regId, (float*)v, 4);
			break;

		default:
			lxAssertNeverReach("Unsupported shader variable type.");
		}
	}
}

core::ParamPackage& ShaderD3D9::GetParamPackage()
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
	};

	for(u32 i = 0; i < sizeof(NAMES) / sizeof(*NAMES); ++i) {
		if(strcmp(NAMES[i], name) == 0)
			return (int)i;
	}

	return -1;
}

core::Type ShaderD3D9::GetDefaultType(u32 id)
{
	switch(id) {
	case DefaultParam_Shininess:
		return core::Type::Float;
	case DefaultParam_Diffuse:
	case DefaultParam_Emissive:
	case DefaultParam_Specular:
		return core::Type::ColorF;
	default:
		return core::Type::Unknown;
	}
}

bool ShaderD3D9::IsTypeCompatible(core::Type a, core::Type b)
{
	if(a == b)
		return true;
	if((a == core::Type::Internal_MatrixCol && b == core::Type::Matrix) || (a == core::Type::Matrix && b == core::Type::Internal_MatrixCol))
		return true;

	return false;
}

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
