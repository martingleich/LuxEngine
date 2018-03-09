#ifndef INCLUDED_VARIABLE_ACCESS_H
#define INCLUDED_VARIABLE_ACCESS_H
#include "core/lxTypes.h"
#include "core/lxFormat.h"
#include "video/TextureLayer.h"
#include "video/Texture.h"
#include "video/CubeTexture.h"
#include "core/lxArray.h"

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
		m_Type(type)
	{
	}

	//! Construct from any object
	explicit VariableAccess(const AnyObject& any, bool isConst = false) :
		m_Data(const_cast<void*>(any.Data())),
		m_Type(isConst ? any.GetType().GetConstantType() : any.GetType())
	{
		lxAssert(!m_Type.IsUnknown());
	}

	VariableAccess(VariableAccess&& old)
	{
		m_Data = old.m_Data;
		m_Type = old.m_Type;
	}

	VariableAccess(const VariableAccess&) = delete;

	template <typename T, bool U = !std::is_same<T, VariableAccess>::value, std::enable_if_t<U, int> = 0>
	constexpr VariableAccess(const T& value) :
		m_Data(const_cast<void*>((const void*)&value)),
		m_Type(core::TemplType<T>::Get().GetConstantType())
	{
		lxAssert(!m_Type.IsUnknown());
	}

	template <typename T>
	static VariableAccess Constant(const T& value)
	{
		return VariableAccess(value);
	}

	template <typename T, bool U = !std::is_same<T, VariableAccess>::value, std::enable_if_t<U, int> = 0>
	constexpr VariableAccess(T& value) :
		m_Data(&value),
		m_Type(core::TemplType<T>::Get())
	{
	}

	template <typename T>
	T As() const
	{
		if(IsValid()) {
			auto type = core::TemplType<T>::Get();
			if(!IsConvertible(m_Type, type))
				throw TypeException("Incompatible types used", m_Type, type);

			T out;
			ConvertBaseType(m_Type, m_Data, type, &out);
			return out;
		}

		return T();
	}

	//! Access as any type
	template <typename T>
	operator T() const
	{
		return As<T>();
	}

	template <typename T>
	T Default(const T& defaultValue) const
	{
		if(IsConvertible(m_Type, core::TemplType<T>::Get()) && IsValid())
			return As<T>();
		else
			return defaultValue;
	}

	operator const char*() const
	{
		if(m_Type != core::Types::String())
			throw TypeException("Incompatible types used", m_Type, core::Types::String());

		if(!IsValid())
			throw Exception("Accessed invalid package parameter");

		return ((core::String*)m_Data)->Data_c();
	}

	void AssignData(const void* data) const
	{
		m_Type.Assign(m_Data, data);
	}

	void CopyData(void* copyTo) const
	{
		m_Type.Assign(copyTo, m_Data);
	}

	//! Access as texture
	operator video::BaseTexture*() const
	{
		return As<video::TextureLayer>().texture;
	}

	//! Access as texture
	operator video::Texture*() const
	{
		auto base = As<video::BaseTexture*>();
		auto out = dynamic_cast<video::Texture*>(base);
		if(!out)
			throw core::Exception("Invalid cast");

		return out;
	}

	//! Access as texture
	operator video::CubeTexture*() const
	{
		auto base = As<video::BaseTexture*>();
		auto out = dynamic_cast<video::CubeTexture*>(base);
		if(!out)
			throw core::Exception("Invalid cast");

		return out;
	}

	//! Access as any type
	template <typename T>
	const VariableAccess& operator=(const T& varVal) const
	{
		if(!core::IsConvertible(core::TemplType<T>::Get(), m_Type))
			throw TypeException("Incompatible types used", core::TemplType<T>::Get(), m_Type);
		if(IsValid())
			core::ConvertBaseType(core::TemplType<T>::Get(), &varVal, m_Type, m_Data);

		return *this;
	}

	const VariableAccess& operator=(const char* string) const
	{
		if(m_Type != core::Types::String())
			throw TypeException("Incompatible types used", m_Type, core::Types::String());

		if(!IsValid())
			throw Exception("Accessed invalid package parameter");

		(*(core::String*)m_Data) = string;

		return *this;
	}

	//! Access as texture
	const VariableAccess& operator=(video::BaseTexture* texture) const
	{
		if(m_Type != core::Types::Texture())
			throw TypeException("Incompatible types used", m_Type, core::Types::Texture());

		if(!IsValid())
			throw Exception("Accessed invalid package parameter");

		((video::TextureLayer*)m_Data)->texture = texture;

		return *this;
	}

	//! Access as texture
	const VariableAccess& operator=(video::Texture* texture) const
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Access as texture
	const VariableAccess& operator=(video::CubeTexture* texture) const
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Access as texture
	const VariableAccess& operator=(StrongRef<video::BaseTexture> texture) const
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Access as texture
	const VariableAccess& operator=(StrongRef<video::Texture> texture) const
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Access as texture
	const VariableAccess& operator=(StrongRef<video::CubeTexture> texture) const
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Copy data from other packge param
	/**
	Deep Copy
	\param varVal The other packge param
	*/
	const VariableAccess& operator=(const VariableAccess& varVal) const
	{
		if(IsConvertible(m_Type, varVal.m_Type))
			throw TypeException("Incompatible types used", m_Type, varVal.m_Type);

		if(!IsValid())
			throw Exception("Accessed invalid package parameter");

		ConvertBaseType(varVal.m_Type, varVal.m_Data, m_Type, m_Data);

		return *this;
	}

	//! Clone from other param package
	/**
	Shallow copy
	\param otherParam The other package param
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
	u32 GetSize() const
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

//! Casting from color to colorf
template <>
inline const VariableAccess& VariableAccess::operator=(const video::Color& color) const
{
	if(m_Type != core::Types::Color() && m_Type != core::Types::ColorF())
		throw TypeException("Incompatible types used", m_Type, core::Types::Color());

	if(m_Type == core::Types::Color())
		*((video::Color*)m_Data) = color;

	if(m_Type == core::Types::ColorF())
		*((video::ColorF*)m_Data) = video::ColorF(color);

	return *this;
}

template <>
inline const VariableAccess& VariableAccess::operator=(const video::Color::EPredefinedColors& color) const
{
	if(m_Type != core::Types::Color() && m_Type != core::Types::ColorF())
		throw TypeException("Incompatible types used", m_Type, core::Types::Color());

	if(m_Type == core::Types::Color())
		*((video::Color*)m_Data) = color;

	if(m_Type == core::Types::ColorF())
		*((video::ColorF*)m_Data) = video::ColorF(color);

	return *this;
}

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
		lxAssert(m_ArrayInfo);
		m_BaseType = m_ArrayInfo->GetBaseType();
#ifdef LUX_ENABLE_ASSERTS
		m_IsConst = type.IsConstant();
#endif
		// Can be read an written
		// But the const type will only be read, never written.
		m_ArrayPtr = const_cast<void*>(ptr);
	}
	size_t Size() const
	{
		return m_ArrayInfo->Size(m_ArrayPtr);
	}
	void Resize(size_t used)
	{
		lxAssert(m_IsConst == false);
		m_ArrayInfo->Resize(m_ArrayPtr, used);
	}
	VariableAccess operator[](size_t i)
	{
		lxAssert(m_IsConst == false);
		return VariableAccess(m_BaseType, m_ArrayInfo->At(m_ArrayPtr, i));
	}

	VariableAccess operator[](size_t i) const
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

	size_t Size() const
	{
		return m_ArrayPtr->Size();
	}
	void Resize(size_t used)
	{
		lxAssert(m_IsConst == false);
		m_ArrayPtr->Resize(used);
	}
	T& operator[](size_t i)
	{
		lxAssert(m_IsConst == false);
		return (*m_ArrayPtr)[i];
	}
	const T& operator[](size_t i) const
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

#endif // #ifndef INCLUDED_VARIABLE_ACCESS_H