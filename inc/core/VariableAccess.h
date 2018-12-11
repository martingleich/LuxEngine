#ifndef INCLUDED_LUX_VARIABLE_ACCESS_H
#define INCLUDED_LUX_VARIABLE_ACCESS_H
#include "core/lxTypes.h"
#include "core/lxFormat.h"
#include "core/lxArray.h"
#include "video/Color.h"

namespace lux
{
namespace core
{

//! Represents access to some type of variable
/**
This could be an attribute, a normal c++ variable, or a value inside a param package.
*/
class VariableAccess
{
public:
	//! Construct an empty invalid parameter package
	VariableAccess() :
		m_Data(nullptr),
		m_Type(core::Type::Unknown)
	{
	}

	//! Construct a package param to some data
	/**
	\param type The type of the parameter
	\param data A pointer to the data of the param, must be valid for the lifetime of this object
	*/
	VariableAccess(core::Type type, const void* data) :
		m_Data(const_cast<void*>(data)),
		m_Type(type.GetConstantType())
	{
		if(m_Type.IsUnknown())
			throw core::GenericInvalidArgumentException("type", "Type can't be unknown.");
	}
	VariableAccess(core::Type type, void* data) :
		m_Data(data),
		m_Type(type)
	{
		if(m_Type.IsUnknown())
			throw core::GenericInvalidArgumentException("type", "Type can't be unknown.");
	}

	VariableAccess(VariableAccess&& old)
	{
		m_Data = old.m_Data;
		m_Type = old.m_Type;
	}

	VariableAccess(const VariableAccess&) = delete;

	template <typename T>
	static VariableAccess Constant(const T& value)
	{
		return VariableAccess(core::TemplType<T>::Get().GetConstantType(), const_cast<void*>((const void*)&value));
	}
	template <typename T>
	static VariableAccess Referable(T& value)
	{
		return VariableAccess(core::TemplType<T>::Get(), &value);
	}

	template <typename T>
	struct VariableGetter
	{
		static T Get(Type t, const void* data)
		{
			auto dest = core::TemplType<T>::Get();
			if(!IsConvertible(t, dest))
				throw TypeCastException(t, dest);

			T out;
			ConvertBaseType(t, data, dest, &out);
			return out;
		}
	};

	template <typename T>
	T Get() const {
		if(!IsValid())
			throw InvalidOperationException("Accessed invalid variable access.");
		return VariableGetter<T>::Get(m_Type, m_Data);
	}

	template <typename T>
	struct VariableSetter
	{
		static void Set(Type t, void* data, const T& value)
		{
			auto source = core::TemplType<T>::Get();
			if(!IsConvertible(source, t))
				throw TypeCastException(source, t);

			ConvertBaseType(t, &value, t, data);
		}
	};
	template <typename T>
	void Set(const T& value) const
	{
		if(!IsValid())
			throw InvalidOperationException("Accessed invalid variable access.");
		VariableSetter<T>::Set(m_Type, m_Data, value);
	}

	template <typename T>
	T GetDefault(const T& defaultValue) const
	{
		if(IsConvertible(m_Type, core::TemplType<T>::Get()) && IsValid())
			return Get<T>();
		else
			return defaultValue;
	}

	void AssignData(const void* data) const
	{
		m_Type.Assign(m_Data, data);
	}

	void CopyData(void* copyTo) const
	{
		m_Type.Assign(copyTo, m_Data);
	}

	const VariableAccess& operator=(const char* string) const
	{
		if(!IsValid())
			throw InvalidOperationException("Accessed invalid package parameter");

		if(m_Type != core::Types::String())
			throw TypeCastException(m_Type, core::Types::String());

		(*(core::String*)m_Data) = string;

		return *this;
	}

	//! Copy data from other packge param
	/**
	Deep Copy
	\param varVal The other packge param
	*/
	const VariableAccess& operator=(const VariableAccess& varVal) const
	{
		if(!IsValid())
			throw InvalidOperationException("Accessed invalid package parameter");

		if(IsConvertible(m_Type, varVal.m_Type))
			throw TypeCastException(m_Type, varVal.m_Type);

		ConvertBaseType(varVal.m_Type, varVal.m_Data, m_Type, m_Data);

		return *this;
	}

	//! Clone from other access
	/**
	Shallow copy
	\param otherParam The other access
	*/
	VariableAccess& CopyAccess(const VariableAccess& otherParam)
	{
		m_Data = otherParam.m_Data;
		m_Type = otherParam.m_Type;

		return *this;
	}

	//! Is it safe to access the param data
	bool IsValid() const
	{
		return (m_Data != nullptr);
	}

	//! The size of the param in bytes
	int GetSize() const
	{
		return m_Type.GetSize();
	}

	//! The type of the param
	core::Type GetType() const
	{
		return m_Type;
	}

	//! A pointer to the raw param data
	void* Pointer() const
	{
		return m_Data;
	}

private:
	void* m_Data;
	Type m_Type;
};

template <>
struct VariableAccess::VariableGetter<StringView>
{
	static core::StringView Get(Type t, const void* data)
	{
		if(t != core::Types::String())
			throw TypeCastException(t, core::Types::String());

		return ((const core::String*)data)->AsView();
	}
};

template <>
struct VariableAccess::VariableSetter<video::Color::EPredefinedColors>
{
	static void Get(Type t, void* data, video::Color::EPredefinedColors color)
	{
		if(t != core::Types::Color() && t != core::Types::ColorF())
			throw TypeCastException(t, core::Types::Color());

		if(t == core::Types::Color())
			*((video::Color*)data) = color;

		if(t == core::Types::ColorF())
			*((video::ColorF*)data) = video::ColorF(color);
	}
};

class VariableArrayAccess
{
public:
	VariableArrayAccess(const VariableAccess& access) :
		VariableArrayAccess(access.Pointer(), access.GetType())
	{
	}

	VariableArrayAccess(const void* ptr, Type type)
	{
		m_ArrayInfo = dynamic_cast<const AbstractArrayTypeInfo*>(type.GetInfo());
		if(!m_ArrayInfo)
			throw core::GenericInvalidArgumentException("type", "type is not an array");
		m_BaseType = m_ArrayInfo->GetBaseType();
#ifdef LUX_ENABLE_ASSERTS
		m_IsConst = type.IsConstant();
#endif
		// Can be read an written
		// But the const type will only be read, never written.
		m_ArrayPtr = const_cast<void*>(ptr);
	}
	int Size() const
	{
		return m_ArrayInfo->Size(m_ArrayPtr);
	}
	void Resize(int used)
	{
		if(m_IsConst)
			throw core::InvalidOperationException("Can't resize constant array.");
		m_ArrayInfo->Resize(m_ArrayPtr, used);
	}
	VariableAccess operator[](int i)
	{
		if(m_IsConst)
			throw core::InvalidOperationException("Can't change constant array.");
		return VariableAccess(m_BaseType, m_ArrayInfo->At(m_ArrayPtr, i));
	}

	VariableAccess operator[](int i) const
	{
		return VariableAccess(m_BaseType.GetConstantType(), m_ArrayInfo->At(m_ArrayPtr, i));
	}

private:
	const AbstractArrayTypeInfo* m_ArrayInfo;
	void* m_ArrayPtr;
	Type m_BaseType;
#ifdef LUX_ENABLE_ASSERTS
	bool m_IsConst;
#endif
};

template <typename T>
class ArrayAccess
{
public:
	ArrayAccess(const VariableAccess& access) :
		ArrayAccess(access.Pointer(), access.GetType())
	{
	}
	ArrayAccess(const void* ptr, Type type)
	{
		lxAssert(Types::IsArray(type));
		lxAssert(Types::GetArrayBase(type).GetBaseType() == core::TemplType<T>::Get());
#ifdef LUX_ENABLE_ASSERTS
		m_IsConst = type.IsConstant();
#endif
		// But the const type will only be read, never written.
		m_ArrayPtr = static_cast<core::Array<T>*>(const_cast<void*>(ptr));
	}

	int Size() const
	{
		return m_ArrayPtr->Size();
	}
	void Resize(int used)
	{
		lxAssert(m_IsConst == false);
		m_ArrayPtr->Resize(used);
	}
	T& operator[](int i)
	{
		lxAssert(m_IsConst == false);
		return (*m_ArrayPtr)[i];
	}
	const T& operator[](int i) const
	{
		return (*m_ArrayPtr)[i];
	}

private:
	core::Array<T>* m_ArrayPtr;
#ifdef LUX_ENABLE_ASSERTS
	bool m_IsConst;
#endif
};

inline void fmtPrint(format::Context& ctx, const VariableAccess& access, format::Placeholder& pl)
{
	access.GetType().GetInfo()->FmtPrint(ctx, access.Pointer(), pl);
}

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_VARIABLE_ACCESS_H