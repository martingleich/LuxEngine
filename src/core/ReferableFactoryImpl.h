#ifndef INCLUDED_REFERABLEFACTORY_IMPL_H
#define INCLUDED_REFERABLEFACTORY_IMPL_H
#include "core/ReferableFactory.h"
#include "core/lxArray.h"
#include "core/lxName.h"

namespace lux
{
namespace core
{

class ReferableFactoryImpl : public ReferableFactory
{
public:
	ReferableFactoryImpl();

	void RegisterType(Referable* prototype);
	void UnregisterType(Name type, Name subType);

	void SetPrototype(Referable* prototype);
	StrongRef<Referable> GetPrototype(Name type, Name subType) const;
	StrongRef<Referable> GetPrototype(size_t id) const;

	StrongRef<Referable> Create(Name type, Name subType);
	StrongRef<Referable> Create(Name type, Name subType, lxID id);

	size_t GetTypeCount() const;

	lxID MakeId();

private:
	struct ReferableType
	{
		Name type;
		Name subType;
		StrongRef<Referable> prototype;

		bool operator<(const ReferableType& other) const
		{
			if(type == other.type)
				return subType < other.subType;
			else
				return type < other.type;
		}

		bool operator==(const ReferableType& other) const
		{
			return type == other.type && subType == other.subType;
		}
	};

private:
	core::array<ReferableType>::Iterator FindEntry(Name type, Name subType) const;

private:
	mutable core::array<ReferableType> m_Referables;
	u32 m_NextID;
};

}
}

#endif // !INCLUDED_REFERABLEFACTORY_IMPL_H
