#ifndef INCLUDED_LUX_REFERABLE_H
#define INCLUDED_LUX_REFERABLE_H
#include "core/ReferenceCounted.h"
#include "core/lxName.h"

namespace lux
{
namespace core
{
//! A referable object
/**
Referable objects can be cloned from older instances.
They also can be used with the \ref ReferableFactory and created there my name.
*/
class Referable : public ReferenceCounted
{
public:
	Referable() { }
	Referable(const Referable&) { }
	virtual ~Referable() { }
	Referable& operator=(const Referable& other) = delete;

	StrongRef<core::Referable> Clone() const
	{
		return CloneImpl();
	}

	//! Get the name of the referable type
	/**
	Must be unique over all types
	\return The name of the referable type
	*/
	virtual core::Name GetReferableType() const = 0;

protected:
	virtual StrongRef<core::Referable> CloneImpl() const
	{
		throw core::NotImplementedException("Referable::CloneImpl");
	}
};

namespace impl_referableRegister
{
class ReferableRegisterBlock;

// Implemented in ReferableFactoryImpl.cpp
LUX_API void RegisterReferableBlock(ReferableRegisterBlock* block);
LUX_API void RunAllRegisterReferableFunctions();

class ReferableRegisterBlock
{
public:
	typedef core::Referable* (*CreateFuncT)(const void*);
	core::Name type;
	CreateFuncT creator;
	ReferableRegisterBlock* nextBlock;

	ReferableRegisterBlock(core::Name t, CreateFuncT _creator) :
		type(t),
		creator(_creator),
		nextBlock(nullptr)
	{
		RegisterReferableBlock(this);
	}

	template <typename T>
	static CreateFuncT GetCreatorFunc()
	{
		return [](const void*) -> core::Referable* { return LUX_NEW(T); };
	}
};
} // namespace impl_ReferableRegister

//! Declare default referable members
/**
Use inside class inherited from Referable.
\param API API used to declare members.
*/
#define LX_REFERABLE_MEMBERS_API(class, API) \
friend ::lux::core::impl_referableRegister::ReferableRegisterBlock; \
public: \
API ::lux::core::Name GetReferableType() const; \
API ::lux::StrongRef<class> Clone() const; \
protected: \
API ::lux::StrongRef<::lux::core::Referable> CloneImpl() const; \
private: \
static ::lux::core::Name REFERABLE_TYPE_NAME;

//! Helper macro to declare all members for Referable classes
/**
Must be placed in the class definition in the header.
At best directly at the begin of the class.
*/
#define LX_REFERABLE_MEMBERS(class) LX_REFERABLE_MEMBERS_API(class,)

//! Register referable class with ReferableFactory
/**
Use in global namespace in a source file.
\param ref_name C-String or core::Name containing the name of the class
\param class Fully classified name of the class
*/
#define LX_REGISTER_REFERABLE_CLASS(class, ref_name) \
static ::lux::core::impl_referableRegister::ReferableRegisterBlock LX_CONCAT(InternalReferableRegisterStaticObject_, __LINE__)(::lux::core::Name(ref_name), ::lux::core::impl_referableRegister::ReferableRegisterBlock::GetCreatorFunc<class>());

//! Helper macro to declare all members for Referable classes
/**
Must be placed in the global namespace in the source file.
\param class The fully qualified name of the class.
\param ref_name The refereable type name for this class.
*/
#define LX_REFERABLE_MEMBERS_SRC(class, ref_name) \
LX_REGISTER_REFERABLE_CLASS(class, ref_name) \
::lux::core::Name class::REFERABLE_TYPE_NAME = ::lux::core::Name(ref_name); \
::lux::core::Name class::GetReferableType() const { return REFERABLE_TYPE_NAME;} \
::lux::StrongRef<::lux::core::Referable> class::CloneImpl() const { return LUX_NEW(class)(*this); } \
::lux::StrongRef<class> class::Clone() const { return CloneImpl().StaticCastStrong<class>(); } \

} // namespace core
} // namespace lux

#endif // INCLUDED_LUX_REFERABLE_H
