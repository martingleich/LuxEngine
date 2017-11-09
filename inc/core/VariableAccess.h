#ifndef INCLUDED_VARIABLE_ACCESS_H
#define INCLUDED_VARIABLE_ACCESS_H
#include "core/lxTypes.h"
#include "video/TextureLayer.h"
#include "video/Texture.h"
#include "video/CubeTexture.h"

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
	//! Construct a package param to some data
	/**
	\param type The type of the parameter
	\param data A pointer to the data of the param, must be valid for the lifetime of this object
	*/
	VariableAccess(core::Type type, void* data) :
		m_Data(data),
		m_Type(type)
	{
	}

	VariableAccess(const AnyObject& any, bool isConst = false) :
		m_Data(const_cast<void*>(any.Data())),
		m_Type(isConst ? any.GetType().GetConstantType() : any.GetType())
	{
	}

	//! Construct an empty invalid parameter package
	VariableAccess() :
		m_Data(nullptr),
		m_Type(core::Type::Unknown)
	{
	}

	//! Access as any type
	template <typename T>
	operator T() const
	{
		if(IsValid()) {
			auto type = core::GetTypeInfo<T>();
			if(!IsConvertible(m_Type, type))
				throw TypeException("Incompatible types used", m_Type, type);

			T out;
			ConvertBaseType(m_Type, m_Data, type, &out);
			return out;
		}

		return T();
	}

	template <typename T>
	T Default(const T& defaultValue) const
	{
		if(!IsConvertible(m_Type, core::GetTypeInfo<T>()) || !IsValid())
			return defaultValue;
		else
			return (T)(*this);
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
		return ((video::TextureLayer)*this).texture;
	}

	//! Access as texture
	operator video::Texture*() const
	{
		auto base = (video::BaseTexture*)*this;
		auto out = dynamic_cast<video::Texture*>(base);
		if(!out)
			throw core::Exception("Invalid cast");

		return out;
	}

	//! Access as texture
	operator video::CubeTexture*() const
	{
		auto base = (video::BaseTexture*)*this;
		auto out = dynamic_cast<video::CubeTexture*>(base);
		if(!out)
			throw core::Exception("Invalid cast");

		return out;
	}

	//! Access as any type
	template <typename T>
	const VariableAccess& operator=(const T& varVal) const
	{
		if(!core::IsConvertible(core::GetTypeInfo<T>(), m_Type))
			throw TypeException("Incompatible types used", core::GetTypeInfo<T>(), m_Type);
		if(IsValid())
			core::ConvertBaseType(core::GetTypeInfo<T>(), &varVal, m_Type, m_Data);

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
	Swallow copy
	\param otherParam The other package param
	*/
	VariableAccess& Set(const VariableAccess& otherParam)
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

	//! Access the raw data
	void* Data() const
	{
		return m_Data;
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
	if(m_Type != core::Types::Color() && m_Type != core::Types::Colorf())
		throw TypeException("Incompatible types used", m_Type, core::Types::Color());

	if(m_Type == core::Types::Color())
		*((video::Color*)m_Data) = color;

	if(m_Type == core::Types::Colorf())
		*((video::Colorf*)m_Data) = video::Colorf(color);

	return *this;
}

template <>
inline const VariableAccess& VariableAccess::operator=(const video::Color::EPredefinedColors& color) const
{
	if(m_Type != core::Types::Color() && m_Type != core::Types::Colorf())
		throw TypeException("Incompatible types used", m_Type, core::Types::Color());

	if(m_Type == core::Types::Color())
		*((video::Color*)m_Data) = color;

	if(m_Type == core::Types::Colorf())
		*((video::Colorf*)m_Data) = video::Colorf(color);

	return *this;
}

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_VARIABLE_ACCESS_H