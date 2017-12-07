#ifndef INCLUDED_REFERABLE_H
#define INCLUDED_REFERABLE_H
#include "core/ReferenceCounted.h"
#include "core/lxName.h"

namespace lux
{

//! A referable object
/**
Referable objects can be cloned from older instances.
They also can be used with the \ref ReferableFactory and created there my name or id
*/
class Referable : public virtual ReferenceCounted
{
public:
	virtual ~Referable() {}

	//! Get the name of the referable type
	/**
	Must be unique over all types
	\return The name of the referable type
	*/
	virtual core::Name GetReferableType() const = 0;

	//! Clones the referable object
	/**
	The newly created object is fully independent of the old one.
	Should copy data from the old object, but doesn't have too.
	\return The new object
	*/
	virtual StrongRef<Referable> Clone() const
	{
		throw core::NotImplementedException();
	}
};

}

#define LX_REFERABLE_MEMBERS(class, name) \
::lux::core::Name GetReferableType() const { static ::lux::core::Name n(name); return n; } \
::lux::StrongRef<::lux::Referable> Clone() const { return LUX_NEW(class)(*this); }

#endif // INCLUDED_REFERABLE_H
