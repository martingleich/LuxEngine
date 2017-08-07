#ifndef INCLUDED_PARAMPACKAGE_H
#define INCLUDED_PARAMPACKAGE_H
#include "core/lxString.h"
#include "core/lxArray.h"

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
	\param name The name of parameter, must be valid for the lifetime of this object(i.e. Is not copied)
	\param data A pointer to the data of the param, must be valid for the lifetime of this object
	*/
	VariableAccess(core::Type type, const char* name, void* data) :
		m_Data(data),
		m_Type(type),
		m_Name(name)
	{
	}

	VariableAccess(const char* name, const AnyObject& any, bool isConst = false) :
		m_Data(const_cast<void*>(any.Data())),
		m_Type(isConst ? any.GetType().GetConstantType() : any.GetType()),
		m_Name(name)
	{

	}

	//! Construct an empty invalid parameter package
	VariableAccess() :
		m_Data(nullptr),
		m_Type(core::Type::Unknown),
		m_Name(nullptr)
	{
	}

	//! Access as any type
	template <typename T>
	operator T() const
	{
		if(IsValid()) {
			if(!IsConvertible(m_Type, core::GetTypeInfo<T>()))
				throw TypeException("Incompatible types used", m_Type, core::GetTypeInfo<T>());

			T out;
			ConvertBaseType(m_Type, m_Data, core::GetTypeInfo<T>(), &out);
			return out;
		}

		return T();
	}

	template <typename T>
	T Default(const T& defaultValue)
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

		return ((String*)m_Data)->Data_c();
	}

	void AssignData(const void* data)
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
	VariableAccess& operator=(const T& varVal)
	{
		if(!core::IsConvertible(core::GetTypeInfo<T>(), m_Type))
			throw TypeException("Incompatible types used", core::GetTypeInfo<T>(), m_Type);
		if(IsValid())
			core::ConvertBaseType(core::GetTypeInfo<T>(), &varVal, m_Type, m_Data);

		return *this;
	}

	VariableAccess& operator=(const char* string)
	{
		if(m_Type != core::Types::String())
			throw TypeException("Incompatible types used", m_Type, core::Types::String());

		if(!IsValid())
			throw Exception("Accessed invalid package parameter");

		(*(String*)m_Data) = string;

		return *this;
	}

	//! Access as texture
	VariableAccess& operator=(video::BaseTexture* texture)
	{
		if(m_Type != core::Types::Texture())
			throw TypeException("Incompatible types used", m_Type, core::Types::Texture());

		if(!IsValid())
			throw Exception("Accessed invalid package parameter");

		((video::TextureLayer*)m_Data)->texture = texture;

		return *this;
	}

	//! Access as texture
	VariableAccess& operator=(video::Texture* texture)
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Access as texture
	VariableAccess& operator=(video::CubeTexture* texture)
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Access as texture
	VariableAccess& operator=(StrongRef<video::BaseTexture> texture)
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Access as texture
	VariableAccess& operator=(StrongRef<video::Texture> texture)
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Access as texture
	VariableAccess& operator=(StrongRef<video::CubeTexture> texture)
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Copy data from other packge param
	/**
	Deep Copy
	\param varVal The other packge param
	*/
	VariableAccess& operator=(const VariableAccess& varVal)
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
		m_Name = otherParam.m_Name;
		m_Type = otherParam.m_Type;

		return *this;
	}

	//! Is it safe to access the param data
	bool IsValid() const
	{
		return (m_Data != nullptr);
	}

	//! Access the raw data
	const void* Data() const
	{
		return m_Data;
	}

	//! Access the raw data
	void* Data()
	{
		return m_Data;
	}

	//! The name of the param
	const char* GetName() const
	{
		if(IsValid())
			return m_Name;
		else
			return "";
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
	void* Pointer()
	{
		return m_Data;
	}

private:
	void* m_Data;
	Type m_Type;
	const char* m_Name;
};

//! The description of a single parameter
struct ParamDesc
{
	const char* name; //!< The name of the parameter
	core::Type type; //!< The type of the parameter
	u32 id; //!< The id of the parameter
};

//! A collection of named variable.
/**
Can be compared with a class, this class contains members and types, but no values.
*/
class ParamPackage
{
private:
	struct Entry
	{
		String name;
		u8 size;
		u8 offset;
		core::Type type;

		bool operator<(const Entry& other) const
		{
			return name < other.name;
		}
	};

public:
	LUX_API ParamPackage();
	LUX_API ~ParamPackage();
	LUX_API ParamPackage(const ParamPackage& other);
	LUX_API ParamPackage& operator=(const ParamPackage& other);

	//! Clears the param package
	LUX_API void Clear();

	//! Creates a new Param
	/**
	\param T: The type of the param
	\param name The name of the new Param(should be unique for package)
	\param defaultValue The default value for this Param, when a new material is created this is the used Value, if null the param is default constructed.
	\param reserved Reserved for internal use, dont use this param
	*/
	template <typename T>
	u32 AddParam(const StringType& name, const T& defaultValue)
	{
		core::Type type = core::GetTypeInfo<T>();
		if(type == core::Type::Unknown)
			throw TypeException("Unsupported type used");

		return AddParam(type, name, &defaultValue);
	}

	//! Creates a new Param
	/**
	\param type: The type of the param
	\param name The name of the new Param(should be unique for package)
	\param defaultValue The default value for this Param, when a new material is created this is the used Value
	\param reserved Reserved for internal use, dont use this param
	*/
	LUX_API u32 AddParam(core::Type type, const StringType& name, const void* defaultValue = nullptr);

	//! Merges two packages
	/**
	Objects with the same name and type will be combined into one parameters.
	Combination of objects with the same name and diffrent types will fail.
	*/
	LUX_API void MergePackage(const ParamPackage& other);

	//! Creates a new ParamPackage
	/**
	\return A pointer to the newly created paramblock
	*/
	LUX_API void* CreatePackage() const;

	//! Destroys a with CreatePackage created ParamBlock
	LUX_API void DestroyPackage(void* p) const;

	//! Compare the data of two packages
	LUX_API bool ComparePackage(const void* a, const void* b) const;

	//! Create a new package with the same content as an other one
	LUX_API void* CopyPackage(const void* b) const;

	//! Retrieves Information about the Param from a index
	/**
	\param param The index of the param from which information is loaded
	\param [out] desc The description of the parameter
	\exception OutOfRange param is out of range
	*/
	LUX_API ParamDesc GetParamDesc(u32 param) const;

	//! Retrieve the name of a param
	/**
	\param Param index of Param
	\return The name of the param
	\exception OutOfRange param is out of range
	*/
	LUX_API const String& GetParamName(u32 param) const;

	//! Get a param from a id
	/**
	\param Param id of the param
	\param baseData The base pointer of the data block which belongs to the param
	\param isConst Should the package param be constant, i.e. can't be changed
	\return The found param
	\exception OutOfRange param is out of range
	*/
	LUX_API VariableAccess GetParam(u32 param, void* baseData, bool isConst) const;

	//! Get a param from a name
	/**
	\param name name of the param to found
	\param baseData The base pointer of the data block which belongs to the param
	\param isConst Should the package param be constant, i.e. can't be changed
	\return The found param
	\exception Exception name does not exist
	*/
	LUX_API VariableAccess GetParamFromName(const StringType& name, void* baseData, bool isConst) const;

	//! Get the n-th Param of a specific type
	/**
	The index is 0 for the first Layer, 1 for the second and so on
	\param type Which type to look for
	\param index The number of which layer should be searched
	\param baseData The base pointer of the data block which belongs to the param
	\param isConst Should the package param be constant, i.e. can't be changed
	\return The index of the found param, if no param could be found the invalid param is returned
	*/
	LUX_API VariableAccess GetParamFromType(core::Type type, u32 index, void* baseData, bool isConst) const;

	//! Access the default value of a param
	/**
	\param param The id of the Param, which default value should be changed
	*/
	LUX_API VariableAccess DefaultValue(u32 param);
	LUX_API VariableAccess DefaultValue(u32 param) const;

	//! Set a new default value for a param
	/**
	\param param The name of the Param, which default value should be changed
	\param defaultValue A pointer to the new default value
	\exception Exception name does not exist
	*/
	LUX_API VariableAccess DefaultValue(const StringType& param);

	//! Get the id of a parameter by it's name.
	/**
	\param name The name of the param.
	\param type The type of the parameter, core::Type::Unknown if any type is ok.
	\return The id of the parameter.
	\exception Exception name does not exist
	*/
	LUX_API u32 GetParamId(const StringType& name, core::Type type = core::Type::Unknown) const;

	//! The number of existing params in this package
	LUX_API u32 GetParamCount() const;

	//! The number of texturelayers in this package
	LUX_API u32 GetTextureCount() const;

	//! The size of the allocated data, in bytes
	LUX_API u32 GetTotalSize() const;

private:
	LUX_API u32 AddEntry(Entry& entry, const void* defaultValue);
	LUX_API bool GetId(StringType name, core::Type t, u32& outId) const;

private:
	struct SelfData;
	SelfData* self; // PIMPL: To make the dll-interface independent
};

//! Contains the values for a parameter Package
class PackagePuffer
{
public:
	//! Create data for given parameter packaged
	/**
	\param pack The parameter package on which the data builds up
	*/
	PackagePuffer(const ParamPackage* pack) :
		m_Pack(nullptr)
	{
		SetType(pack);
	}

	PackagePuffer(const PackagePuffer& other) :
		m_Pack(other.m_Pack)
	{
		if(m_Pack)
			m_Data = m_Pack->CopyPackage(other.m_Data);
	}

	~PackagePuffer()
	{
		if(m_Pack)
			m_Pack->DestroyPackage(m_Data);
	}

	PackagePuffer& operator=(const PackagePuffer& other)
	{
		if(m_Pack)
			m_Pack->DestroyPackage(m_Data);

		m_Pack = other.m_Pack;

		if(m_Pack)
			m_Data = m_Pack->CopyPackage(other.m_Data);

		return *this;
	}

	//! Equality
	bool operator==(const PackagePuffer& other) const
	{
		if(m_Pack != other.m_Pack)
			return false;
		if(m_Pack == nullptr)
			return true;

		return m_Pack->ComparePackage(m_Data, other.m_Data);
	}

	//! Unequality
	bool operator!=(const PackagePuffer& other) const
	{
		return !(*this == other);
	}

	//! Set the type of the package puffer
	/**
	\param pack The new type of the package
	*/
	void SetType(const ParamPackage* pack)
	{
		if(m_Pack == pack)
			return;

		if(m_Pack)
			m_Pack->DestroyPackage(m_Data);

		m_Pack = pack;

		if(m_Pack)
			m_Data = pack->CreatePackage();
	}

	//! Get the type of the puffer
	const ParamPackage* GetType() const
	{
		return m_Pack;
	}

	//! Reset the puffer to its default state
	void Reset()
	{
		if(m_Pack) {
			m_Pack->DestroyPackage(m_Data);
			m_Data = m_Pack->CreatePackage();
		}
	}

	//! Get a param from its name
	/**
	\param name The name of the param
	\param isConst Should the param be constant
	*/
	VariableAccess FromName(const StringType& name, bool isConst) const
	{
		if(!m_Pack)
			throw Exception("No param pack set");

		return m_Pack->GetParamFromName(name, m_Data, isConst);
	}

	//! Get a param from its type
	/**
	\param type The type of the param
	\param index Which param of this type
	\param isConst Should the param be constant
	*/
	VariableAccess FromType(core::Type type, u32 index, bool isConst) const
	{
		if(!m_Pack)
			throw Exception("No param pack set");

		return m_Pack->GetParamFromType(type, index, m_Data, isConst);
	}

	//! Get a param from its id
	/**
	\param id The index of the param
	\param isConst Should the param be constant
	*/
	VariableAccess FromID(u32 id, bool isConst) const
	{
		if(m_Pack)
			return m_Pack->GetParam(id, m_Data, isConst);
		else
			throw core::Exception("Not param pack set");
	}

	//! The total number of parameters in the package
	u32 GetParamCount() const
	{
		if(m_Pack)
			return m_Pack->GetParamCount();
		else
			return 0;
	}

	//! The total number of textures in the package
	u32 GetTextureCount() const
	{
		if(m_Pack)
			return m_Pack->GetTextureCount();
		else
			return 0;
	}

	core::VariableAccess Param(const StringType& name)
	{
		return FromName(name, false);
	}

	core::VariableAccess Param(const StringType& name) const
	{
		return FromName(name, true);
	}

	core::VariableAccess Param(u32 id)
	{
		return FromID(id, false);
	}

	core::VariableAccess Param(u32 id) const
	{
		return FromID(id, true);
	}

private:
	const ParamPackage* m_Pack;
	void* m_Data;
};

//! Casting from color to colorf
template <>
inline VariableAccess& VariableAccess::operator=(const video::Color& color)
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
inline VariableAccess& VariableAccess::operator=(const video::Color::EPredefinedColors& color)
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
} // namespace video

#endif
