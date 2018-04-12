#ifndef INCLUDED_LUX_PARAMPACKAGE_H
#define INCLUDED_LUX_PARAMPACKAGE_H
#include "core/lxString.h"
#include "core/lxArray.h"

#include "core/VariableAccess.h"

namespace lux
{
namespace core
{

//! The description of a single parameter
struct ParamDesc
{
	const char* name; //!< The name of the parameter
	core::Type type; //!< The type of the parameter
	int id; //!< The id of the parameter
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
		core::String name;
		core::Type type;
		u8 size;
		u8 offset;

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
	*/
	template <typename T>
	int AddParam(const core::StringType& name, const T& defaultValue)
	{
		core::Type type = core::TemplType<T>::Get();
		if(type == core::Type::Unknown)
			throw TypeException("Unsupported type used");

		return AddParam(type, name, &defaultValue);
	}

	//! Creates a new Param
	/**
	\param type: The type of the param
	\param name The name of the new Param(should be unique for package)
	\param defaultValue The default value for this Param, when a new material is created this is the used Value
	*/
	LUX_API int AddParam(core::Type type, const core::StringType& name, const void* defaultValue = nullptr);

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
	LUX_API ParamDesc GetParamDesc(int param) const;

	//! Retrieve the name of a param
	/**
	\param Param index of Param
	\return The name of the param
	\exception OutOfRange param is out of range
	*/
	LUX_API const core::String& GetParamName(int param) const;

	//! Get a param from a id
	/**
	\param Param id of the param
	\param baseData The base pointer of the data block which belongs to the param
	\param isConst Should the package param be constant, i.e. can't be changed
	\return The found param
	\exception OutOfRange param is out of range
	*/
	LUX_API VariableAccess GetParam(int param, void* baseData, bool isConst) const;

	//! Get a param from a name
	/**
	\param name name of the param to found
	\param baseData The base pointer of the data block which belongs to the param
	\param isConst Should the package param be constant, i.e. can't be changed
	\return The found param
	\exception Exception name does not exist
	*/
	LUX_API VariableAccess GetParamFromName(const core::StringType& name, void* baseData, bool isConst) const;

	//! Get the n-th Param of a specific type
	/**
	The index is 0 for the first Layer, 1 for the second and so on
	\param type Which type to look for
	\param index The number of which layer should be searched
	\param baseData The base pointer of the data block which belongs to the param
	\param isConst Should the package param be constant, i.e. can't be changed
	\return The index of the found param, if no param could be found the invalid param is returned
	*/
	LUX_API VariableAccess GetParamFromType(core::Type type, int index, void* baseData, bool isConst) const;

	//! Access the default value of a param
	/**
	\param param The id of the Param, which default value should be changed
	*/
	LUX_API VariableAccess DefaultValue(int param);
	LUX_API VariableAccess DefaultValue(int param) const;

	//! Set a new default value for a param
	/**
	\param param The name of the Param, which default value should be changed
	\param defaultValue A pointer to the new default value
	\exception Exception name does not exist
	*/
	LUX_API VariableAccess DefaultValue(const core::StringType& param);

	//! Get the id of a parameter by it's name.
	/**
	\param name The name of the param.
	\param type The type of the parameter, core::Type::Unknown if any type is ok.
	\return The id of the parameter.
	\exception Exception name does not exist
	*/
	LUX_API int GetParamId(const core::StringType& name, core::Type type = core::Type::Unknown) const;

	//! The number of existing params in this package
	LUX_API int GetParamCount() const;

	//! The number of texturelayers in this package
	LUX_API int GetTextureCount() const;

	//! The size of the allocated data, in bytes
	LUX_API int GetTotalSize() const;

private:
	LUX_API int AddEntry(Entry& entry, const void* defaultValue);
	LUX_API bool GetId(core::StringType name, core::Type t, int& outId) const;

private:
	struct SelfData;
	SelfData* self; // PIMPL: To make the dll-interface independent
};

//! Contains the values for a parameter Package
class PackagePuffer
{
public:
	PackagePuffer() :
		m_Pack(nullptr)
	{
	}

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
	VariableAccess FromName(const core::StringType& name, bool isConst) const
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
	VariableAccess FromType(core::Type type, int index, bool isConst) const
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
	VariableAccess FromID(int id, bool isConst) const
	{
		if(m_Pack)
			return m_Pack->GetParam(id, m_Data, isConst);
		else
			throw core::Exception("Not param pack set");
	}

	//! The total number of parameters in the package
	int GetParamCount() const
	{
		if(m_Pack)
			return m_Pack->GetParamCount();
		else
			return 0;
	}

	//! The total number of textures in the package
	int GetTextureCount() const
	{
		if(m_Pack)
			return m_Pack->GetTextureCount();
		else
			return 0;
	}

	core::VariableAccess Param(const core::StringType& name)
	{
		return FromName(name, false);
	}

	core::VariableAccess Param(const core::StringType& name) const
	{
		return FromName(name, true);
	}

	core::VariableAccess Param(int id)
	{
		return FromID(id, false);
	}

	core::VariableAccess Param(int id) const
	{
		return FromID(id, true);
	}

private:
	const ParamPackage* m_Pack;
	void* m_Data;
};

} // namespace core
} // namespace lux

#endif
