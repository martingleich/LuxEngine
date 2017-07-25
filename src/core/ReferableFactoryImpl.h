#ifndef INCLUDED_REFERABLEFACTORY_IMPL_H
#define INCLUDED_REFERABLEFACTORY_IMPL_H
#include "core/ReferableFactory.h"
#include "core/lxArray.h"
#include "core/lxName.h"
#include "core/lxOrderedMap.h"

namespace lux
{
namespace core
{

class ReferableFactoryImpl : public ReferableFactory
{
public:
	ReferableFactoryImpl();

	void RegisterType(Name type, CreationFunc create);
	void UnregisterType(Name type);

	StrongRef<Referable> Create(Name type);

	size_t GetTypeCount() const;

	lxID MakeId(Referable* r);
	void FreeId(lxID id);

private:
	struct ReferableType
	{
		CreationFunc create;

		ReferableType() :
			create(nullptr)
		{}
		ReferableType(CreationFunc c) :
			create(c)
		{}
	};

private:
	core::OrderedMap<Name, ReferableType> m_Types;
	u32 m_NextID;
};

}
}

#endif // !INCLUDED_REFERABLEFACTORY_IMPL_H
