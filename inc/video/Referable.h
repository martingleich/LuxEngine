#ifndef INCLUDED_REFERABLE_H
#define INCLUDED_REFERABLE_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "core/lxName.h"

namespace lux
{
namespace ReferableResource
{
LUX_API extern const core::Name SceneNode;
LUX_API extern const core::Name SceneNodeComponent;
}

//! A referable object
/**
Referable objects can be cloned from older instances.
They also can be used with the \ref ReferableFactory and created there my name or id
*/
class Referable : public ReferenceCounted
{
public:
	//! Get the name of the referable type
	/**
	Must be unique over all types
	\return The name of the referable type
	*/
	virtual core::Name GetTypeName() const = 0;

	//! Returns the id the resource type
	/**
	\return The id of the resource type
	*/
	virtual core::Name GetResourceName() const = 0;

	//! Clones the referable object
	/**
	The newly created object is fully independent of the old one.
	\return The new object
	*/
	virtual StrongRef<Referable> Clone() const = 0;
};

}
#endif // INCLUDED_REFERABLE_H
