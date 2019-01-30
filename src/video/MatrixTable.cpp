#include "video/MatrixTable.h"

namespace lux
{
namespace video
{

MatrixTable::MatrixTable()
{
	for(int i = 0; i < MAT_COUNT; ++i)
		m_Matrices[i] = LUX_NEW(MatrixAttribute)(this, i);
}

static const int worldDependencies[] = {
	MatrixTable::MAT_WORLD,
	MatrixTable::MAT_WORLD_INV,
	MatrixTable::MAT_WORLD_VIEW,
	MatrixTable::MAT_WORLD_PROJ,
	MatrixTable::MAT_WORLD_VIEW_PROJ,
	MatrixTable::MAT_WORLD_VIEW_INV,
	MatrixTable::MAT_WORLD_TRANS,
	MatrixTable::MAT_WORLD_INV_TRANS,
	MatrixTable::MAT_WORLD_VIEW_TRANS,
	MatrixTable::MAT_WORLD_VIEW_INV_TRANS
};
static const int viewDependencies[] = {
	MatrixTable::MAT_VIEW,
	MatrixTable::MAT_VIEW_INV,
	MatrixTable::MAT_WORLD_VIEW,
	MatrixTable::MAT_VIEW_PROJ,
	MatrixTable::MAT_WORLD_VIEW_PROJ,
	MatrixTable::MAT_WORLD_VIEW_INV,
	MatrixTable::MAT_VIEW_TRANS,
	MatrixTable::MAT_VIEW_INV_TRANS,
	MatrixTable::MAT_WORLD_VIEW_TRANS,
	MatrixTable::MAT_WORLD_VIEW_INV_TRANS,
};
static const int projDependencies[] = {
	MatrixTable::MAT_PROJ,
	MatrixTable::MAT_WORLD_PROJ,
	MatrixTable::MAT_VIEW_PROJ,
	MatrixTable::MAT_WORLD_VIEW_PROJ,
};

void MatrixTable::SetMatrix(EMatrixType type, const math::Matrix4& matrix)
{
	if(type > MAT_PROJ)
		throw core::GenericInvalidArgumentException("type", "Can only set base matrices");

	m_Matrices[type]->value = matrix;
	switch(type) {
	case MAT_WORLD:
		// Invalide all matrices containing world
		for(int i : worldDependencies)
			m_Matrices[i]->SetDirty();
		break;
	case MAT_VIEW:
		// Invalide all matrices containing view
		for(int i : viewDependencies)
			m_Matrices[i]->SetDirty();
		break;
	case MAT_PROJ:
		// Invalide all matrices containing proj
		for(int i : projDependencies)
			m_Matrices[i]->SetDirty();
		break;
	default:
		return;
	}

	m_Matrices[type]->ClearDirty();
}

const math::Matrix4& MatrixTable::GetMatrix(EMatrixType type) const
{
	if(!IsUpToDate(type))
		UpdateMatrix(type);

	return m_Matrices[type]->value;
}

bool MatrixTable::IsUpToDate(EMatrixType type) const
{
	return !m_Matrices[type]->dirty;
}

void MatrixTable::UpdateMatrix(EMatrixType type) const
{
	switch(type) {
	case MAT_WORLD:
		lxAssert(IsUpToDate(MAT_WORLD_INV));
		m_Matrices[MAT_WORLD]->value = m_Matrices[MAT_WORLD_INV]->value.GetTransformInverted();
		break;
	case MAT_VIEW:
		lxAssert(IsUpToDate(MAT_VIEW_INV));
		m_Matrices[MAT_VIEW]->value = m_Matrices[MAT_VIEW_INV]->value.GetTransformInverted();
		break;
	case MAT_WORLD_INV:
		lxAssert(IsUpToDate(MAT_WORLD));
		m_Matrices[MAT_WORLD_INV]->value = m_Matrices[MAT_WORLD]->value.GetTransformInverted();
		break;
	case MAT_VIEW_INV:
		lxAssert(IsUpToDate(MAT_VIEW));
		m_Matrices[MAT_VIEW_INV]->value = m_Matrices[MAT_VIEW]->value.GetTransformInverted();
		break;
	case MAT_WORLD_VIEW:
		m_Matrices[MAT_WORLD_VIEW]->value.SetByProduct(GetMatrix(MAT_VIEW), GetMatrix(MAT_WORLD));
		break;
	case MAT_WORLD_PROJ:
		m_Matrices[MAT_WORLD_PROJ]->value.SetByProduct(GetMatrix(MAT_PROJ), GetMatrix(MAT_WORLD));
		break;
	case MAT_VIEW_PROJ:
		m_Matrices[MAT_VIEW_PROJ]->value.SetByProduct(GetMatrix(MAT_PROJ), GetMatrix(MAT_VIEW));
		break;
	case MAT_WORLD_VIEW_PROJ:
		if(IsUpToDate(MAT_VIEW_PROJ))
			m_Matrices[MAT_WORLD_VIEW_PROJ]->value.SetByProduct(GetMatrix(MAT_VIEW_PROJ), GetMatrix(MAT_WORLD));
		else if(IsUpToDate(MAT_WORLD_VIEW))
			m_Matrices[MAT_WORLD_VIEW_PROJ]->value.SetByProduct(GetMatrix(MAT_PROJ), GetMatrix(MAT_WORLD_VIEW));
		else {
			m_Matrices[MAT_WORLD_VIEW]->value.SetByProduct(GetMatrix(MAT_VIEW), GetMatrix(MAT_WORLD));
			m_Matrices[MAT_WORLD_VIEW]->ClearDirty();
			m_Matrices[MAT_WORLD_VIEW_PROJ]->value.SetByProduct(GetMatrix(MAT_PROJ), GetMatrix(MAT_WORLD_VIEW));
		}
		break;
	case MAT_WORLD_VIEW_INV:
		if(IsUpToDate(MAT_WORLD_VIEW)) {
			m_Matrices[MAT_WORLD_VIEW]->value.SetByProduct(GetMatrix(MAT_VIEW), GetMatrix(MAT_WORLD));
			m_Matrices[MAT_WORLD_VIEW]->ClearDirty();
		}
		m_Matrices[MAT_WORLD_VIEW_INV]->value = GetMatrix(MAT_WORLD_VIEW).GetTransformInverted();
		break;
	case MAT_WORLD_TRANS:
		m_Matrices[MAT_WORLD_TRANS]->value = GetMatrix(MAT_WORLD).GetTransposed();
		break;
	case MAT_VIEW_TRANS:
		m_Matrices[MAT_VIEW_TRANS]->value = GetMatrix(MAT_VIEW).GetTransposed();
		break;
	case MAT_WORLD_INV_TRANS:
		m_Matrices[MAT_WORLD_INV_TRANS]->value = GetMatrix(MAT_WORLD_INV).GetTransposed();
		break;
	case MAT_VIEW_INV_TRANS:
		m_Matrices[MAT_VIEW_INV_TRANS]->value = GetMatrix(MAT_VIEW_INV).GetTransposed();
		break;
	case MAT_WORLD_VIEW_TRANS:
		m_Matrices[MAT_WORLD_VIEW_TRANS]->value = GetMatrix(MAT_WORLD_VIEW).GetTransposed();
		break;
	case MAT_WORLD_VIEW_INV_TRANS:
		m_Matrices[MAT_WORLD_VIEW_INV_TRANS]->value = GetMatrix(MAT_WORLD_VIEW_INV).GetTransposed();
		break;
	default:
		lxAssertNeverReach("Can't generate matrix");
	}

	m_Matrices[type]->ClearDirty();
}

const core::String& MatrixTable::GetMatrixName(int id) const
{
	static const core::String MATRIX_NAMES[16] = {
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
