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
			core::Attribute(_table->GetMatrixName((EMatrixType)_id), core::Types::Matrix()),
			table(_table),
			id((EMatrixType)_id)
		{
		}

		void SetValuePtr(const void* ptr) { table->SetMatrix(id, *(math::Matrix4*)ptr); }
		const void* GetValuePtr() { return &table->GetMatrix(id); }

		void SetDirty()
		{
			++dirty;
			++m_ChangeId;
		}
		void ClearDirty()
		{
			dirty = false;
		}
		void UpdateChangeId() { m_ChangeId++; }

		math::Matrix4 value;
		bool dirty=true;
	private:
		MatrixTable* table;
		EMatrixType id;
	};

public:
	MatrixTable();
	void SetMatrix(EMatrixType type, const math::Matrix4& matrix);
	const math::Matrix4& GetMatrix(EMatrixType type) const;

	core::Range<StrongRef<MatrixAttribute>*> Attributes()
	{
		return core::Range<StrongRef<MatrixAttribute>*>(m_Matrices, m_Matrices + MAT_COUNT);
	}

private:
	const core::String& GetMatrixName(int id) const;

private:
	bool IsUpToDate(EMatrixType type) const;
	void UpdateMatrix(EMatrixType type) const;

private:

	mutable StrongRef<MatrixAttribute> m_Matrices[MAT_COUNT];
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_MATRIX_TABLE_H