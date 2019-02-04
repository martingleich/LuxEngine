#include "core/ReferableFactory.h"
#include "core/Logger.h"
#include "core/lxException.h"

#define LX_SORT_REFERABLE_BLOCKS

namespace lux
{
namespace core
{
namespace impl_referableRegister
{
static ReferableRegisterBlock* g_FirstReferableBlock = nullptr;
void RegisterReferableBlock(ReferableRegisterBlock* block)
{
	block->nextBlock = g_FirstReferableBlock;
	g_FirstReferableBlock = block;
}

#ifdef LX_SORT_REFERABLE_BLOCKS
static ReferableRegisterBlock* SortReferableBlocks(ReferableRegisterBlock* start, int count, ReferableRegisterBlock*& end)
{
	if(count == 1) {
		end = start->nextBlock;
		return start;
	}
	// Split in half.
	ReferableRegisterBlock* half;
	start = SortReferableBlocks(start, count/2, half);
	half = SortReferableBlocks(half, count-count/2, end);

	// Merge
	int cl = 0;
	int cr = 0;
	ReferableRegisterBlock* newStart;
	if((start->type.AsView()).Smaller(half->type.AsView(), core::EStringCompare::CaseInsensitive)) {
		++cl;
		newStart = start;
		start = start->nextBlock;
	} else {
		++cr;
		newStart = half;
		half = half->nextBlock;
	}
	ReferableRegisterBlock* cur = newStart;

	while(cl < count/2 && cr < count-count/2) {
		if(start->type.AsView().Smaller(half->type.AsView(), core::EStringCompare::CaseInsensitive)) {
			++cl;
			cur->nextBlock = start;
			start = start->nextBlock;
		} else {
			++cr;
			cur->nextBlock = half;
			half = half->nextBlock;
		}
		cur = cur->nextBlock;
	}

	while(cl < count/2) {
		++cl;
		cur->nextBlock = start;
		start = start->nextBlock;
		cur = cur->nextBlock;
	}
	while(cr < count - count/2) {
		++cr;
		cur->nextBlock = half;
		half = half->nextBlock;
		cur = cur->nextBlock;
	}

	cur->nextBlock = end;

	return newStart;
}
#endif

void RunAllRegisterReferableFunctions()
{
#ifdef LX_SORT_REFERABLE_BLOCKS
	// Sort blocks by name to make finding new referabled in the debug report easier.
	int count = 0;
	auto cur = g_FirstReferableBlock;
	while(cur) {
		++count;
		cur = cur->nextBlock;
	}
	g_FirstReferableBlock = SortReferableBlocks(g_FirstReferableBlock, count, cur);
#endif
	for(auto block = g_FirstReferableBlock; block; block = block->nextBlock)
		core::ReferableFactory::Instance()->RegisterType(block->type, block->creator);
}
} // namespace impl_referableRegister

static StrongRef<ReferableFactory> g_ReferableFactory;

void ReferableFactory::Initialize()
{
	g_ReferableFactory = LUX_NEW(ReferableFactory);
}

ReferableFactory* ReferableFactory::Instance()
{
	return g_ReferableFactory;
}

void ReferableFactory::Destroy()
{
	g_ReferableFactory.Reset();
}


ReferableFactory::ReferableFactory()
{
}

ReferableFactory::~ReferableFactory()
{
}

void ReferableFactory::RegisterType(Name type, CreationFunc create)
{
	if(type == Name::INVALID)
		throw GenericInvalidArgumentException("type", "An empty name is not allowed.");
	LX_CHECK_NULL_ARG(create);

	bool set = m_Types.SetIfNotExist(type, ReferableType(create)).addedNew;
	if(!set)
		throw core::GenericInvalidArgumentException("type", "Type name is already used");

	log::Debug("Registered type {0}.", type);
}

void ReferableFactory::UnregisterType(Name type)
{
	if(m_Types.Erase(type).removed)
		log::Debug("Unregistered type {0}.", type);
}

StrongRef<core::Referable> ReferableFactory::Create(Name type, const void* data)
{
	auto& entry = m_Types.Get(type, ReferableType());
	CreationFunc create = entry.create;
	StrongRef<core::Referable> r = create ? create(data) : nullptr;
	if(!r)
		throw FactoryCreateException(type.AsView(), "Can't create instance of referable.");

	return r;
}

int ReferableFactory::GetTypeCount() const
{
	return m_Types.Size();
}

core::Array<core::Name> ReferableFactory::GetTypeNames() const
{
	core::Array<core::Name> names;
	names.Reserve(m_Types.Size());
	for(auto& name : m_Types.Keys())
		names.PushBack(name);
	return names;
}

} // namespace core
} // namespace lux
