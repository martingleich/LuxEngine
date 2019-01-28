#ifndef INCLUDED_LUX_MATRIX_TABLE_H
#define INCLUDED_LUX_MATRIX_TABLE_H
#include "math/Matrix4.h"
#include "core/Attributes.h"

namespace lux
{
namespace video
{

class MatrixTable
{
	friend class MatrixAttribute;
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

	class MatrixAttribute : public core::Attribute
	{
	public:
		MatrixAttribute(MatrixTable* _table, int _id) :
			table(_table),
			id(_id)
		{
		}

		core::VariableAccess GetAccess(bool) override
		{
			return table->GetParamById(id);
		}

		const core::String& GetName() const override
		{
			return table->GetMatrixName(id);
		}

		core::Type GetType() const override
		{
			return core::Types::Matrix();
		}

		u32 GetChangeId() override
		{
			if(!table->IsUpToDate((EMatrixType)id)) {
				table->UpdateMatrix((EMatrixType)id);
				++m_ChangeId;
			}
			return m_ChangeId;
		}
	private:
		MatrixTable* table;
		int id;
		u32 m_ChangeId=0;
	};

public:
	MatrixTable();

	void SetMatrix(EMatrixType type, const math::Matrix4& matrix);
	core::VariableAccess GetParamById(int id) const;
	const math::Matrix4& GetMatrix(EMatrixType type) const;
	bool IsDirty(EMatrixType type) const;
	void ClearDirty(EMatrixType type) const;

	int GetCount() const;
	const core::String& GetMatrixName(int id) const;
	StrongRef<core::Attribute> CreateAttribute(int id);

private:
	bool IsUpToDate(EMatrixType type) const;
	void UpdateMatrix(EMatrixType type) const;

private:
	mutable math::Matrix4 m_Matrices[MAT_COUNT];
	mutable u32 m_UpToDate;
	mutable u32 m_Dirty;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_MATRIX_TABLE_H