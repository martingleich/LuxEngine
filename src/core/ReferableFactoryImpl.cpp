#include "ReferableFactoryImpl.h"
#include "core/Logger.h"

namespace lux
{
namespace core
{

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

bool ReferableFactoryImpl::RegisterType(Referable* default)
{
	ReferableType entry;
	entry.type = default->GetReferableType();
	entry.subType = default->GetReferableSubType();
	entry.referable = default;

	if(entry.type == Name::INVALID || entry.subType == Name::INVALID) {
		log::Error("Invalid referable type or name.");
		return false;
	}

	core::array<ReferableType>::Iterator i, n;
	i = core::Binary_Search(entry, m_Referables.First(), m_Referables.End(), &n);

	if(i == m_Referables.End()) {
		m_Referables.Insert(entry, n);
		log::Debug("Registered new ~s: ~s.", entry.type, entry.subType);
		return true;
	}

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

bool ReferableFactoryImpl::SetDefault(Referable* default)
{
	auto it = FindEntry(default->GetReferableType(), default->GetReferableSubType());
	if(it != m_Referables.End()) {
		it->referable = default;
		return true;
	}

	return false;
}

StrongRef<Referable> ReferableFactoryImpl::GetDefault(Name type, Name name) const
{
	auto it = FindEntry(type, name);
	if(it != m_Referables.End())
		return it->referable;
	else
		return nullptr;
}

StrongRef<Referable> ReferableFactoryImpl::GetDefault(size_t id) const
{
	if(id < m_Referables.Size())
		return m_Referables[id].referable;
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
	StrongRef<Referable> default = GetDefault(type, subType);
	StrongRef<Referable> out = nullptr;
	if(default)
		out = default->Clone();

	if(!out)
		log::Warning("Not clonable default referable: ~s.~s.", type, subType);

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

} // namespace core
} // namespace lux