#ifndef INCLUDED_MATRIX_TABLE_H
#define INCLUDED_MATRIX_TABLE_H
#include "math/Matrix4.h"
#include "core/ParamPackage.h"

namespace lux
{
namespace video
{

class MatrixTable
{
public:
	enum EMatrixType
	{
		MAT_WORLD = 0,
		MAT_VIEW,
		MAT_PROJ,

		MAT_WORLD_INV,
		MAT_VIEW_INV,

		MAT_WORLD_VIEW,
		MAT_WORLD_PROJ,
		MAT_VIEW_PROJ,
		MAT_WORLD_VIEW_PROJ,
		MAT_WORLD_VIEW_INV,

		MAT_WORLD_TRANS,
		MAT_VIEW_TRANS,

		MAT_WORLD_INV_TRANS,
		MAT_VIEW_INV_TRANS,

		MAT_WORLD_VIEW_TRANS,
		MAT_WORLD_VIEW_INV_TRANS,

		MAT_COUNT,
	};

public:
	MatrixTable();
	void SetMatrix(EMatrixType type, const math::Matrix4& matrix);
	core::PackageParam GetParamById(u32 id) const;
	bool GetParamIdByName(const char* name, u32& id);
	const math::Matrix4& GetMatrix(EMatrixType type) const;
	bool IsDirty(EMatrixType type) const;
	void ClearDirty(EMatrixType type) const;
	u32 GetCount() const;

private:
	bool IsUpToDate(EMatrixType type) const;
	void UpdateMatrix(EMatrixType type) const;
	const char* GetMatrixName(u32 id) const;

private:
	mutable math::Matrix4 m_Matrices[MAT_COUNT];
	mutable u32 m_UpToDate;
	mutable u32 m_Dirty;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_MATRIX_TABLE_H