#include "core/ReferableFactory.h"
#include "core/Logger.h"
#include "core/lxException.h"

namespace lux
{
namespace impl_referableRegister
{
static ReferableRegisterBlock* g_FirstReferableBlock = nullptr;
void RegisterReferableBlock(ReferableRegisterBlock* block)
{
	block->nextBlock = g_FirstReferableBlock;
	g_FirstReferableBlock = block;
}

void RunAllRegisterReferableFunctions()
{
	for(auto block = g_FirstReferableBlock; block; block = block->nextBlock)
		core::ReferableFactory::Instance()->RegisterType(block->type, block->creator);
}
} // namespace impl_referableRegister

namespace core
{
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
		throw InvalidArgumentException("type", "An empty name is not allowed.");
	if(!create)
		throw InvalidArgumentException("create", "A creation function must be given.");

	bool set = m_Types.SetIfNotExist(type, ReferableType(create));
	if(!set)
		throw core::InvalidArgumentException("type", "Type name is already used");

	log::Debug("Registered type ~s.", type);
}

void ReferableFactory::UnregisterType(Name type)
{
	if(m_Types.Erase(type))
		log::Debug("Unregistered type ~s.", type);
}

StrongRef<Referable> ReferableFactory::Create(Name type, const void* data)
{
	auto& entry = m_Types.Get(type, ReferableType());
	CreationFunc create = entry.create;
	StrongRef<Referable> r = create ? create(data) : nullptr;
	if(!r)
		throw Exception("Can't create new instance of given type.");

	return r;
}

StrongRef<Referable> ReferableFactory::CreateShared(Name type, const void* data)
{
	auto& entry = m_Types.Get(type, ReferableType());
	if(entry.sharedInstance)
		return entry.sharedInstance;
	CreationFunc create = entry.create;
	StrongRef<Referable> r = create ? create(data) : nullptr;
	if(!r)
		throw Exception("Can't create new instance of given type.");

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
