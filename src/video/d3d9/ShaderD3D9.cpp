#ifdef LUX_COMPILE_WITH_D3D9
#include "core/Logger.h"
#include "video/VideoDriver.h"

#include "ShaderD3D9.h"

namespace lux
{
namespace video
{

ShaderParam& ShaderParam::operator= (const ShaderParam& other)
{
	m_pOwner = other.m_pOwner;
	m_pName = other.m_pName;
	m_Register_PS = other.m_Register_PS;
	m_Register_VS = other.m_Register_VS;
	m_Type = other.m_Type;
	m_TypeSize = other.m_TypeSize;

	return *this;
}

ShaderImpl::ShaderImpl(VideoDriver* Driver)
	: m_D3DDevice((IDirect3DDevice9*)Driver->GetDevice()),
	m_paramInvalid(this, core::Type::Unknown, 0, nullptr, 0xFFFFFFFF, 0xFFFFFFFF),
	m_pVertexShader(nullptr), m_pPixelShader(nullptr),
	m_pPixelShaderConstants(nullptr), m_pVertexShaderConstants(nullptr),
	m_pNames(nullptr), m_SceneValues(Driver->GetSceneValues())
{
}

ShaderImpl::~ShaderImpl()
{
	if(m_pVertexShader)
		m_pVertexShader->Release();

	if(m_pPixelShader)
		m_pPixelShader->Release();

	LUX_FREE_ARRAY(m_pNames);
}

bool ShaderImpl::Init(const char* pcVSCode, const char* pcVSEntryPoint, size_t VSLength, const char* pcVSProfile,
	const char* pcPSCode, const char* pcPSEntryPoint, size_t PSLength, const char* pcPSProfile)
{
	if(!pcVSCode || !pcPSCode)    false;

	// Länge anpassen
	if(VSLength == 0) VSLength = strlen(pcVSCode);
	if(PSLength == 0) PSLength = strlen(pcPSCode);

	// Wenn keine Einsprungpunkte angegeben sind Standardnamen verwenden
	if(!pcVSEntryPoint) pcVSEntryPoint = "mainVS";
	if(!pcPSEntryPoint) pcPSEntryPoint = "mainPS";

	// Vertexshader erstellen
	if(!CreateVertexShader(pcVSCode, pcVSEntryPoint, VSLength, pcVSProfile))
		return false;

	// Pixelshader erstellen
	if(pcPSCode) {
		if(!CreatePixelShader(pcPSCode, pcPSEntryPoint, PSLength, pcPSProfile))
			return false;
	}

	// Gehe die Param und Scene- Strukturen beider Shader durch und lade die Parameter
	core::array<SHelperEntry> Helper;
	u32 NameMemory = 0;
	u32 ParamCount = 0;
	u32 SceneCount = 0;
	if(m_pVertexShader)
		ParamCount += LoadParams(m_pVertexShaderConstants, true, Helper, NameMemory, ParamCount, SceneCount);
	if(m_pPixelShader)
		ParamCount += LoadParams(m_pPixelShaderConstants, true, Helper, NameMemory, ParamCount, SceneCount);
	if(m_pVertexShader)
		SceneCount += LoadParams(m_pVertexShaderConstants, false, Helper, NameMemory, ParamCount, SceneCount);
	if(m_pPixelShader)
		SceneCount += LoadParams(m_pPixelShaderConstants, false, Helper, NameMemory, ParamCount, SceneCount);

	m_MaterialParamCount = ParamCount;
	m_pNames = LUX_NEW_ARRAY(char, NameMemory);
	u32 NameCursor = 0;
	m_Params.Reserve(Helper.Size());
	u32 count = (u32)Helper.Size();
	if(count > 0xFFFF) {
		assertNeverReach("Helper array overflow");
		count = 0xFFFF;
	}
	for(u32 i = 0; i < count; ++i) {
		SParamEntry entry;
		const SHelperEntry& h = Helper[i];
		if(i >= m_MaterialParamCount) {
			// Its a scene values
			entry.index = m_SceneValues->GetParamID(h.pName);
			if(entry.index == 0xFFFFFFFF)
				log::Warning("Unknown scene value in shader: ~s.", h.pName);
		} else {
			entry.index = 0xFFFFFFFF;
			// Its a param value
			static u32 f[16];
			if(h.default)
				CastShaderToType(h.type, h.default, f);

			// Create Package elem, and link it with shaderparam <ParamID>
			m_ParamPackage.AddParam(h.type, h.pName, f, (u16)i);
		}

		memcpy(m_pNames + NameCursor, Helper[i].pName, Helper[i].NameLength);

		entry.Param = ShaderParam(this, h.type, h.TypeSize, m_pNames + NameCursor, h.Register_VS, h.Register_PS);
		m_Params.Push_Back(std::move(entry));

		NameCursor += h.NameLength;
	}

	Helper.Clear();

	if(m_pVertexShader) {
		m_pVertexShaderConstants->Release();
		m_pVertexShaderConstants = nullptr;
	}

	if(m_pPixelShader) {
		m_pPixelShaderConstants->Release();
		m_pPixelShaderConstants = nullptr;
	}

	return true;
}

u32 ShaderImpl::LoadParams(ID3DXConstantTable* pFrom, bool IsParam, core::array<SHelperEntry>& target, u32& StringSize, u32 ParamCount, u32 SceneCount)
{
	const char* pStructureName = IsParam ? "param" : "scene";

	D3DXHANDLE hParam = pFrom->GetConstantByName(NULL, pStructureName);
	if(hParam == NULL)
		return 0;

	UINT Count = 1;
	D3DXCONSTANT_DESC SDesc;
	HRESULT hr = pFrom->GetConstantDesc(hParam, &SDesc, &Count);
	if(FAILED(hr))
		return 0;

	//target.SetSize(-(int)SDesc.StructMembers);

	// Namen müssen gesucht werden von
	/*
	u32 start = ParamCount + SceneCount;
	u32 end = target.Size();
	*/
	u32 start;
	u32 end;
	u32 NewParams = 0;
	if(IsParam) {
		start = 0;
		end = ParamCount;
	} else {
		start = ParamCount;
		end = ParamCount + SceneCount;
	}

	for(u32 i = 0; i < SDesc.StructMembers; ++i) {
		u32 Size, Register;
		core::Type type;
		const char* name;
		const void* default;
		GetStructureElemType(hParam, i, pFrom, type, Size, Register, name, default);
		if(name[0] == '_')
			continue;

		bool Found = false;
		for(u32 j = start; j < end; ++j) {
			if(strcmp(target[j].pName, name) == 0) {
				if(pFrom == m_pVertexShaderConstants)
					target[j].Register_VS = Register;
				else
					target[j].Register_PS = Register;

				Found = true;
				break;
			}
		}

		if(!Found) {
			SHelperEntry HEntry;
			HEntry.NameLength = (u32)strlen(name) + 1;
			HEntry.pName = name;
			HEntry.type = type;
			HEntry.TypeSize = (u8)Size;
			HEntry.default = default;
			if(pFrom == m_pVertexShaderConstants)
				HEntry.Register_VS = Register;
			else
				HEntry.Register_PS = Register;
			target.Push_Back(HEntry);
			StringSize += HEntry.NameLength;
			NewParams++;
		}
	}

	return NewParams;
}

bool ShaderImpl::CreateVertexShader(const char* pcCode, const char* pcEntryPoint, size_t length, const char* pcProfile)
{
	ID3DXBuffer* pOutput = 0;
	ID3DXBuffer* pErrors = 0;

	HRESULT hr = D3DXCompileShader(pcCode, (UINT)length,
		NULL, NULL, pcEntryPoint,
		pcProfile,
		0, &pOutput, &pErrors,
		&m_pVertexShaderConstants);
	if(FAILED(hr)) {
		log::Error("Failed to compile vertexshader.");

		// Die Fehlermeldung ausgeben
		if(pErrors) {
			log::Error((const char*)(pErrors->GetBufferPointer()));
			pErrors->Release();
		}
		return false;
	}

	if(pErrors) {
		log::Warning((const char*)(pErrors->GetBufferPointer()));
		pErrors->Release();
		pErrors = 0;
	}

	if(FAILED(m_D3DDevice->CreateVertexShader((DWORD*)(pOutput->GetBufferPointer()), &m_pVertexShader))) {
		log::Error("Failed to compile vertexshader.");
		return false;
	}

	pOutput->Release();
	pOutput = 0;

	return true;
}

bool ShaderImpl::CreatePixelShader(const char* pcCode, const char* pcEntryPoint, size_t length, const char* pcProfile)
{
	ID3DXBuffer* pOutput = 0;
	ID3DXBuffer* pErrors = 0;

	if(FAILED(D3DXCompileShader(pcCode, (UINT)length,
		NULL, NULL, pcEntryPoint,
		pcProfile,
		0, &pOutput, &pErrors,
		&m_pPixelShaderConstants))) {
		log::Error("Failed to compile pixelshader.");

		if(pErrors) {
			// Die Fehlermeldung ausgeben
			log::Error((const char*)(pErrors->GetBufferPointer()));
			pErrors->Release();
		}
		return false;
	}

	if(pErrors) {
		log::Warning((const char*)(pErrors->GetBufferPointer()));
		pErrors->Release();
		pErrors = 0;
	}


	if(FAILED(m_D3DDevice->CreatePixelShader((DWORD*)(pOutput->GetBufferPointer()), &m_pPixelShader))) {
		log::Error("Failed to create pixelshader.");
		pOutput->Release();
		return false;
	}

	pOutput->Release();

	return true;
}

const ShaderParam& ShaderImpl::GetParam(const char* pcName)
{
	for(u32 i = 0; i < m_Params.Size(); ++i) {
		if(strcmp(m_Params[i].Param.GetName(), pcName) == 0) {
			return m_Params[i].Param;
		}
	}

	return m_paramInvalid;
}

const ShaderParam& ShaderImpl::GetParam(u32 index)
{
	if(index < m_Params.Size())
		return m_Params[index].Param;

	return m_paramInvalid;
}

u32 ShaderImpl::GetParamCount() const
{
	return (u32)m_Params.Size();
}

void ShaderImpl::Enable()
{
	HRESULT hr = m_D3DDevice->SetVertexShader(m_pVertexShader);
	hr = m_D3DDevice->SetPixelShader(m_pPixelShader);
}

void ShaderImpl::LoadParams(const core::PackagePuffer& Puffer)
{
	// Load Params
	const core::ParamPackage* pack = Puffer.GetType();
	if(pack != nullptr) {
		core::ParamPackage::ParamDesc Desc;
		for(u32 i = 0; i < pack->GetParamCount(); ++i) {
			if(pack->GetParamDesc(i, Desc)) {
				if(Desc.reserved != 0xFFFF) {
					// Its a shader param
					ShaderParam& param = m_Params[Desc.reserved].Param;
					param.SetShaderValue(Puffer.FromID(i, true).Pointer());
				}
			}
		}
	}

	// Load Scenevalues
	for(size_t i = m_MaterialParamCount; i < m_Params.Size(); ++i) {
		if(m_Params[i].IsSceneValue()) {
			m_Params[i].Param.SetShaderValue(m_SceneValues->GetParamValue(m_Params[i].index));
		}
	}
}

void ShaderImpl::Disable()
{
	m_D3DDevice->SetVertexShader(NULL);
	m_D3DDevice->SetPixelShader(NULL);
}

void ShaderImpl::GetStructureElemType(D3DXHANDLE hStruct, u32 index, ID3DXConstantTable* pTable, core::Type& outType, u32& outSize, u32& registerID, const char*& name, const void*& default)
{
	D3DXCONSTANT_DESC Desc;
	UINT Count = 1;
	D3DXHANDLE Handle = pTable->GetConstant(hStruct, index);

	pTable->GetConstantDesc(Handle, &Desc, &Count);

	outType = core::Type::Unknown;
	if(Desc.Class == D3DXPC_STRUCT)
		outType = core::Type::Internal_Composed;
	else if(Desc.Class == D3DXPC_SCALAR) {
		if(Desc.Type == D3DXPT_BOOL)
			outType = core::Type::Bool;
		else if(Desc.Type == D3DXPT_INT)
			outType = core::Type::Integer;
		else if(Desc.Type == D3DXPT_FLOAT)
			outType = core::Type::Float;
	} else if(Desc.Class == D3DXPC_VECTOR) {
		if(Desc.Type == D3DXPT_FLOAT) {
			if(Desc.Columns == 2)
				outType = core::Type::Vector2;
			else if(Desc.Columns == 3)
				outType = core::Type::Vector3;
			else if(Desc.Columns == 4)
				outType = core::Type::ColorF;
		}
	} else if(Desc.Class == D3DXPC_MATRIX_ROWS && Desc.Rows == 4 && Desc.Columns == 4) {
		if(Desc.Type == D3DXPT_FLOAT)
			outType = core::Type::Matrix;
	} else if(Desc.Class == D3DXPC_MATRIX_COLUMNS && Desc.Rows == 4 && Desc.Columns == 4) {
		if(Desc.Type == D3DXPT_FLOAT)
			outType = core::Type::Internal_MatrixCol;
	} else if(Desc.Class == D3DXPC_OBJECT) {
		if(Desc.Type == D3DXPT_SAMPLER)
			outType = core::Type::Texture;
	}

	outSize = Desc.Bytes;
	registerID = Desc.RegisterIndex;
	name = Desc.Name;
	default = Desc.DefaultValue;
}

void ShaderImpl::CastTypeToShader(core::Type type, const void* in, void* out)
{
	switch((core::Type::EType)type) {
	case core::Type::Bool:
		((BOOL*)out)[0] = (*(bool*)in) ? TRUE : FALSE;
		//((BOOL*)out)[1] = FALSE;
		//((BOOL*)out)[2] = FALSE;
		//((BOOL*)out)[3] = FALSE;
		break;
	case core::Type::Texture:
	case core::Type::Integer:
		((int*)out)[0] = *(int*)in;
		//((int*)out)[1] = 0;
		//((int*)out)[2] = 0;
		//((int*)out)[3] = 0;
		break;
		/*
		case core::Type::Float:
			((float*)out)[0] = ((float*)in)[0];
			//((float*)out)[1] = 0.0f;
			//((float*)out)[2] = 0.0f;
			//((float*)out)[3] = 0.0f;
			break;
		case core::Type::Vector2:
			((float*)out)[0] = ((float*)in)[0];
			((float*)out)[1] = ((float*)in)[1];
			//((float*)out)[2] = 0.0f;
			//((float*)out)[3] = 0.0f;
			break;
		case core::Type::Vector3:
			((float*)out)[0] = ((float*)in)[0];
			((float*)out)[1] = ((float*)in)[1];
			((float*)out)[2] = ((float*)in)[2];
			//((float*)out)[3] = 0.0f;
			break;
		case core::Type::ColorF:
			((float*)out)[0] = ((float*)in)[0];
			((float*)out)[1] = ((float*)in)[1];
			((float*)out)[2] = ((float*)in)[2];
			((float*)out)[3] = ((float*)in)[3];
			break;
		*/
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
		assertNeverReach("Unsupported shader variable type.");
	}
}

void ShaderImpl::CastShaderToType(core::Type type, const void* in, void* out)
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
		assertNeverReach("Unsupported shader variable type.");
	}
}

void ShaderImpl::GetShaderValue(u32 RegisterVS, u32 RegisterPS, core::Type type, u32 Size, void* out)
{
	LUX_UNUSED(Size);

	if((RegisterVS == -1 && RegisterPS == -1) || type == core::Type::Unknown)
		return;

	int i[4];
	float f[16];
	BOOL b[4];
	u32 Register;
	if(RegisterVS != -1) {
		Register = RegisterVS;
		switch((core::Type::EType)type) {
		case core::Type::Bool:
		{
			m_D3DDevice->GetVertexShaderConstantB(Register, b, 1);
			*(bool*)out = (*b == TRUE);
		}
		break;
		case core::Type::Texture:
		case core::Type::Integer:
			m_D3DDevice->GetVertexShaderConstantI(Register, i, 1);
			*((int*)out) = *i;
			break;
		case core::Type::Float:
			m_D3DDevice->GetVertexShaderConstantF(Register, f, 1);
			*((float*)out) = *f;
			break;
		case core::Type::Vector2:
			m_D3DDevice->GetVertexShaderConstantF(Register, f, 1);
			((float*)out)[0] = f[0];
			((float*)out)[1] = f[1];
			break;
		case core::Type::Vector3:
			m_D3DDevice->GetVertexShaderConstantF(Register, f, 1);
			((float*)out)[0] = f[0];
			((float*)out)[1] = f[1];
			((float*)out)[2] = f[2];
			break;
		case core::Type::ColorF:
			m_D3DDevice->GetVertexShaderConstantF(Register, (float*)out, 1);
			break;
		case core::Type::Matrix:
			m_D3DDevice->GetVertexShaderConstantF(Register, (float*)out, 4);
			break;
		case core::Type::Internal_MatrixCol:
			m_D3DDevice->GetVertexShaderConstantF(Register, f, 4);
			{
				float* pf = (float*)out;
				pf[1] = f[4];   pf[2] = f[8];   pf[3] = f[12];
				pf[4] = f[1];                    pf[6] = f[9];   pf[7] = f[13];
				pf[8] = f[2];   pf[9] = f[6];                    pf[11] = f[14];
				pf[12] = f[3];    pf[13] = f[7]; pf[14] = f[11];
			}
			break;
		default:
			assertNeverReach("Unsupported shader variable type.");
		}
	} else {
		Register = RegisterPS;
		switch((core::Type::EType)type) {
		case core::Type::Bool:
		{
			m_D3DDevice->GetPixelShaderConstantB(Register, b, 1);
			*(bool*)out = (*b == TRUE);
		}
		break;
		case core::Type::Texture:
		case core::Type::Integer:
			m_D3DDevice->GetPixelShaderConstantI(Register, i, 1);
			*((int*)out) = *i;
			break;
		case core::Type::Float:
			m_D3DDevice->GetPixelShaderConstantF(Register, f, 1);
			*((float*)out) = *f;
			break;
		case core::Type::Vector2:
			m_D3DDevice->GetPixelShaderConstantF(Register, f, 1);
			((float*)out)[0] = f[0];
			((float*)out)[1] = f[1];
			break;
		case core::Type::Vector3:
			m_D3DDevice->GetPixelShaderConstantF(Register, f, 1);
			((float*)out)[0] = f[0];
			((float*)out)[1] = f[1];
			((float*)out)[2] = f[2];
			break;
		case core::Type::ColorF:
			m_D3DDevice->GetPixelShaderConstantF(Register, (float*)out, 1);
			break;
		case core::Type::Matrix:
			m_D3DDevice->GetPixelShaderConstantF(Register, (float*)out, 4);
			break;
		case core::Type::Internal_MatrixCol:
			m_D3DDevice->GetPixelShaderConstantF(Register, f, 4);
			{
				float* pf = (float*)out;
				pf[1] = f[4];   pf[2] = f[8];   pf[3] = f[12];
				pf[4] = f[1];                    pf[6] = f[9];   pf[7] = f[13];
				pf[8] = f[2];   pf[9] = f[6];                    pf[11] = f[14];
				pf[12] = f[3];    pf[13] = f[7]; pf[14] = f[11];
			}
			break;

		default:
			assertNeverReach("Unsupported shader variable type.");
		}
	}
}

void ShaderImpl::SetShaderValue(u32 RegisterVS, u32 RegisterPS, core::Type type, u32 Size, const void* data)
{
	LUX_UNUSED(Size);

	if((RegisterVS == -1 && RegisterPS == -1) || type == core::Type::Unknown)
		return;

	static u32 v[16];

	CastTypeToShader(type, data, v);

	u32 Register;
	HRESULT hr;
	if(RegisterVS != -1) {
		Register = RegisterVS;
		switch((core::Type::EType)type) {
		case core::Type::Bool:
			hr = m_D3DDevice->SetVertexShaderConstantB(Register, (BOOL*)v, 1);
			break;
		case core::Type::Texture:
		case core::Type::Integer:
			hr = m_D3DDevice->SetVertexShaderConstantI(Register, (int*)v, 1);
			break;
		case core::Type::Float:
		case core::Type::Vector2:
		case core::Type::Vector3:
		case core::Type::ColorF:
			hr = m_D3DDevice->SetVertexShaderConstantF(Register, (float*)v, 1);
			break;
		case core::Type::Matrix:
		case core::Type::Internal_MatrixCol:
			hr = m_D3DDevice->SetVertexShaderConstantF(Register, (float*)v, 4);
			break;

		default:
			assertNeverReach("Unsupported shader variable type.");
		}
	}
	if(RegisterPS != -1) {
		Register = RegisterPS;
		switch((core::Type::EType)type) {
		case core::Type::Bool:
			hr = m_D3DDevice->SetPixelShaderConstantB(Register, (BOOL*)v, 1);
			break;
		case core::Type::Texture:
		case core::Type::Integer:
			hr = m_D3DDevice->SetPixelShaderConstantI(Register, (int*)v, 1);
			break;
		case core::Type::Float:
		case core::Type::Vector2:
		case core::Type::Vector3:
		case core::Type::ColorF:
			hr = m_D3DDevice->SetPixelShaderConstantF(Register, (float*)v, 1);
			break;

		case core::Type::Matrix:
		case core::Type::Internal_MatrixCol:
			hr = m_D3DDevice->SetPixelShaderConstantF(Register, (float*)v, 4);
			break;

		default:
			assertNeverReach("Unsupported shader variable type.");
		}
	}
}

#if 0
/*
ShaderParam ShaderImpl::HandleParam(D3DXHANDLE Handle, bool VertexShader)
{
	0 = structure,
	1 = bool
	2 = int
	3 = float
	4 = vec2
	5 = vec3
	6 = colorf
	7 = matrix_row
	8 = matrix_col

	if(type != -1)
		return ShaderParam(this, VertexShader, Desc.RegisterIndex, type, Desc.bytes, Desc.Elements, Handle);
	else
		return m_paramInvalid;
}
*/

//******************************************************************
//Fragt einen Parameter ab
/*
ShaderParam ShaderImpl::Param(const char* pcName, bool VertexShader)
{
	D3DXHANDLE Handle;
	if(VertexShader)
	{
		if(m_pVertexShaderConstants)
			Handle = m_pVertexShaderConstants->GetConstantByName(0, pcName);
	}
	else
	{
		if(m_pPixelShaderConstants)
			Handle = m_pPixelShaderConstants->GetConstantByName(0, pcName);
	}

	if(!Handle)
		return m_paramInvalid;
	else
		return HandleParam(Handle, VertexShader);
}

ShaderParam ShaderImpl::IndexParam(const ShaderParam* Param, int index)
{
	D3DXHANDLE Handle;
	if(Param->IsVertexParam())
	{
		if(m_pVertexShaderConstants)
			Handle = m_pVertexShaderConstants->GetConstantElement(Param->GetHandle(), index);
	}
	else
	{
		if(m_pPixelShaderConstants)
			Handle = m_pPixelShaderConstants->GetConstantElement(Param->GetHandle(), index);
	}

	if(!Handle)
		return m_paramInvalid;
	else
		return HandleParam(Handle, Param->IsVertexParam());
}

ShaderParam ShaderImpl::ElemParam(const ShaderParam* Param, const char* pcName)
{
	D3DXHANDLE Handle;
	if(Param->IsVertexParam())
	{
		if(m_pVertexShaderConstants)
			Handle = m_pVertexShaderConstants->GetConstantByName(Param->GetHandle(), pcName);
	}
	else
	{
		if(m_pPixelShaderConstants)
			Handle = m_pPixelShaderConstants->GetConstantByName(Param->GetHandle(), pcName);
	}

	if(!Handle)
		return m_paramInvalid;
	else
		return HandleParam(Handle, Param->IsVertexParam());
}
*/

#endif
}

}


#endif // LUX_COMPILE_WITH_D3D9
