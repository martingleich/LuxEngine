#ifndef INCLUDED_LXTYPES_H
#define INCLUDED_LXTYPES_H
#include "LuxBase.h"
#include "lxException.h"
#include "lxUtil.h"
#include <string.h>
#include <new>

namespace lux
{
namespace core
{

class TypeInfo
{
public:
	TypeInfo(const char* name, size_t size, size_t align, bool isTrivial) :
		m_Name(name),
		m_Size(size),
		m_Align(align),
		m_IsTrivial(isTrivial)
	{
	}

	virtual void Construct(void* ptr) const = 0;
	virtual void CopyConstruct(void* ptr, const void* other) const = 0;
	virtual void Destruct(void* ptr) const = 0;
	virtual void Assign(void* ptr, const void* other) const = 0;
	virtual bool Compare(const void* a, const void* b) const = 0;

	//! Unique name of the type
	inline const char* GetName() const
	{
		return m_Name;
	}

	//! Size of the type in bytes.
	inline size_t GetSize() const
	{
		return m_Size;
	}

	//! Alignment of the type in bytes.
	inline size_t GetAlign() const
	{
		return m_Align;
	}

	//! Can the type be copied via memcpy
	inline bool IsTrivial() const
	{
		return m_IsTrivial;
	}

private:
	const char* const m_Name;
	const size_t m_Size;
	const size_t m_Align;
	const bool m_IsTrivial;
};

template <typename T>
class TypeInfoTemplate : public TypeInfo
{
public:
	TypeInfoTemplate(const char* name, bool trivial = std::is_trivial<T>::value) :
		TypeInfo(name, sizeof(T), alignof(T), trivial)
	{
	}

	void Construct(void* ptr) const
	{
		new (ptr) T();
	}

	void CopyConstruct(void* ptr, const void* other) const
	{
		new (ptr) T(*(reinterpret_cast<const T*>(other)));
	}

	void Destruct(void* ptr) const
	{
		if(ptr)
			reinterpret_cast<const T*>(ptr)->~T();
	}

	void Assign(void* ptr, const void* other) const
	{
		*(reinterpret_cast<T*>(ptr)) = *(reinterpret_cast<const T*>(other));
	}

	bool Compare(const void* a, const void* b) const
	{
		return *(reinterpret_cast<const T*>(a)) == *(reinterpret_cast<const T*>(b));
	}
};

class TypeInfoVirtual : public TypeInfo
{
public:
	TypeInfoVirtual(const char* name, size_t size, size_t align, bool isTrivial) :
		TypeInfo(name, size, align, isTrivial)
	{
	}

	void Construct(void*) const {}

	void CopyConstruct(void* ptr, const void* other) const
	{
		memcpy(ptr, other, GetSize());
	}

	void Destruct(void*) const {}

	void Assign(void* ptr, const void* other) const
	{
		memcpy(ptr, other, GetSize());
	}

	bool Compare(const void* a, const void* b) const
	{
		return (memcmp(a, b, GetSize()) == 0);
	}
};

//! Represent a type in the lux-engine
/**
This class is used to save type information.
*/
class Type
{
public:
	LUX_API static const Type Unknown;

public:
	Type() :
		m_Info(Type::Unknown.GetInfo()),
		m_IsConstant(false)
	{
	}

	//! Can be used to declare a new type.
	explicit Type(const TypeInfo* info) :
		m_Info(info),
		m_IsConstant(false)
	{
	}

	bool operator==(const Type& other) const
	{
		if(m_Info == other.m_Info)
			return true;
		return strcmp(m_Info->GetName(), other.m_Info->GetName()) == 0;
	}

	bool operator!=(const Type& other) const
	{
		return !(*this == other);
	}

	const TypeInfo* GetInfo() const
	{
		return m_Info;
	}

	const char* GetName() const
	{
		return m_Info->GetName();
	}

	void Construct(void* ptr) const
	{
		m_Info->Construct(ptr);
	}

	void CopyConstruct(void* ptr, const void* other) const
	{
		m_Info->CopyConstruct(ptr, other);
	}

	bool Compare(const void* a, const void* b) const
	{
		return m_Info->Compare(a, b);
	}

	void Destruct(void* ptr) const
	{
		m_Info->Destruct(ptr);
	}

	void Assign(void* ptr, const void* other) const
	{
		m_Info->Assign(ptr, other);
	}

	size_t GetSize() const
	{
		return m_Info->GetSize();
	}

	size_t GetAlign() const
	{
		return m_Info->GetAlign();
	}

	bool IsConstant() const
	{
		return m_IsConstant;
	}

	bool IsUnknown() const
	{
		return (*this == Unknown);
	}

	bool IsTrivial() const
	{
		return m_Info->IsTrivial();
	}

	Type GetConstantType() const
	{
		Type out = *this;
		out.m_IsConstant = true;

		return out;
	}

	Type GetBaseType() const
	{
		Type out = *this;
		out.m_IsConstant = true;

		return out;
	}

private:
	const TypeInfo* m_Info;
	bool m_IsConstant;
};

template <>
struct HashType<Type>
{
	size_t operator()(Type x)
	{
		HashType<const TypeInfo*> hasher;
		return hasher(x.GetInfo());
	}
};

namespace Types
{
inline Type Unknown()
{
	return Type::Unknown;
}
LUX_API Type Integer();
LUX_API Type U32();
LUX_API Type Byte();
LUX_API Type Float();
LUX_API Type Boolean();
}

//! Exception thrown when a type mismatch occures.
/**
Some type is not valid in this place, or two types are not compatible.
*/
struct TypeException : ErrorException
{
	explicit TypeException(const char* _msg, Type _typeA = Type::Unknown, Type _typeB = Type::Unknown) :
		ErrorException("type error"),
		msg(_msg),
		typeA(_typeA),
		typeB(_typeB)
	{
	}

	ExceptionSafeString msg;

	//! First type which was part of the problem
	Type typeA;

	//! Second type which was part of the porblem
	Type typeB;
};

//! Available Types for params
template <typename T>
struct TemplType { static Type Get() { return Types::Unknown(); } };

template <> struct TemplType<int> { static Type Get() { return Types::Integer(); } };
template <> struct TemplType<u32> { static Type Get() { return Types::U32(); } };
template <> struct TemplType<u8> { static Type Get() { return Types::Byte(); } };
template <> struct TemplType<float> { static Type Get() { return Types::Float(); } };
template <> struct TemplType<bool> { static Type Get() { return Types::Boolean(); } };

class AnyObject
{
public:
	AnyObject() :
		m_Type(Types::Unknown()),
		m_Data(nullptr)
	{
	}

	explicit AnyObject(const Type& type, const void* data = nullptr) :
		m_Type(type.GetBaseType())
	{
		m_Data = LUX_NEW_ARRAY(u8, m_Type.GetSize());
		if(data)
			m_Type.CopyConstruct(m_Data, data);
		else
			m_Type.Construct(m_Data);
	}

	AnyObject(const AnyObject& other) :
		m_Type(other.m_Type)
	{
		m_Data = LUX_NEW_ARRAY(u8, m_Type.GetSize());
		m_Type.CopyConstruct(m_Data, other.m_Data);
	}

	~AnyObject()
	{
		m_Type.Destruct(m_Data);
		LUX_FREE_ARRAY((u8*)m_Data);
	}

	AnyObject& operator=(const AnyObject& other)
	{
		if(m_Type == other.m_Type) {
			m_Type.Assign(m_Data, other.m_Data);
		} else {
			m_Type.Destruct(m_Data);
			m_Type = other.m_Type;
			LUX_FREE_ARRAY((u8*)m_Data);
			m_Data = LUX_NEW_ARRAY(u8, m_Type.GetSize());
			m_Type.CopyConstruct(m_Data, other.m_Data);
		}
		return *this;
	}

	void* Data()
	{
		return m_Data;
	}

	const void* Data() const
	{
		return m_Data;
	}

	void Assign(const void* data)
	{
		m_Type.Assign(m_Data, data);
	}

	template <typename T>
	T& Get()
	{
		if(TemplType<T>::Get() != m_Type)
			throw core::TypeException("Invalid type cast");
		return *reinterpret_cast<T*>(m_Data);
	}

	template <typename T>
	const T& Get() const
	{
		if(TemplType<T>::Get() != m_Type)
			throw core::TypeException("Invalid type cast");
		return *reinterpret_cast<const T*>(m_Data);
	}

	const Type& GetType() const
	{
		return m_Type;
	}

private:
	Type m_Type;
	void* m_Data;
};

//! Converts between base types.
/**
The base types are int, u32, float and bool.
*/
inline bool ConvertBaseType(Type fromType, const void* fromData, Type toType, void* toData)
{
	if(fromType == toType) {
		fromType.Assign(toData, fromData);
		return true;
	}

	if(!fromType.IsTrivial() || !toType.IsTrivial())
		return false;

	if(fromType == Types::Integer()) {
		if(toType == Types::U32()) {
			*((u32*)toData) = *((int*)fromData);
			return true;
		}
		if(toType == Types::Float()) {
			*((float*)toData) = (float)*((int*)fromData);
			return true;
		}
		if(toType == Types::Boolean()) {
			*((bool*)toData) = *((int*)fromData) ? true : false;
			return true;
		}
		return false;
	}

	if(fromType == Types::U32()) {
		if(toType == Types::Integer()) {
			*((int*)toData) = *((u32*)fromData);
			return true;
		}
		if(toType == Types::Float()) {
			*((float*)toData) = (float)*((u32*)fromData);
			return true;
		}

		if(toType == Types::Boolean()) {
			*((bool*)toData) = *((u32*)fromData) ? true : false;
			return true;
		}
		return false;
	}

	if(fromType == Types::Float()) {
		if(toType == Types::Integer()) {
			*((int*)toData) = (int)*((float*)fromData);
			return true;
		}
		if(toType == Types::U32()) {
			*((u32*)toData) = (size_t)*((float*)fromData);
			return true;
		}
		return false;
	}

	if(fromType == Types::Boolean()) {
		if(toType == Types::Integer()) {
			*((int*)toData) = *((bool*)fromData) ? 1 : 0;
			return true;
		}
		if(toType == Types::U32()) {
			*((u32*)toData) = *((bool*)fromData) ? 1 : 0;
			return true;
		}
		if(toType == Types::Float()) {
			*((float*)toData) = *((bool*)fromData) ? 1.0f : 0.0f;
			return true;
		}
		return false;
	}

	return false;
}

inline bool IsConvertible(Type fromType, Type toType)
{
	if(fromType == toType)
		return true;

	if(!fromType.IsTrivial() || !toType.IsTrivial())
		return false;

	if(fromType == Types::Integer()) {
		if(toType == Types::U32()) {
			return true;
		}
		if(toType == Types::Float()) {
			return true;
		}
		if(toType == Types::Boolean()) {
			return true;
		}
		return false;
	}

	if(fromType == Types::U32()) {
		if(toType == Types::Integer()) {
			return true;
		}
		if(toType == Types::Float()) {
			return true;
		}

		if(toType == Types::Boolean()) {
			return true;
		}
		return false;
	}

	if(fromType == Types::Float()) {
		if(toType == Types::Integer()) {
			return true;
		}
		if(toType == Types::U32()) {
			return true;
		}
		return false;
	}

	if(fromType == Types::Boolean()) {
		if(toType == Types::Integer()) {
			return true;
		}
		if(toType == Types::U32()) {
			return true;
		}
		if(toType == Types::Float()) {
			return true;
		}
		return false;
	}

	return false;
}

} // !namespace core
} // !namespace lux

#endif // !INCLUDED_LXTYPES_H
