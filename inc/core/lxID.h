#ifndef INCLUDED_LX_ID_H
#define INCLUDED_LX_ID_H
#include "core/LuxBase.h"
#include "core/lxFormat.h"
#include "core/ReferenceCounted.h"

namespace lux
{
class Referable;
namespace core
{

class ID
{
public:
	ID() :
		m_Value(0)
	{
	}
	explicit ID(u32 value) :
		m_Value(value)
	{
	}

	u32 GetValue() const
	{
		return m_Value;
	}

	bool operator==(const ID& other) const
	{
		return m_Value == other.m_Value;
	}

	bool operator!=(const ID& other) const
	{
		return m_Value != other.m_Value;
	}

private:
	u32 m_Value;
};

inline void fmtPrint(format::Context& ctx, ID id, format::Placeholder& placeholder)
{
	format::fmtPrint(ctx, id.GetValue(), placeholder);
}

class IDManager : public ReferenceCounted
{
public:
	LUX_API static IDManager* Instance();

	LUX_API IDManager();
	IDManager(IDManager&) = delete;
	LUX_API ~IDManager();

	LUX_API ID Create(Referable* obj);
	LUX_API void Release(ID id);
	LUX_API Referable* LookUp(ID id);

private:
	struct IDList;
	IDList* m_List;
};

}
}

#endif // #ifndef INCLUDED_LX_ID_H