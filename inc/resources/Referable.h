#ifndef INCLUDED_REFERABLE_H
#define INCLUDED_REFERABLE_H
#include "core/ReferenceCounted.h"
#include "core/lxName.h"
#include "resources/lxID.h"

namespace lux
{
namespace ReferableType
{
LUX_API extern const core::Name Resource;
LUX_API extern const core::Name SceneNode;
LUX_API extern const core::Name SceneNodeComponent;
}

//! A referable object
/**
Referable objects can be cloned from older instances.
They also can be used with the \ref ReferableFactory and created there my name or id
*/
class Referable : public virtual ReferenceCounted
{
public:
	//! Get the name of the referable type
	/**
	Returns something like "scenenode" or "scenenodecomponent" or "resource"
	Must be unique over all types
	\return The name of the referable type
	*/
	virtual core::Name GetReferableType() const = 0;

	//! Returns the name of the exact type or the referable
	/**
	Returns something like "skybox", "cameracontrol", "texture" or "mesh".
	Must be unique over a group of main types.
	\return The name of the resource type
	*/
	virtual core::Name GetReferableSubType() const = 0;

	//! Returns the id of the object
	virtual core::lxID GetID() const
	{
		return m_ID;
	}

	//! Set's the id of the object
	/**
	This function should be use with extreme care.
	There should be no reference to the old id of the object.
	The new id shouldn't already be in use by another object.
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
	virtual StrongRef<Referable> Clone() const = 0;

private:
	core::lxID m_ID;
};

}

#endif // INCLUDED_REFERABLE_H
