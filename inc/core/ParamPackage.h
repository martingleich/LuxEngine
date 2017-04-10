#ifndef INCLUDED_PARAMPACKAGE_H
#define INCLUDED_PARAMPACKAGE_H
#include "core/lxString.h"
#include "core/lxArray.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "video/PipelineSettings.h"
#include "video/MaterialLayer.h"
#include "lxTypes.h"
#include "core/ReferenceCounted.h"

// TODO: Remove more headers

namespace lux
{
namespace video
{
class BaseTexture;
class Texture;
class CubeTexture;
}

namespace core
{

//! Represents a single parameter in a parameter package
class LUX_API PackageParam
{
	friend class ParamPackage;
	friend class PackagePuffer;
private:
	u8 m_Size;

	// The type core::Type is independet of the client or dll.
#pragma warning(suppress: 4251)
	core::Type m_Type;
	u8* m_Data;
	const char* m_Name;

public:
	static const PackageParam INVALID; //!< The invalid parameter package

private:
	PackageParam(u8 size, core::Type type, u8* data, const char* name);


public:
	//! Construct an empty invalid parameter package
	PackageParam();

	//! Make a swallow copy of another parameter package
	PackageParam(const PackageParam& other);

	//! Access as any type
	template <typename T>
	operator T()
	{
		assert(core::GetTypeInfo<T>() == m_Type);

		if(IsValid())
			return *((T*)m_Data);
		return T();
	}

	template <typename T>
	T Default(const T& default)
	{
		if(core::GetTypeInfo<T>() != m_Type || !IsValid())
			return default;
		else
			return (T)(*this);
	}

	//! Access as MaterialLayer
	operator video::MaterialLayer();
	//! Acess as texture
	operator video::BaseTexture*();
	//! Acess as texture
	operator video::Texture*();
	//! Acess as texture
	operator video::CubeTexture*();
	//! Acess as texture
	operator StrongRef<video::BaseTexture>();
	//! Acess as texture
	operator StrongRef<video::Texture>();
	//! Acess as texture
	operator StrongRef<video::CubeTexture>();

	//! Access as any type
	template <typename T>
	PackageParam& operator= (const T& varVal)
	{
		assert(core::GetTypeInfo<T>() == m_Type);
		if(IsValid())
			*((T*)m_Data) = varVal;

		return *this;
	}

	//! Acess as texture
	PackageParam& operator=(video::BaseTexture* texture);

	//! Acess as texture
	PackageParam& operator=(video::Texture* texture);

	//! Acess as texture
	PackageParam& operator=(video::CubeTexture* texture);

	//! Acess as texture
	PackageParam& operator=(StrongRef<video::BaseTexture> texture);

	//! Acess as texture
	PackageParam& operator=(StrongRef<video::Texture> texture);

	//! Acess as texture
	PackageParam& operator=(StrongRef<video::CubeTexture> texture);

	//! Acess as Materiallayer
	PackageParam& operator=(video::MaterialLayer& Layer);

	//! Copy data from other packge param
	/**
	Deep Copy
	\param varVal The other packge param
	*/
	PackageParam& operator=(const PackageParam& varVal);

	//! Clone from other param package
	/**
	Swallow copy
	\param otherParam The other package param
	*/
	PackageParam& Set(const PackageParam& otherParam);

	//! Is it safe to access the param data
	bool IsValid() const;

	//! The name of the param
	const char* GetName() const;

	//! The size of the param in bytes
	u32 GetSize() const;

	//! The type of the param
	core::Type GetType() const;

	//! A pointer to the raw param data
	void* Pointer();
};


//! Presents a description of the parameters of a MaterialType
class LUX_API ParamPackage
{
public:
	//! The description of a single parameter
	struct ParamDesc
	{
		const char* name; //!< The name of the parameter
		u8 size; //!< The size of the parameter in bytes
		core::Type type; //!< The type of the parameter
		u16 reserved; //!< System reserved value
		const void* default; //!< The default value of the parameter
	};

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

	//class "lux::core::array<lux::core::ParamPackage::Entry>" erfordert eine DLL-Schnittstelle, die von Clients von class "lux::core::ParamPackage" verwendet wird
	// BUT: The user can not access m_Params so its alright
#pragma warning(suppress: 4251)
	core::array<Entry> m_Params;

	u32 m_TotalSize;
	u32 m_TextureCount;

#pragma warning(suppress: 4251)
	core::mem::RawMemory m_DefaultPackage;

	void AddEntry(Entry& entry, const void* default);

public:

	//! Clears the param package
	void Clear();

	//! Default constuctor, creates a package without params
	ParamPackage();

	//! Destructor
	~ParamPackage();

	//! Copies another param package
	ParamPackage(const ParamPackage& other);

	//! Moves from another param package
	ParamPackage(ParamPackage&& old);

	//! Copies another param package
	ParamPackage& operator=(const ParamPackage& other);

	//! Moves from another param package
	ParamPackage& operator=(ParamPackage&& old);

	//! Creates a new Param
	/**
	\param T: The type of the param
	\param name The name of the new Param(should be unique for package)
	\param default The default value for this Param, when a new material is created this is the used Value
	\param reserved Reserved for internal use, dont use this param
	*/
	template <typename T>
	void AddParam(const char* name, const T& default, u16 reserved = -1)
	{
		core::Type type = core::GetTypeInfo<T>();
		if(type == core::Type::Unknown) {
			assertNeverReach("Unsupported type");
			return;
		}

		AddParam(type, name, &default, reserved);
	}

	//! Creates a new Param
	/**
	\param type: The type of the param
	\param name The name of the new Param(should be unique for package)
	\param default The default value for this Param, when a new material is created this is the used Value
	\param reserved Reserved for internal use, dont use this param
	*/
	void AddParam(core::Type type, const char* name, const void* default, u16 reserved = -1);

	//! Creates a new Param
	/**
	\param desc The description of the parameter
	*/
	void AddParam(const ParamDesc& desc);

	//! Creates a new ParamPackage
	/**
	\return A pointer to the newly created paramblock
	*/
	void* CreatePackage() const;

	//! A pointer to the default data of this package
	const void* GetDefault() const;

	//! Retrieves Information about the Param from a index
	/**
	\param param The index of the param from which information is loaded
	\param [out] desc The description of the parameter
	\return True if the param could be found, otherwise false
	*/
	bool GetParamDesc(u32 param, ParamDesc& desc) const;

	//! Retrieve the name of a param
	/**
	\param Param index of Param
	\return The name of the param
	*/
	const string& GetParamName(u32 param) const;

	//! Get a param from a id
	/**
	\param Param id of the param
	\param baseData The base pointer of the data block which belongs to the param
	\param isConst Should the package param be constant, i.e. can't be changed
	\return The found param
	*/
	PackageParam GetParam(u32 param, void* baseData, bool isConst) const;

	//! Get a param from a name
	/**
	\param name name of the param to found
	\param baseData The base pointer of the data block which belongs to the param
	\param isConst Should the package param be constant, i.e. can't be changed
	\return The found param, the invalid param if the name couldnt be found
	*/
	PackageParam GetParamFromName(const string& name, void* baseData, bool isConst) const;

	//! Get the n-th Param of a specific type
	/**
	The index is 0 for the first Layer, 1 for the second and so on
	\param type Which type to look for
	\param index The number of which layer should be searched
	\param baseData The base pointer of the data block which belongs to the param
	\param isConst Should the package param be constant, i.e. can't be changed
	\return The index of the found param, if no param could be found -1 is returned
	*/
	PackageParam GetParamFromType(core::Type type, int index, void* baseData, bool isConst) const;

	//! Set a new default value for a param
	/**
	\param param The id of the Param, which default value should be changed
	\param default A pointer to the new default value
	*/
	void SetDefaultValue(u32 param, const void* default);

	//! The number of existing params in this package
	u32 GetParamCount() const;

	//! The number of texturelayers in this package
	u32 GetTextureCount() const;

	//! The size of the allocated data, in bytes
	u32 GetTotalSize() const;
};

//! Contains the values for a parameter Package
class LUX_API PackagePuffer
{
private:
	const ParamPackage* m_Pack;
	void* m_Data;
	u32 m_MaxSize;

public:
	//! Create data for given parameter packaged
	/**
	\param pack The parameter package on which the data builds up
	*/
	PackagePuffer(const ParamPackage* pack);

	//! Copy package puffer
	PackagePuffer(const PackagePuffer& other);

	//! Move package puffer
	PackagePuffer(PackagePuffer&& old);

	//! Destructor
	~PackagePuffer();

	//! Equality
	bool operator==(const PackagePuffer& other) const;

	//! Unequality
	bool operator!=(const PackagePuffer& other) const;

	//! Copy from another package puffer
	PackagePuffer& operator=(const PackagePuffer& other);

	//! Move from another package puffer
	PackagePuffer& operator=(PackagePuffer&& old);

	//! Set the type of the package puffer
	/**
	\param pack The new type of the package
	*/
	void SetType(const ParamPackage* pack);

	//! Get the type of the puffer
	const ParamPackage* GetType() const;

	//! Reset the puffer to its default state
	void Reset();

	//! Get a param from its name
	/**
	\param name The name of the param
	\param isConst Should the param be constant
	*/
	PackageParam FromName(const string& name, bool isConst) const;

	//! Get a param from its type
	/**
	\param type The type of the param
	\param index Which param of this type
	\param isConst Should the param be constant
	*/
	PackageParam FromType(core::Type type, u32 index, bool isConst) const;

	//! Get a param from its id
	/**
	\param id The index of the param
	\param isConst Should the param be constant
	*/
	PackageParam FromID(u32 id, bool isConst) const;

	//! The total number of parameters in the package
	u32 GetParamCount() const;

	//! The total number of textures in the package
	u32 GetTextureCount() const;

	core::PackageParam Param(const string& name)
	{
		return FromName(name, false);
	}

	core::PackageParam Param(const string& name) const
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
};

}    

}    


#endif