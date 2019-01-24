#ifndef INCLUDED_LUX_PARAMPACKAGE_H
#define INCLUDED_LUX_PARAMPACKAGE_H
#include "core/lxString.h"
#include "core/VariableAccess.h"

namespace lux
{
namespace core
{

class ParamPackage
{
	struct ParamEntry
	{
		int strEntry;
		int strLength;
		int valueOffset;
		Type type;
	};

public:
	struct CreateEntry : public core::Uncopyable
	{
		core::String name;
		core::Type type;
		core::RawMemory defaultValue;
		CreateEntry(core::StringView _name, core::Type _type, const void* _defaultValue)
		{
			name = _name;
			type = _type;
			defaultValue.SetMinSize(type.GetSize());
			type.CopyConstruct(defaultValue, _defaultValue);
		}
		template <typename T>
		CreateEntry(core::StringView _name, const T& value) :
			CreateEntry(_name, core::TemplType<T>::Get(), &value)
		{
		}
		CreateEntry(CreateEntry&& old) :
			name(std::move(old.name)),
			type(std::move(old.type)),
			defaultValue(std::move(old.defaultValue))
		{
		}
		~CreateEntry()
		{
			type.Destruct(defaultValue);
		}
		CreateEntry& operator=(CreateEntry&& old)
		{
			name = std::move(old.name);
			type = std::move(old.type);
			defaultValue = std::move(old.defaultValue);
			return *this;
		}
	};

	LUX_API static const ParamPackage EMPTY;

	LUX_API ParamPackage();
	LUX_API ParamPackage(const CreateEntry* createEntries, int paramCount);
	LUX_API ~ParamPackage();
	LUX_API ParamPackage(const ParamPackage& other);
	LUX_API ParamPackage(ParamPackage&& old);
	LUX_API ParamPackage& operator=(const ParamPackage& other);
	LUX_API ParamPackage& operator=(ParamPackage&& old);

	LUX_API void* CreatePackage() const;
	LUX_API void DestroyPackage(void* data) const;
	LUX_API bool ComparePackage(const void* a, const void* b) const;
	LUX_API void* CopyPackage(const void* b) const;
	LUX_API void AssignPackage(void* a, const void* b) const;

	LUX_API int GetParamIdByName(StringView name) const;
	
	core::StringView GetParamName(int id) const
	{
		auto& param = m_Params.At(id);
		return core::StringView(GetStr(param.strEntry), param.strLength);
	}
	core::Type GetParamType(int id) const
	{
		return m_Params.At(id).type;
	}
	core::VariableAccess GetParamDefaultValue(int id) const
	{
		auto& param = m_Params.At(id);
		return core::VariableAccess(param.type.GetConstantType(), (const u8*)m_DefaultData + param.valueOffset);
	}
	core::VariableAccess GetParam(int id, void* baseData, bool constAccess) const
	{
		auto& param = m_Params.At(id);
		return core::VariableAccess(
			constAccess ? param.type.GetConstantType() : param.type,
			(u8*)baseData + param.valueOffset);
	}
	int GetParamCount() const
	{
		return m_Params.Size();
	}

private:
	const char* GetStr(int i) const
	{
		lxAssert(i >= 0 && i <= (int)m_Strings.GetSize());
		return (const char*)m_Strings + i;
	}
	int GetPackSize() const
	{
		return m_DefaultData.GetSize();
	}

private:
	core::Array<ParamEntry> m_Params;
	core::RawMemory m_Strings;
	core::RawMemory m_DefaultData;
	bool m_IsTrivial;
};

class ParamPackageBuilder : core::Uncopyable
{
public:
	void AddParamSet(const ParamPackage& other)
	{
		for(int i = 0; i < other.GetParamCount(); ++i)
			AddParam(other.GetParamName(i), other.GetParamType(i), other.GetParamDefaultValue(i).Pointer());
	}
	void MergeParamSet(const ParamPackage& other)
	{
		for(int i = 0; i < other.GetParamCount(); ++i)
			MergeParam(other.GetParamName(i), other.GetParamType(i), other.GetParamDefaultValue(i).Pointer());
	}
	template <typename T>
	void AddParam(StringView name, const T& defaultValue)
	{
		AddParam(name, core::TemplType<T>::Get(), &defaultValue);
	}

	int GetParamCount()
	{
		return m_Params.Size();
	}
	//! Add a new parameter
	/*
	Adds a new parameter to the pack.
	The id of the parameter is continuos.
	If a parameter with the same name already exists, a exception is thrown.
	*/
	int AddParam(StringView name, core::Type type, const void* defaultValue)
	{
		LX_CHECK_NULL_ARG(defaultValue);
		if(type.IsUnknown())
			throw core::GenericInvalidArgumentException("type", "Invalid type passed");

		// Check if it already exists.
		for(auto& p : m_Params) {
			if(p.name == name)
				throw core::GenericInvalidArgumentException("name", "Parameter already exists.");
		}
		auto id = m_Params.Size();
		AddUnsafe(name, type, defaultValue);
		return id;
	}

	template <typename T>
	void MergeParam(StringView name, const T& defaultValue)
	{
		MergeParam(name, core::TemplType<T>::Get(), &defaultValue);
	}

	//! Merge a parameter.
	/**
	If a parameter with the same name and same type already exists, nothing happens.
	If a parameter with the same name and diffrent type already exists, a exception is thrown.
	Otherwise the parameter is added.
	\return true if a new parameter was added, false if a merge happend
	*/
	bool MergeParam(StringView name, core::Type type, const void* defaultValue)
	{
		LX_CHECK_NULL_ARG(defaultValue);
		if(type.IsUnknown())
			throw core::GenericInvalidArgumentException("type", "Invalid type passed");

		// Check if it already exists.
		for(auto& p : m_Params) {
			if(p.name == name) {
				if(p.type != type)
					throw core::GenericInvalidArgumentException("name", "Parameter already exists, with diffrent type.");
				return false;
			}
		}

		AddUnsafe(name, type, defaultValue);
		return true;
	}

	ParamPackage Build()
	{
		return ParamPackage(m_Params.Data(), m_Params.Size());
	}
	ParamPackage BuildAndReset()
	{
		ParamPackage out(m_Params.Data(), m_Params.Size());
		Reset();
		return out;
	}
	void Reset()
	{
		m_Params.Clear();
	}

private:
	void AddUnsafe(StringView name, core::Type type, const void* defaultValue)
	{
		m_Params.EmplaceBack(name, type, defaultValue);
	}
private:
	core::Array<ParamPackage::CreateEntry> m_Params;
};

//! Contains the values for a parameter Package
class PackagePuffer
{
public:
	PackagePuffer() :
		m_Data(nullptr),
		m_Pack(&ParamPackage::EMPTY)
	{
	}

	//! Create data for given parameter packaged
	/**
	\param pack The parameter package on which the data builds up
	*/
	explicit PackagePuffer(const ParamPackage* pack) :
		m_Data(nullptr),
		m_Pack(&ParamPackage::EMPTY)
	{
		LX_CHECK_NULL_ARG(pack);
		m_Data = pack->CreatePackage();
		m_Pack = pack;
	}

	PackagePuffer(const PackagePuffer& other) :
		m_Pack(other.m_Pack)
	{
		m_Data = m_Pack->CopyPackage(other.m_Data);
	}

	~PackagePuffer()
	{
		m_Pack->DestroyPackage(m_Data);
	}

	PackagePuffer& operator=(const PackagePuffer& other)
	{
		if(m_Pack == other.m_Pack) {
			m_Pack->AssignPackage(m_Data, other.m_Data);
		} else {
			m_Pack->DestroyPackage(m_Data);
			m_Pack = &ParamPackage::EMPTY;

			m_Data = other.m_Pack->CopyPackage(other.m_Data);
			m_Pack = other.m_Pack;
		}

		return *this;
	}

	//! Equality
	bool operator==(const PackagePuffer& other) const
	{
		if(m_Pack != other.m_Pack)
			return false;

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
		LX_CHECK_NULL_ARG(pack);
		if(m_Pack == pack)
			return;

		m_Pack->DestroyPackage(m_Data);
		m_Pack = &ParamPackage::EMPTY;

		m_Data = pack->CreatePackage();
		m_Pack = pack;
	}

	//! Reset the puffer to its default state
	void Reset()
	{
		auto pack = m_Pack;

		m_Pack->DestroyPackage(m_Data);
		m_Pack = &ParamPackage::EMPTY;

		m_Data = m_Pack->CreatePackage();
		m_Pack = pack;
	}

	//! Get a param from its name
	/**
	\param name The name of the param
	\param isConst Should the param be constant
	*/
	VariableAccess FromName(StringView name, bool isConst) const
	{
		int i = m_Pack->GetParamIdByName(name);
		if(i < 0)
			throw ObjectNotFoundException(name);
		return m_Pack->GetParam(i, m_Data, isConst);
	}

	//! Get a param from its id
	/**
	\param id The index of the param
	\param isConst Should the param be constant
	*/
	VariableAccess FromID(int id, bool isConst) const
	{
		return m_Pack->GetParam(id, m_Data, isConst);
	}

	//! The total number of parameters in the package
	int GetParamCount() const { return m_Pack->GetParamCount(); }
	//! Get the type of the puffer
	const ParamPackage* GetType() const { return m_Pack; }

	VariableAccess Param(StringView name) { return FromName(name, false); }
	VariableAccess Param(StringView name) const { return FromName(name, true); }
	VariableAccess Param(int id) { return FromID(id, false); } 
	VariableAccess Param(int id) const { return FromID(id, true); }

private:
	const ParamPackage* m_Pack;
	void* m_Data;
};

} // namespace core
} // namespace lux

#endif
