#include "core/lxTypes.h"
#include "core/lxArray.h"

namespace lux
{
namespace core
{
namespace
{
struct Info
{
	u32 size;
	bool isTrivial;
};
core::array<Info> m_Table;
}

TypeTable& TypeTable::Instance()
{
	static TypeTable instance;
	return instance;
}

u32 TypeTable::GetSize(u32 id)
{
	return m_Table[id].size;
}

bool IsTrivial(u32 id)
{
	return m_Table[id].isTrivial;
}

void AddType(u32 id, u32 size, bool isTrivial)
{
	Info i;
	i.isTrivial = isTrivial;
	i.size = size;

	m_Table.Resize(math::Max((u32)id, m_Table.Size()));
	m_Table[id] = i;
}

}
}
