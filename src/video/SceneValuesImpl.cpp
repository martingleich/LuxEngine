#include "SceneValuesImpl.h"
#include "core/StringConverter.h"

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

	// The invalid light, is a disabled light.
	m_InvalidLight(0, 0) = 0.0f;
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

bool SceneValuesImpl::SetLight(u32 id, const video::LightData& light)
{
	math::matrix4 m;
	if(light.type == video::LightData::EType::Directional)
		m(0, 3) = 1.0f;
	else if(light.type == video::LightData::EType::Point)
		m(0, 3) = 2.0f;
	else if(light.type == video::LightData::EType::Spot)
		m(0, 3) = 3.0f;
	else
		return false;

	m(0, 0) = light.color.r;
	m(0, 1) = light.color.g;
	m(0, 2) = light.color.b;

	m(1, 0) = light.position.x;
	m(1, 1) = light.position.y;
	m(1, 2) = light.position.z;

	m(2, 0) = light.direction.x;
	m(2, 1) = light.direction.y;
	m(2, 2) = light.direction.z;

	m(3, 0) = light.range;
	m(3, 1) = light.falloff;
	m(3, 2) = light.innerCone;
	m(3, 3) = light.outerCone;

	m(1, 3) = 0.0f;
	m(2, 3) = 0.0f;

	m_Lights.Resize(id + 1);
	m_Lights[id].name = "lightData" + core::StringConverter::ToString(id);
	m_Lights[id].matrix = m;

	return true;
}

void SceneValuesImpl::ClearLights()
{
	for(auto it = m_Lights.First(); it != m_Lights.End(); ++it) {
		it->matrix(0, 3) = 0.0f;
	}
}

u32 SceneValuesImpl::AddParam(const string& name, core::Type type)
{
	int id = GetParamID(name);
	if(id != -1) {
		lxAssertEx(id != -1, "Param already used");
		return 0xFFFFFFFF;
	}

	lxAssertEx(type.GetSize() != 0, "Invalid type");
	SParam Param(name, m_CurrentOffset, type, type.GetSize());
	m_Params.Push_Back(Param);

	if(m_CurrentOffset + Param.Size > m_MaxSize)
		Realloc((m_CurrentOffset + Param.Size) * 2);

	m_CurrentOffset += Param.Size;

	return (u32)(m_Params.Size() + MATRIX_COUNT - 1);
}

/*
void SceneValuesImpl::RemoveParam(u32 id)
{
	if(id >= m_Params.Size() || id < 10)
		return;

	const SParam& p = m_Params[id-10];

	u32 After = m_CurrentOffset - p.offset - p.Size;
	if(After != 0)
		memmove((u8*)m_ParamData+p.offset, (u8*)m_ParamData+p.offset+p.Size, After);

	for(u32 i = id-10; i < m_Params.Size(); ++i)
		m_Params[i].offset -= p.Size;

	m_CurrentOffset -= p.Size;
	m_Params.DeleteEntry(m_Params.First()+id-10, true);
}

void SceneValuesImpl::RemoveAllParams()
{
	m_CurrentOffset = 0;
	m_Params.Clear();
}
*/

u32 SceneValuesImpl::GetParamCount() const
{
	return (u32)(m_Params.Size() + MATRIX_COUNT);
}

u32 SceneValuesImpl::GetParamID(const string& name) const
{
	// Check matrizes
	u32 first = *name.First();
	if(first == 'w' || first == 'v' || first == 'p') {
		for(u32 i = 0; i < MATRIX_COUNT; ++i)
			if(MATRIX_NAMES[i] == name)
				return i;
	}

	if(name.StartsWith("lightData")) {
		const char* next;
		int id = core::StringConverter::ParseInt((name.First() + 9).Pointer(), -1, &next);
		if(id >= 0 && next == name.End().Pointer()) {
			return FIRST_LIGHT_ID + (u32)id;
		}
	}

	for(u32 i = 0; i < m_Params.Size(); ++i)
		if(m_Params[i].name == name)
			return i + MATRIX_COUNT;

	return 0xFFFFFFFF;
}

const string& SceneValuesImpl::GetParamName(u32 id) const
{
	if(id < MATRIX_COUNT) {
		return MATRIX_NAMES[id];
	} else if(id >= FIRST_LIGHT_ID) {
		return m_Lights[id - FIRST_LIGHT_ID].name;
	} else {
		return m_Params[id - MATRIX_COUNT].name;
	}
}

core::Type SceneValuesImpl::GetParamType(u32 id) const
{
	if(id < MATRIX_COUNT)
		return core::Type::Matrix;
	else if(id >= FIRST_LIGHT_ID)
		return core::Type::Matrix;
	else
		return m_Params[id - MATRIX_COUNT].type;
}

const void* SceneValuesImpl::GetParamValue(u32 id) const
{
	if(id < MATRIX_COUNT) {
		return &GetMatrix((EMatrizes)id);
	} else if(id >= FIRST_LIGHT_ID) {
		id -= FIRST_LIGHT_ID;
		if(id < m_Lights.Size())
			return m_Lights[id].matrix.DataRowMajor();
		return m_InvalidLight.DataRowMajor();
	} else {
		return (u8*)m_ParamData + m_Params[id - MATRIX_COUNT].offset;
	}
}

void SceneValuesImpl::SetParamValue(u32 id, const void* p)
{
	if(id < MATRIX_COUNT) {
		SetMatrix((EMatrizes)id, *(const math::matrix4*)p);
	} else if(id >= FIRST_LIGHT_ID) {
		id -= FIRST_LIGHT_ID;
		if(id < m_Lights.Size())
			m_Lights[id].matrix = *(const math::matrix4*)p;
	} else {
		memcpy((u8*)m_ParamData + m_Params[id - MATRIX_COUNT].offset, p, m_Params[id - MATRIX_COUNT].Size);
	}
}

u32 SceneValuesImpl::GetSysMatrixCount() const
{
	return 16;
}

}
}