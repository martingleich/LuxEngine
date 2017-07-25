#ifndef INCLUDED_REFERABLE_H
#define INCLUDED_REFERABLE_H
#include "core/ReferenceCounted.h"
#include "core/lxName.h"
#include "core/lxID.h"

namespace lux
{

//! A referable object
/**
Referable objects can be cloned from older instances.
They also can be used with the \ref ReferableFactory and created there my name or id
*/
class Referable : public ReferenceCounted
{
public:
	LUX_API Referable();
	LUX_API Referable(const Referable& other);
	LUX_API virtual ~Referable();

	//! Get the name of the referable type
	/**
	Must be unique over all types
	\return The name of the referable type
	*/
	virtual core::Name GetReferableType() const = 0;

	//! Returns the id of the object
	virtual core::lxID GetID() const
	{
		return m_ID;
	}

	//! Set's the id of the object
	/**
	This method is for internal use.
	The user should never call it.
	\param newID The new id of the object.
	*/
	virtual void SetID(core::lxID id)
	{
		m_ID = id;
	}

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

private:
	core::lxID m_ID;
};

}

#endif // INCLUDED_REFERABLE_H
