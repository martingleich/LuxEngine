#ifndef INCLUDED_REFERABLE_H
#define INCLUDED_REFERABLE_H
#include "core/ReferenceCounted.h"
#include "core/lxName.h"
#include "core/lxTypes.h"
#include "lxID.h"

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
	Referable() :
		m_Id(core::IDManager::Instance()->Create(this))
	{
	}
	Referable(const Referable&) :
		m_Id(core::IDManager::Instance()->Create(this))
	{
	}
	virtual ~Referable()
	{
		core::IDManager::Instance()->Release(m_Id);
	}

	//! Get the name of the referable type
	/**
	Must be unique over all types
	\return The name of the referable type
	*/
	virtual core::Name GetReferableType() const = 0;

	core::ID GetUniqueId()
	{
		return m_Id;
	}

	StrongRef<Referable> Clone() const
	{
		return CloneImpl();
	}

protected:
	virtual StrongRef<Referable> CloneImpl() const
	{
		throw core::NotImplementedException();
	}

private:
	core::ID m_Id;
};

namespace core
{
namespace impl_referableRegister
{
struct ReferableRegisterBlock;

// Implemented in ReferableFactoryImpl.cpp
LUX_API void RegisterReferableBlock(ReferableRegisterBlock* block);
LUX_API void RunAllRegisterReferableFunctions();

struct ReferableRegisterBlock
{
	Name type;
	Referable* (*creator)(const void*);
	ReferableRegisterBlock* nextBlock;

	ReferableRegisterBlock(Name t, Referable* (*_creator)(const void*)) :
		type(t),
		creator(_creator),
		nextBlock(nullptr)
	{
		RegisterReferableBlock(this);
	}
};

}
namespace Types
{
LUX_API Type StrongID();
LUX_API Type WeakID();

LUX_API bool IsIDType(Type t);
}

template <> struct TemplType<core::ID> { static Type Get() { return Types::StrongID(); } };
}
}

//! Helper macro to declare all members for Referable classes
/**
Must be placed in the class definition in the header.
At best directly at the begin of the class.
*/
#define LX_REFERABLE_MEMBERS(class) \
public: \
::lux::core::Name GetReferableType() const; \
::lux::StrongRef<class> Clone() const; \
protected: \
::lux::StrongRef<::lux::Referable> CloneImpl() const; \
private:

//! Declare default referable members
/**
Use inside class inherited from Referable.
\param API API used to declare members.
*/
#define LX_REFERABLE_MEMBERS_API(class, API) \
public: \
API ::lux::core::Name GetReferableType() const; \
API ::lux::StrongRef<class> Clone() const; \
protected: \
API ::lux::StrongRef<::lux::Referable> CloneImpl() const; \
private:

//! Register referable class with ReferableFactory
/**
Use in global namespace in a source file.
\param ref_name C-String or core::Name containing the name of the class
\param class Fully classified name of the class
*/
#define LX_REGISTER_REFERABLE_CLASS(class, ref_name) \
static ::lux::Referable* LX_CONCAT(InternalCreatorFunc_, __LINE__)(const void*) { return LUX_NEW(class); } \
static ::lux::core::impl_referableRegister::ReferableRegisterBlock LX_CONCAT(InternalReferableRegisterStaticObject_, __LINE__)(::lux::core::Name(ref_name), &LX_CONCAT(InternalCreatorFunc_, __LINE__));

//! Helper macro to declare all members for Referable classes
/**
Must be placed in the global namespace in the source file.
\param class The fully qualified name of the class.
\param ref_name The refereable type name for this class.
*/
#define LX_REFERABLE_MEMBERS_SRC(class, ref_name) \
LX_REGISTER_REFERABLE_CLASS(class, ref_name) \
::lux::core::Name class::GetReferableType() const { static ::lux::core::Name n(ref_name); return n; } \
::lux::StrongRef<::lux::Referable> class::CloneImpl() const { return LUX_NEW(class)(*this); } \
::lux::StrongRef<class> class::Clone() const { return CloneImpl().StaticCastStrong<class>(); }

#endif // INCLUDED_REFERABLE_H
