#include "ReferableFactoryImpl.h"
#include "core/Logger.h"
#include "core/ReferableRegister.h"
#include "core/lxException.h"

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

void RunAllRegisterReferableFunctions()
{
	for(auto block = g_FirstReferableBlock; block; block = block->nextBlock)
		ReferableFactory::Instance()->RegisterType(block->type, block->creator);
}
}

ReferableFactoryImpl::ReferableFactoryImpl()
{
}

void ReferableFactoryImpl::RegisterType(Name type, CreationFunc create)
{
	if(type == Name::INVALID)
		throw InvalidArgumentException("type", "An empty name is not allowed.");
	if(!create)
		throw InvalidArgumentException("create", "A creation function must be given.");

	if(m_Types.HasKey(type))
		throw core::InvalidArgumentException("type", "Type name is already used");

	m_Types.Set(type, ReferableType(create));

	log::Debug("Registered type ~s.", type);
}

void ReferableFactoryImpl::UnregisterType(Name type)
{
	if(m_Types.Erase(type))
		log::Debug("Unregistered type ~s.", type);
}

StrongRef<Referable> ReferableFactoryImpl::Create(Name type, const void* data)
{
	CreationFunc create = m_Types.At(type).create;
	Referable* r = create ? create(data) : nullptr;
	if(!r)
		throw Exception("Can't create new instance of given type.");

	return r;
}

size_t ReferableFactoryImpl::GetTypeCount() const
{
	return m_Types.Size();
}

}
}
