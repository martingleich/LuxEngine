#include "core/Referable.h"
#include "core/ReferableFactory.h"

namespace lux
{
Referable::Referable()
{
	m_ID = core::ReferableFactory::Instance()->MakeId(this);
}

Referable::Referable(const Referable& other)
{
	LUX_UNUSED(other);
	m_ID = core::ReferableFactory::Instance()->MakeId(this);
}

Referable::~Referable()
{
	core::ReferableFactory::Instance()->FreeId(m_ID);
}

}

