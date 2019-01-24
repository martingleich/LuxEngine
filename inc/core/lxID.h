#ifndef INCLUDED_LUX_ID_H
#define INCLUDED_LUX_ID_H
#include "core/LuxBase.h"
#include "core/lxFormat.h"
#include "core/lxTypes.h"
#include "core/ReferenceCounted.h"

namespace lux
{
namespace core
{

class Referable;
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

	LUX_API ID Create(core::Referable* obj);
	LUX_API void Release(ID id);
	LUX_API core::Referable* LookUp(ID id);

private:
	struct IDList;
	IDList* m_List;
};

namespace Types
{
LUX_API Type StrongID();
LUX_API Type WeakID();

LUX_API bool IsIDType(Type t);
}

template <> struct TemplType<core::ID> { static Type Get() { return Types::StrongID(); } };
}
}

#endif // #ifndef INCLUDED_LUX_ID_H
