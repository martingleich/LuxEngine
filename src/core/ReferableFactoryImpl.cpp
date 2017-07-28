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
	if(g_FirstReferableBlock)
		block->nextBlock = g_FirstReferableBlock;

	g_FirstReferableBlock = block;
}

void RunAllRegisterReferableFunctions()
{
	for(auto block = g_FirstReferableBlock; block; block = block->nextBlock)
		ReferableFactory::Instance()->RegisterType(block->type, block->creator);
}
}

ReferableFactoryImpl::ReferableFactoryImpl() :
	m_NextID(1)
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

	log::Debug("Registerd type ~s.", type);
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

	r->SetID(MakeId(r));

	return r;
}

size_t ReferableFactoryImpl::GetTypeCount() const
{
	return m_Types.Size();
}

lxID ReferableFactoryImpl::MakeId(Referable* r)
{
	LUX_UNUSED(r);

	if(m_NextID == 0)
		throw RuntimeException("Out of unique referable id's");

	lxID id;
	id.value = m_NextID;
	++m_NextID;
	return id;
}

void ReferableFactoryImpl::FreeId(lxID id)
{
	LUX_UNUSED(id);
}

}
}
