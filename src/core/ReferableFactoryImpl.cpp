#include "ReferableFactoryImpl.h"
#include "core/Logger.h"
#include "core/ReferableRegister.h"

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

	return core::Binary_Search(dummy, m_Referables.First(), m_Referables.End());
}

ReferableFactoryImpl::ReferableFactoryImpl() :
	m_NextID(1)
{
}

bool ReferableFactoryImpl::RegisterType(Referable* prototype)
{
	ReferableType entry;
	entry.type = prototype->GetReferableType();
	entry.subType = prototype->GetReferableSubType();
	entry.prototype = prototype;

	if(entry.type == Name::INVALID || entry.subType == Name::INVALID) {
		log::Error("Invalid prototype type or name.");
		return false;
	}

	core::array<ReferableType>::Iterator i, n;
	i = core::Binary_Search(entry, m_Referables.First(), m_Referables.End(), &n);

	if(i == m_Referables.End()) {
		m_Referables.Insert(entry, n);
		log::Debug("Registered new ~s: ~s.", entry.type, entry.subType);
		return true;
	}

	log::Error("Multiple registered referable type: ~s: ~s.", entry.type, entry.subType);

	return false;
}

void ReferableFactoryImpl::UnregisterType(Name type, Name name)
{
	auto it = FindEntry(type, name);
	if(it != m_Referables.End()) {
		log::Debug("Unregistered type ~s: ~s.", type, name);
		m_Referables.Erase(it, true);
	}
}

bool ReferableFactoryImpl::SetPrototype(Referable* prototype)
{
	auto it = FindEntry(prototype->GetReferableType(), prototype->GetReferableSubType());
	if(it != m_Referables.End()) {
		it->prototype = prototype;
		return true;
	}

	return false;
}

StrongRef<Referable> ReferableFactoryImpl::GetPrototype(Name type, Name name) const
{
	auto it = FindEntry(type, name);
	if(it != m_Referables.End())
		return it->prototype;
	else
		return nullptr;
}

StrongRef<Referable> ReferableFactoryImpl::GetPrototype(size_t id) const
{
	if(id < m_Referables.Size())
		return m_Referables[id].prototype;
	else
		return nullptr;
}

StrongRef<Referable> ReferableFactoryImpl::Create(Name type, Name subType)
{
	if(m_NextID == 0) {
		log::Error("No more id's available.");
		return nullptr;
	}

	StrongRef<Referable> out = Create(type, subType, MakeId());

	return out;
}

StrongRef<Referable> ReferableFactoryImpl::Create(Name type, Name subType, lxID id)
{
	StrongRef<Referable> prototype = GetPrototype(type, subType);
	StrongRef<Referable> out = nullptr;
	if(prototype)
		out = prototype->Clone();

	if(!out)
		log::Warning("Not clonable prototype prototype: ~s.~s.", type, subType);

	if(out)
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

