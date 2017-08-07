#include "video/MatrixTable.h"

namespace lux
{
namespace video
{

MatrixTable::MatrixTable() :
	m_Dirty(0xFFFFFFFF)
{
	m_UpToDate = 1 | 2 | 4; // world, view and proj are up to date
}

void MatrixTable::SetMatrix(EMatrixType type, const math::Matrix4& matrix)
{
	if(type > MAT_PROJ)
		throw core::InvalidArgumentException("type");

	m_Matrices[type] = matrix;
	switch(type) {
	case MAT_WORLD:
		// Invalide all matrices containing world
		m_UpToDate &= ~(8 | 32 | 64 | 256 | 512 | 1024 | 4096 | 16384 | 32768);
		m_UpToDate |= 1; // But world is up to date
		m_Dirty |= ~m_UpToDate;
		m_Dirty |= 1;
		break;
	case MAT_VIEW:
		// Invalide all matrices containing view
		m_UpToDate &= ~(16 | 32 | 128 | 256 | 512 | 2048 | 8192 | 16384 | 32768);
		m_UpToDate |= 2; // But view is up to data
		m_Dirty |= ~m_UpToDate;
		m_Dirty |= 2;
		break;
	case MAT_PROJ:
		// Invalide all matrices containing proj
		m_UpToDate &= ~(64 | 128 | 256);
		m_UpToDate |= 4; // But proj is up to date
		m_Dirty |= ~m_UpToDate;
		m_Dirty |= 4;
		break;
	default:
		return;
	}
}

core::VariableAccess MatrixTable::GetParamById(u32 id) const
{
	const math::Matrix4& m = GetMatrix((EMatrixType)id);
	return core::VariableAccess(core::Types::Matrix().GetConstantType(), GetMatrixName((EMatrixType)id), const_cast<float*>(m.DataRowMajor()));
}

bool MatrixTable::GetParamIdByName(const char* name, u32& id)
{
	for(id = 0; id < GetCount(); ++id) {
		if(strcmp(GetMatrixName(id), name) == 0)
			return true;
	}

	return false;
}

const math::Matrix4& MatrixTable::GetMatrix(EMatrixType type) const
{
	if(!IsUpToDate(type))
		UpdateMatrix(type);

	return m_Matrices[type];
}

bool MatrixTable::IsDirty(EMatrixType type) const
{
	return (m_Dirty && (1 << type)) != 0;
}

void MatrixTable::ClearDirty(EMatrixType type) const
{
	m_Dirty &= ~(1 << type);
}

u32 MatrixTable::GetCount() const
{
	return MAT_COUNT;
}

bool MatrixTable::IsUpToDate(EMatrixType type) const
{
	return (m_UpToDate & (1 << type)) != 0;
}

void MatrixTable::UpdateMatrix(EMatrixType type) const
{
	switch(type) {
	case MAT_WORLD:
		lxAssert(IsUpToDate(MAT_WORLD_INV));
		m_Matrices[MAT_WORLD] = m_Matrices[MAT_WORLD_INV].GetTransformInverted();
		break;
	case MAT_VIEW:
		lxAssert(IsUpToDate(MAT_VIEW_INV));
		m_Matrices[MAT_VIEW] = m_Matrices[MAT_VIEW_INV].GetTransformInverted();
		break;
	case MAT_WORLD_INV:
		lxAssert(IsUpToDate(MAT_WORLD));
		m_Matrices[MAT_WORLD_INV] = m_Matrices[MAT_WORLD].GetTransformInverted();
		break;
	case MAT_VIEW_INV:
		lxAssert(IsUpToDate(MAT_VIEW));
		m_Matrices[MAT_VIEW_INV] = m_Matrices[MAT_VIEW].GetTransformInverted();
		break;
	case MAT_WORLD_VIEW:
		m_Matrices[MAT_WORLD_VIEW].SetByProduct(GetMatrix(MAT_WORLD), GetMatrix(MAT_WORLD));
		break;
	case MAT_WORLD_PROJ:
		m_Matrices[MAT_WORLD_PROJ].SetByProduct(GetMatrix(MAT_WORLD), GetMatrix(MAT_PROJ));
		break;
	case MAT_VIEW_PROJ:
		m_Matrices[MAT_VIEW_PROJ].SetByProduct(GetMatrix(MAT_VIEW), GetMatrix(MAT_PROJ));
		break;
	case MAT_WORLD_VIEW_PROJ:
		if(IsUpToDate(MAT_VIEW_PROJ))
			m_Matrices[MAT_WORLD_VIEW_PROJ].SetByProduct(GetMatrix(MAT_WORLD), GetMatrix(MAT_VIEW_PROJ));
		else if(IsUpToDate(MAT_WORLD_VIEW))
			m_Matrices[MAT_WORLD_VIEW_PROJ].SetByProduct(GetMatrix(MAT_WORLD_VIEW), GetMatrix(MAT_PROJ));
		else {
			m_Matrices[MAT_WORLD_VIEW].SetByProduct(GetMatrix(MAT_WORLD), GetMatrix(MAT_VIEW));
			m_UpToDate |= 1 << MAT_WORLD_VIEW;
			m_Matrices[MAT_WORLD_VIEW_PROJ].SetByProduct(GetMatrix(MAT_WORLD_VIEW), GetMatrix(MAT_PROJ));
		}
		break;
	case MAT_WORLD_VIEW_INV:
		if(IsUpToDate(MAT_WORLD_VIEW)) {
			m_Matrices[MAT_WORLD_VIEW].SetByProduct(GetMatrix(MAT_WORLD), GetMatrix(MAT_VIEW));
			m_UpToDate |= 1 << MAT_WORLD_VIEW;
		}
		m_Matrices[MAT_WORLD_VIEW_INV] = GetMatrix(MAT_WORLD_VIEW).GetTransformInverted();
		break;
	case MAT_WORLD_TRANS:
		m_Matrices[MAT_WORLD_TRANS] = GetMatrix(MAT_WORLD).GetTransposed();
		break;
	case MAT_VIEW_TRANS:
		m_Matrices[MAT_VIEW_TRANS] = GetMatrix(MAT_VIEW).GetTransposed();
		break;
	case MAT_WORLD_INV_TRANS:
		m_Matrices[MAT_WORLD_INV_TRANS] = GetMatrix(MAT_WORLD_INV).GetTransposed();
		break;
	case MAT_VIEW_INV_TRANS:
		m_Matrices[MAT_VIEW_INV_TRANS] = GetMatrix(MAT_VIEW_INV).GetTransposed();
		break;
	case MAT_WORLD_VIEW_TRANS:
		m_Matrices[MAT_WORLD_VIEW_TRANS] = GetMatrix(MAT_WORLD_VIEW).GetTransposed();
		break;
	case MAT_WORLD_VIEW_INV_TRANS:
		m_Matrices[MAT_WORLD_VIEW_INV_TRANS] = GetMatrix(MAT_WORLD_VIEW_INV).GetTransposed();
		break;
	default:
		lxAssertNeverReach("Can't generate matrix");
	}

	m_UpToDate |= (1 << type);
}

const char* MatrixTable::GetMatrixName(u32 id) const
{
	static const char* MATRIX_NAMES[16] = {
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

	return MATRIX_NAMES[id];
}

} // namespace video
} // namespace lux
