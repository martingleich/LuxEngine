#include "ReferableFactoryImpl.h"
#include "core/Logger.h"
#include "core/ReferableRegister.h"
#include "core/lxException.h"

namespace lux
{
namespace core
{

namespace impl
{
static ReferableRegisterBlock* g_FirstReferableBlock = nullptr;
void RegisterReferable(ReferableRegisterBlock* block)
{
	if(g_FirstReferableBlock)
		block->nextBlock = g_FirstReferableBlock;

	g_FirstReferableBlock = block;
}

void RunAllRegisterReferableFunctions()
{
	for(auto block = g_FirstReferableBlock; block; block = block->nextBlock)
		ReferableFactoryImpl::Instance()->RegisterType(block->prototypeCreator());
}
}

ReferableFactoryImpl* ReferableFactoryImpl::Instance()
{
	static StrongRef<ReferableFactoryImpl> instance = nullptr;
	if(!instance)
		instance = LUX_NEW(ReferableFactoryImpl);

	return instance;
}

core::array<ReferableFactoryImpl::ReferableType>::Iterator ReferableFactoryImpl::FindEntry(Name type, Name subType) const
{
	ReferableType dummy;
	dummy.type = type;
	dummy.subType = subType;

	return core::BinarySearch(dummy, m_Referables.First(), m_Referables.End());
}

ReferableFactoryImpl::ReferableFactoryImpl() :
	m_NextID(1)
{
}

void ReferableFactoryImpl::RegisterType(Referable* prototype)
{
	ReferableType entry;
	entry.type = prototype->GetReferableType();
	entry.subType = prototype->GetReferableSubType();
	entry.prototype = prototype;

	if(entry.type == Name::INVALID || entry.subType == Name::INVALID)
		throw Exception("Invalid prototype type or name");

	core::array<ReferableType>::Iterator i, n;
	i = core::BinarySearch(entry, m_Referables.First(), m_Referables.End(), &n);
	if(i != m_Referables.End())
		throw Exception("Prototype already registered");

	if(i == m_Referables.End()) {
		m_Referables.Insert(entry, n);
		log::Debug("Registered new ~s: ~s.", entry.type, entry.subType);
	}
}

void ReferableFactoryImpl::UnregisterType(Name type, Name name)
{
	auto it = FindEntry(type, name);
	if(it != m_Referables.End()) {
		log::Debug("Unregistered type ~s: ~s.", type, name);
		m_Referables.Erase(it, true);
	}
}

void ReferableFactoryImpl::SetPrototype(Referable* prototype)
{
	auto it = FindEntry(prototype->GetReferableType(), prototype->GetReferableSubType());
	if(it != m_Referables.End())
		it->prototype = prototype;
}

StrongRef<Referable> ReferableFactoryImpl::GetPrototype(Name type, Name name) const
{
	auto it = FindEntry(type, name);
	if(it == m_Referables.End())
		throw ObjectNotFoundException((string(type.c_str()) + "." + string(name.c_str())).Data());

	return it->prototype;
}

StrongRef<Referable> ReferableFactoryImpl::GetPrototype(size_t id) const
{
	return m_Referables.At(id).prototype;
}

StrongRef<Referable> ReferableFactoryImpl::Create(Name type, Name subType)
{
	if(m_NextID == 0)
		throw Exception("Out of unique referable id's");

	return Create(type, subType, MakeId());
}

StrongRef<Referable> ReferableFactoryImpl::Create(Name type, Name subType, lxID id)
{
	StrongRef<Referable> prototype = GetPrototype(type, subType);
	StrongRef<Referable> out = prototype->Clone();

	if(!out)
		throw Exception("Cloning of prototype failed");

	out->SetID(id);

	return out;
}

size_t ReferableFactoryImpl::GetTypeCount() const
{
	return m_Referables.Size();
}

lxID ReferableFactoryImpl::MakeId()
{
	lxID id;
	id.value = m_NextID;
	++m_NextID;
	return id;
}

}

}

