#ifndef INCLUDED_PARAMPACKAGE_H
#define INCLUDED_PARAMPACKAGE_H
#include "core/lxString.h"
#include "core/lxArray.h"
#include "math/vector2.h"
#include "math/vector3.h"

#include "video/TextureLayer.h"
#include "video/Texture.h"
#include "video/CubeTexture.h"

namespace lux
{
namespace core
{

//! The description of a single parameter
struct ParamDesc
{
	const char* name; //!< The name of the parameter
	u8 size; //!< The size of the parameter in bytes
	core::Type type; //!< The type of the parameter
	u32 id;

	const void* defaultValue;

	u32 reserved;
};

//! Represents a single parameter in a parameter package
class PackageParam
{
public:
	//! Construct a package param to some data
	/**
	\param size The size of parameter in byte
	\param type The type of the parameter
	\param data A pointer to the data of the param, must be valid for the lifetime of this object
	\param name The name of parameter, must be valid for the lifetime of this object(i.e. Is not copied)
	*/
	PackageParam(const ParamDesc& desc, u8* data) :
		m_Data(data),
		m_Desc(desc)
	{
	}

	//! Construct an empty invalid parameter package
	PackageParam() :
		m_Data(nullptr)
	{
	}

	//! Access as any type
	template <typename T>
	operator T()
	{
		if(IsValid()) {
			if(!IsConvertible(m_Desc.type, core::GetTypeInfo<T>()))
				throw TypeException("Incompatible types used", m_Desc.type, core::GetTypeInfo<T>());

			T out;
			ConvertBaseType(m_Desc.type, m_Data, core::GetTypeInfo<T>(), &out);
			return out;
		}

		return T();
	}

	template <typename T>
	T Default(const T& defaultValue)
	{
		if(!IsConvertible(m_Desc.type, core::GetTypeInfo<T>()) || !IsValid())
			return defaultValue;
		else
			return (T)(*this);
	}

	//! Access as TextureLayer
	operator video::TextureLayer()
	{
		if(m_Desc.type != core::Type::Texture)
			throw TypeException("Incompatible types used", m_Desc.type, core::Type::Texture);

		if(!IsValid())
			throw Exception("Accessed invalid package parameter");

		return *(video::TextureLayer*)m_Data;
	}

	//! Access as texture
	operator video::BaseTexture*()
	{
		return ((video::TextureLayer)*this).texture;
	}
	//! Access as texture
	operator video::Texture*()
	{
		auto base = (video::BaseTexture*)*this;
		auto out = dynamic_cast<video::Texture*>(base);
		if(!out)
			throw core::Exception("Invalid cast");

		return out;
	}

	//! Access as texture
	operator video::CubeTexture*()
	{
		auto base = (video::BaseTexture*)*this;
		auto out = dynamic_cast<video::CubeTexture*>(base);
		if(!out)
			throw core::Exception("Invalid cast");

		return out;
	}

	//! Access as any type
	template <typename T>
	PackageParam& operator=(const T& varVal)
	{
		if(!core::IsConvertible(core::GetTypeInfo<T>(), m_Desc.type))
			throw TypeException("Incompatible types used", core::GetTypeInfo<T>(), m_Desc.type);
		if(IsValid())
			core::ConvertBaseType(core::GetTypeInfo<T>(), &varVal, m_Desc.type, m_Data);

		return *this;
	}

	//! Access as Materiallayer
	PackageParam& operator=(video::TextureLayer& layer)
	{
		if(m_Desc.type != core::Type::Texture)
			throw TypeException("Incompatible types used", m_Desc.type, core::Type::Texture);
		if(!IsValid())
			throw Exception("Accessed invalid package parameter");

		*(video::TextureLayer*)m_Data = layer;

		return *this;
	}

	//! Access as texture
	PackageParam& operator=(video::BaseTexture* texture)
	{
		if(m_Desc.type != core::Type::Texture)
			throw TypeException("Incompatible types used", m_Desc.type, core::Type::Texture);

		if(!IsValid())
			throw Exception("Accessed invalid package parameter");

		((video::TextureLayer*)m_Data)->texture = texture;

		return *this;
	}

	//! Access as texture
	PackageParam& operator=(video::Texture* texture)
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Access as texture
	PackageParam& operator=(video::CubeTexture* texture)
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Access as texture
	PackageParam& operator=(StrongRef<video::BaseTexture> texture)
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Access as texture
	PackageParam& operator=(StrongRef<video::Texture> texture)
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Access as texture
	PackageParam& operator=(StrongRef<video::CubeTexture> texture)
	{
		return (*this = (video::BaseTexture*)texture);
	}

	//! Copy data from other packge param
	/**
	Deep Copy
	\param varVal The other packge param
	*/
	PackageParam& operator=(const PackageParam& varVal)
	{
		if(IsConvertible(m_Desc.type, varVal.m_Desc.type))
			throw TypeException("Incompatible types used", m_Desc.type, varVal.m_Desc.type);

		if(!IsValid())
			throw Exception("Accessed invalid package parameter");

		if(m_Desc.type == core::Type::Texture)
			*(video::TextureLayer*)m_Data = *(const video::TextureLayer*)varVal.m_Data;
		else
			ConvertBaseType(varVal.m_Desc.type, varVal.m_Data, m_Desc.type, m_Data);

		return *this;
	}

	//! Clone from other param package
	/**
	Swallow copy
	\param otherParam The other package param
	*/
	PackageParam& Set(const PackageParam& otherParam)
	{
		m_Data = otherParam.m_Data;
		m_Desc = otherParam.m_Desc;

		return *this;
	}

	//! Is it safe to access the param data
	bool IsValid() const
	{
		return (m_Data != nullptr);
	}

	//! The name of the param
	const char* GetName() const
	{
		if(IsValid())
			return m_Desc.name;
		else
			return "";
	}

	//! The size of the param in bytes
	u32 GetSize() const
	{
		return m_Desc.size;
	}

	const ParamDesc& GetDesc() const
	{
		return m_Desc;
	}

	//! The type of the param
	core::Type GetType() const
	{
		return m_Desc.type.GetBaseType();
	}

	//! A pointer to the raw param data
	void* Pointer()
	{
		return m_Data;
	}

private:
	void* m_Data;
	ParamDesc m_Desc;
};

//! A collection of named variable.
/**
Can be compared with a class, this class contains members and types, but no values.
*/
class LUX_API ParamPackage
{
private:
	struct Entry
	{
		string name;
		u8 size;
		u8 offset;
		core::Type type;
		u16 reserved;    // Reservierte Variable, z.B. Der zugehörige ShaderParam

		bool operator<(const Entry& other) const
		{
			return name < other.name;
		}
	};

public:
	ParamPackage();
	~ParamPackage();
	ParamPackage(const ParamPackage& other);
	ParamPackage& operator=(const ParamPackage& other);

	//! Clears the param package
	void Clear();

	//! Creates a new Param
	/**
	\param T: The type of the param
	\param name The name of the new Param(should be unique for package)
	\param defaultValue The default value for this Param, when a new material is created this is the used Value
	\param reserved Reserved for internal use, dont use this param
	*/
	template <typename T>
	void AddParam(const string_type& name, const T& defaultValue, u16 reserved = -1)
	{
		core::Type type = core::GetTypeInfo<T>();
		if(type == core::Type::Unknown)
			throw TypeException("Unsupported type used");

		AddParam(type, name, &defaultValue, reserved);
	}

	//! Creates a new Param
	/**
	\param type: The type of the param
	\param name The name of the new Param(should be unique for package)
	\param defaultValue The default value for this Param, when a new material is created this is the used Value
	\param reserved Reserved for internal use, dont use this param
	*/
	void AddParam(core::Type type, const string_type& name, const void* defaultValue, u16 reserved = -1);

	//! Merges two packages
	/**
	Objects with the same name and type will be combined into one parameters.
	Combination of objects with the same name and diffrent types will fail.
	*/
	void MergePackage(const ParamPackage& other);

	//! Creates a new ParamPackage
	/**
	\return A pointer to the newly created paramblock
	*/
	void* CreatePackage() const;

	//! Destroys a with CreatePackage created ParamBlock
	void DestroyPackage(void* p) const;

	//! Compare the data of two packages
	bool ComparePackage(const void* a, const void* b) const;

	//! Create a new package with the same content as an other one
	void* CopyPackage(const void* b) const;

	//! Retrieves Information about the Param from a index
	/**
	\param param The index of the param from which information is loaded
	\param [out] desc The description of the parameter
	\exception OutOfRange param is out of range
	*/
	ParamDesc GetParamDesc(u32 param) const;

	//! Retrieve the name of a param
	/**
	\param Param index of Param
	\return The name of the param
	\exception OutOfRange param is out of range
	*/
	const string& GetParamName(u32 param) const;

	//! Get a param from a id
	/**
	\param Param id of the param
	\param baseData The base pointer of the data block which belongs to the param
	\param isConst Should the package param be constant, i.e. can't be changed
	\return The found param
	\exception OutOfRange param is out of range
	*/
	PackageParam GetParam(u32 param, void* baseData, bool isConst) const;

	//! Get a param from a name
	/**
	\param name name of the param to found
	\param baseData The base pointer of the data block which belongs to the param
	\param isConst Should the package param be constant, i.e. can't be changed
	\return The found param
	\exception Exception name does not exist
	*/
	PackageParam GetParamFromName(const string_type& name, void* baseData, bool isConst) const;

	//! Get the n-th Param of a specific type
	/**
	The index is 0 for the first Layer, 1 for the second and so on
	\param type Which type to look for
	\param index The number of which layer should be searched
	\param baseData The base pointer of the data block which belongs to the param
	\param isConst Should the package param be constant, i.e. can't be changed
	\return The index of the found param, if no param could be found the invalid param is returned
	*/
	PackageParam GetParamFromType(core::Type type, u32 index, void* baseData, bool isConst) const;

	//! Set a new default value for a param
	/**
	\param param The id of the Param, which default value should be changed
	\param defaultValue A pointer to the new default value
	\exception OutOfRange param is out of range
	*/
	void SetDefaultValue(u32 param, const void* defaultValue, core::Type type = core::Type::Unknown);

	//! Set a new default value for a param
	/**
	\param param The name of the Param, which default value should be changed
	\param defaultValue A pointer to the new default value
	\exception Exception name does not exist
	*/
	void SetDefaultValue(const string_type& param, const void* defaultValue, core::Type type = core::Type::Unknown);

	//! Convienice function for SetDefaultValue.
	template <typename T>
	void SetDefaultValue(const char* name, const T& defaultValue)
	{
		SetDefaultValue(name, &defaultValue, core::GetTypeInfo<T>());
	}

	//! Convienice function for SetDefaultValue.
	template <typename T>
	void SetDefaultValue(u32 param, const T& defaultValue)
	{
		SetDefaultValue(param, &defaultValue, core::GetTypeInfo<T>());
	}

	//! Get the id of a parameter by it's name.
	/**
	\param name The name of the param.
	\param type The type of the parameter, core::Type::Unknown if any type is ok.
	\return The id of the parameter.
	\exception Exception name does not exist
	*/
	u32 GetParamId(const string_type& name, core::Type type = core::Type::Unknown) const;

	//! The number of existing params in this package
	u32 GetParamCount() const;

	//! The number of texturelayers in this package
	u32 GetTextureCount() const;

	//! The size of the allocated data, in bytes
	u32 GetTotalSize() const;

private:
	void AddEntry(Entry& entry, const void* defaultValue);
	bool GetId(string_type name, core::Type t, u32& outId) const;

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
		if(!pack) {
			m_Pack = pack;
			return;
		}

		if(m_Pack != pack) {
			// TÓDO: Move data from the old package to the new one
			if(m_Pack)
				m_Pack->DestroyPackage(m_Data);
			m_Data = pack->CreatePackage();
			m_Pack = pack;
		}
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
	PackageParam FromName(const string_type& name, bool isConst) const
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
	PackageParam FromType(core::Type type, u32 index, bool isConst) const
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
	PackageParam FromID(u32 id, bool isConst) const
	{
		if(m_Pack)
			return m_Pack->GetParam(id, m_Data, isConst);
		else
			return PackageParam();
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

	core::PackageParam Param(const string_type& name)
	{
		return FromName(name, false);
	}

	core::PackageParam Param(const string_type& name) const
	{
		return FromName(name, true);
	}

	core::PackageParam Param(u32 id)
	{
		return FromID(id, false);
	}

	core::PackageParam Param(u32 id) const
	{
		return FromID(id, true);
	}

private:
	const ParamPackage* m_Pack;
	void* m_Data;
};

class ParamList
{
public:
	struct Param
	{
		string name;
		u8* data;
		core::Type type;

		Param() :
			data(nullptr)
		{
		}

		Param(const Param& other) :
			name(other.name),
			type(other.type)
		{
			data = LUX_NEW_ARRAY(u8, type.GetSize());
			if(type == core::Type::Texture)
				new (data) video::TextureLayer(*(const video::TextureLayer*)other.data);
			else {
				lxAssert(type.IsTrivial());
				memcpy(data, other.data, type.GetSize());
			}
		}

		Param(Param&& old) :
			name(std::move(old.name)),
			type(std::move(old.type)),
			data(old.data)
		{
			old.data = nullptr;
		}

		Param& operator=(const Param& other)
		{
			this->~Param();
			data = LUX_NEW_ARRAY(u8, type.GetSize());
			if(type == core::Type::Texture)
				new (data) video::TextureLayer(*(const video::TextureLayer*)other.data);
			else {
				lxAssert(type.IsTrivial());
				memcpy(data, other.data, type.GetSize());
			}

			return *this;
		}

		Param& operator=(Param&& old)
		{
			this->~Param();
			name = std::move(old.name);
			type = std::move(old.type);
			data = old.data;

			old.data = nullptr;

			return *this;
		}

		Param(const string_type& n, core::Type t) :
			name(n.data),
			type(t)
		{
			data = LUX_NEW_ARRAY(u8, type.GetSize());
			if(type == core::Type::Texture)
				new (data) video::TextureLayer();
			else
				lxAssert(type.IsTrivial());
		}

		~Param()
		{
			if(type == core::Type::Texture)
				((video::TextureLayer*)data)->~TextureLayer();
			else
				lxAssert(type.IsTrivial());

			LUX_FREE_ARRAY(data);
		}
	};

public:
	u32 AddParam(const string_type& name, core::Type type)
	{
		u32 id;
		if(GetId(name, id))
			throw core::InvalidArgumentException("name", "Name already used");

		m_Params.PushBack(std::move(Param(name, type)));
		return (m_Params.Size() - 1);
	}

	u32 GetId(const string_type& name) const
	{
		u32 id;
		if(!GetId(name, id))
			throw core::ObjectNotFoundException(name.data);
		return id;
	}

	core::PackageParam operator[](u32 id) const
	{
		const auto& p = m_Params.At(id);

		core::ParamDesc desc;
		desc.id = id;
		desc.name = p.name.Data();
		desc.type = p.type;
		desc.size = (u8)p.type.GetSize();

		return core::PackageParam(desc, p.data);
	}

	u32 Size() const
	{
		return m_Params.Size();
	}

private:
	bool GetId(const string_type& name, u32& outId) const
	{
		for(size_t i = 0; i < m_Params.Size(); ++i) {
			if(m_Params[i].name == name) {
				outId = i;
				return true;
			}
		}
		return false;
	}

private:
	core::array<Param> m_Params;
};

} // namespace core
} // namespace video

#endif
