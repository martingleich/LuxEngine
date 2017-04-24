#include "SceneValuesImpl.h"

namespace lux
{
namespace scene
{

const string SceneValuesImpl::MATRIX_NAMES[16] = {
	"world",
	"view",
	"proj",
	"worldI",
	"viewI",
	"worldView",
	"worldProj",
	"viewProj",
	"worldViewProj",
	"worldViewI",
	"worldT",
	"viewT",
	"worldIT",
	"viewIT",
	"worldViewT",
	"worldViewIT",
};

void SceneValuesImpl::Realloc(u32 newSize)
{
	void* New = LUX_NEW_ARRAY(u8, newSize);
	if(m_ParamData) {
		memcpy(New, m_ParamData, m_CurrentOffset);
		LUX_FREE_ARRAY(m_ParamData);
	}

	m_ParamData = New;
	m_MaxSize = newSize;
}

bool SceneValuesImpl::IsUpToDate(EMatrizes type) const
{
	return ((m_UpdateMatrizes & (1 << type)) == 0);
}

SceneValuesImpl::SceneValuesImpl(bool AllMatrizesToIdent) : m_ParamData(nullptr), m_CurrentOffset(0), m_MaxSize(0)
{
	if(AllMatrizesToIdent)
		m_UpdateMatrizes = 0xFFFFFFFF;
	else
		m_UpdateMatrizes = 0;
}

SceneValuesImpl::~SceneValuesImpl()
{
	LUX_FREE_ARRAY(m_ParamData);
}

void SceneValuesImpl::SetMatrix(EMatrizes type, const math::matrix4& Matrix)
{
#if 0
	if(type <= MAT_PROJ) {
		m_Matrizes[type] = Matrix;
		switch(type) {
		case MAT_WORLD:
			m_UpdateMatrizes |= 8 | 32 | 64 | 256 | 512;
			break;
		case MAT_VIEW:
			m_UpdateMatrizes |= 16 | 32 | 128 | 256 | 512;
			break;
		case MAT_PROJ:
			m_UpdateMatrizes |= 64 | 128 | 256;
			break;
		}
	}
#else
	lxAssert(type <= MAT_VIEW_INV);
	if(type <= MAT_VIEW_INV) {
		m_Matrizes[type] = Matrix;
		switch(type) {
		case MAT_WORLD:
			m_UpdateMatrizes |= 8 | 32 | 64 | 256 | 512 | 1024 | 4096 | 16384 | 32768;
			m_UpdateMatrizes &= ~1;
			break;
		case MAT_VIEW:
			m_UpdateMatrizes |= 16 | 32 | 128 | 256 | 512 | 2048 | 8192 | 16384 | 32768;
			m_UpdateMatrizes &= ~2;
			break;
		case MAT_PROJ:
			m_UpdateMatrizes |= 64 | 128 | 256;
			m_UpdateMatrizes &= ~4;
			break;
		case MAT_WORLD_INV:
			m_UpdateMatrizes |= 1 | 32 | 64 | 256 | 512 | 1024 | 4096 | 16384 | 32768;
			m_UpdateMatrizes &= ~8;
			break;
		case MAT_VIEW_INV:
			m_UpdateMatrizes |= 2 | 32 | 128 | 256 | 512 | 2048 | 8192 | 16384 | 32768;
			m_UpdateMatrizes &= ~16;
		default:
			lxAssertNeverReach("There's something fishy.");
		}
	}
#endif
}

void SceneValuesImpl::SetMatrix(EMatrizes type, const math::matrix4& Matrix, const math::matrix4& InvMatrix)
{
	lxAssert(type <= MAT_VIEW);
	if(type <= MAT_VIEW) {
		m_Matrizes[type] = Matrix;
		m_Matrizes[type + 3] = InvMatrix;

		switch(type) {
		case MAT_WORLD:
			m_UpdateMatrizes |= 32 | 64 | 256 | 512 | 1024 | 4096 | 16384 | 32768;
			m_UpdateMatrizes &= ~(1 | 8);
			break;
		case MAT_VIEW:
			m_UpdateMatrizes |= 32 | 128 | 256 | 512 | 2048 | 8192 | 16384 | 32768;
			m_UpdateMatrizes &= ~(2 | 16);
			break;
		default:
			lxAssertNeverReach("There's something fishy.");
		}
	}
}

const math::matrix4& SceneValuesImpl::GetMatrix(EMatrizes type) const
{
	if(IsUpToDate(type)) {
		return m_Matrizes[type];
	} else {
		switch(type) {
		case MAT_WORLD:
			lxAssert(IsUpToDate(MAT_WORLD_INV));
			m_Matrizes[MAT_WORLD_INV].GetInverted(m_Matrizes[MAT_WORLD]);
			break;
		case MAT_VIEW:
			lxAssert(IsUpToDate(MAT_VIEW_INV));
			m_Matrizes[MAT_VIEW_INV].GetInverted(m_Matrizes[MAT_VIEW]);
			break;
		case MAT_WORLD_INV:
			lxAssert(IsUpToDate(MAT_WORLD));
			m_Matrizes[MAT_WORLD].GetInverted(m_Matrizes[MAT_WORLD_INV]);
			break;
		case MAT_VIEW_INV:
			lxAssert(IsUpToDate(MAT_VIEW));
			m_Matrizes[MAT_VIEW].GetInverted(m_Matrizes[MAT_VIEW_INV]);
			break;
		case MAT_WORLD_VIEW:
			m_Matrizes[MAT_WORLD_VIEW].SetByProduct(GetMatrix(MAT_WORLD), GetMatrix(MAT_WORLD));
			break;
		case MAT_WORLD_PROJ:
			m_Matrizes[MAT_WORLD_PROJ].SetByProduct(GetMatrix(MAT_WORLD), GetMatrix(MAT_PROJ));
			break;
		case MAT_VIEW_PROJ:
			m_Matrizes[MAT_VIEW_PROJ].SetByProduct(GetMatrix(MAT_VIEW), GetMatrix(MAT_PROJ));
			break;
		case MAT_WORLD_VIEW_PROJ:
			if(IsUpToDate(MAT_VIEW_PROJ))
				m_Matrizes[MAT_WORLD_VIEW_PROJ].SetByProduct(GetMatrix(MAT_WORLD), GetMatrix(MAT_VIEW_PROJ));
			else if(IsUpToDate(MAT_WORLD_VIEW))
				m_Matrizes[MAT_WORLD_VIEW_PROJ].SetByProduct(GetMatrix(MAT_WORLD_VIEW), GetMatrix(MAT_PROJ));
			else {
				m_Matrizes[MAT_WORLD_VIEW].SetByProduct(GetMatrix(MAT_WORLD), GetMatrix(MAT_VIEW));
				m_UpdateMatrizes ^= 1 << MAT_WORLD_VIEW;
				m_Matrizes[MAT_WORLD_VIEW_PROJ].SetByProduct(GetMatrix(MAT_WORLD_VIEW), GetMatrix(MAT_PROJ));
			}
			break;
		case MAT_WORLD_VIEW_INV:
			if(IsUpToDate(MAT_WORLD_VIEW)) {
				m_Matrizes[MAT_WORLD_VIEW].SetByProduct(GetMatrix(MAT_WORLD), GetMatrix(MAT_VIEW));
				m_UpdateMatrizes ^= 1 << MAT_WORLD_VIEW;
			}
			GetMatrix(MAT_WORLD_VIEW).GetInverted(m_Matrizes[MAT_WORLD_VIEW_INV]);
			break;
		case MAT_WORLD_TRANS:
			GetMatrix(MAT_WORLD).GetTransposed(m_Matrizes[MAT_WORLD_TRANS]);
			m_UpdateMatrizes ^= 1 << MAT_WORLD_TRANS;
			break;
		case MAT_VIEW_TRANS:
			GetMatrix(MAT_VIEW).GetTransposed(m_Matrizes[MAT_VIEW_TRANS]);
			m_UpdateMatrizes ^= 1 << MAT_VIEW_TRANS;
			break;
		case MAT_WORLD_INV_TRANS:
			GetMatrix(MAT_WORLD_INV).GetTransposed(m_Matrizes[MAT_WORLD_INV_TRANS]);
			m_UpdateMatrizes ^= 1 << MAT_WORLD_INV_TRANS;
			break;
		case MAT_VIEW_INV_TRANS:
			GetMatrix(MAT_VIEW_INV).GetTransposed(m_Matrizes[MAT_VIEW_INV_TRANS]);
			m_UpdateMatrizes ^= 1 << MAT_VIEW_INV_TRANS;
			break;
		case MAT_WORLD_VIEW_TRANS:
			GetMatrix(MAT_WORLD_VIEW).GetTransposed(m_Matrizes[MAT_WORLD_VIEW_TRANS]);
			m_UpdateMatrizes ^= 1 << MAT_WORLD_VIEW_TRANS;
			break;
		case MAT_WORLD_VIEW_INV_TRANS:
			GetMatrix(MAT_WORLD_VIEW_INV).GetTransposed(m_Matrizes[MAT_WORLD_VIEW_INV_TRANS]);
			m_UpdateMatrizes ^= 1 << MAT_WORLD_VIEW_INV_TRANS;
			break;
		default:
			lxAssertNeverReach("Can't generate matrix");
		}

		m_UpdateMatrizes ^= (1 << type);
		return m_Matrizes[type];
	}
}

u32 SceneValuesImpl::AddParam(const string& name, core::Type type)
{
	int id = GetParamID(name.Data());
	if(id != -1) {
		lxAssertEx(id != -1, "Param already used");
		return 0xFFFFFFFF;
	}

	lxAssertEx(type.GetSize() != 0, "Invalid type");
	SParam Param(name, m_CurrentOffset, type, type.GetSize());
	m_aParams.Push_Back(Param);

	if(m_CurrentOffset + Param.Size > m_MaxSize)
		Realloc((m_CurrentOffset + Param.Size) * 2);

	m_CurrentOffset += Param.Size;

	return (u32)(m_aParams.Size() + MATRIX_COUNT);
}

/*
void SceneValuesImpl::RemoveParam(u32 id)
{
	if(id >= m_aParams.Size() || id < 10)
		return;

	const SParam& p = m_aParams[id-10];

	u32 After = m_CurrentOffset - p.offset - p.Size;
	if(After != 0)
		memmove((u8*)m_ParamData+p.offset, (u8*)m_ParamData+p.offset+p.Size, After);

	for(u32 i = id-10; i < m_aParams.Size(); ++i)
		m_aParams[i].offset -= p.Size;

	m_CurrentOffset -= p.Size;
	m_aParams.DeleteEntry(m_aParams.First()+id-10, true);
}

void SceneValuesImpl::RemoveAllParams()
{
	m_CurrentOffset = 0;
	m_aParams.Clear();
}
*/
u32 SceneValuesImpl::GetParamCount() const
{
	return (u32)(m_aParams.Size() + MATRIX_COUNT);
}

u32 SceneValuesImpl::GetParamID(const char* pName) const
{
	// Check matrizes
	if(*pName == 'w' || *pName == 'v' || *pName == 'p') {
		for(u32 i = 0; i < 16; ++i)
			if(MATRIX_NAMES[i] == pName)
				return i;
	}

	for(u32 i = 0; i < m_aParams.Size(); ++i)
		if(m_aParams[i].name == pName)
			return i + MATRIX_COUNT;

	return 0xFFFFFFFF;
}

const string& SceneValuesImpl::GetParamName(u32 id) const
{
	if(id < MATRIX_COUNT) {
		return MATRIX_NAMES[id];
	} else {
		return m_aParams[id - MATRIX_COUNT].name;
	}
}

core::Type SceneValuesImpl::GetParamType(u32 id) const
{
	if(id < MATRIX_COUNT)
		return core::Type::Matrix;
	else
		return m_aParams[id - MATRIX_COUNT].type;
}

const void* SceneValuesImpl::GetParamValue(u32 id) const
{
	if(id < MATRIX_COUNT) {
		return &GetMatrix((EMatrizes)id);
	} else {
		return (u8*)m_ParamData + m_aParams[id - MATRIX_COUNT].offset;
	}
}

void SceneValuesImpl::SetParamValue(u32 id, const void* p)
{
	if(id < MATRIX_COUNT) {
		SetMatrix((EMatrizes)id, *(const math::matrix4*)p);
	} else {
		memcpy((u8*)m_ParamData + m_aParams[id - MATRIX_COUNT].offset, p, m_aParams[id - MATRIX_COUNT].Size);
	}
}

u32 SceneValuesImpl::GetSysMatrixCount() const
{
	return 16;
}

}
}