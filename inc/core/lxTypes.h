#ifndef INCLUDED_LXTYPES_H
#define INCLUDED_LXTYPES_H
#include "LuxBase.h"
#include "lxException.h"
#include <string.h>
#include <new>

namespace lux
{
namespace core
{

class TypeInfo
{
public:
	TypeInfo(const char* name, size_t size, bool isTrivial) :
		m_Name(name),
		m_Size(size),
		m_IsTrivial(isTrivial)
	{
	}

	virtual void Construct(void* ptr) const = 0;
	virtual void CopyConstruct(void* ptr, const void* other) const = 0;
	virtual void Destruct(void* ptr) const = 0;
	virtual void Assign(void* ptr, const void* other) const = 0;
	virtual bool Compare(const void* a, const void* b) const = 0;

	inline const char* GetName() const
	{
		return m_Name;
	}

	inline size_t GetSize() const
	{
		return m_Size;
	}

	inline bool IsTrivial() const
	{
		return m_IsTrivial;
	}

private:
	const char* const m_Name;
	const size_t m_Size;
	const bool m_IsTrivial;
};

template <typename T>
class TypeInfoTemplate : public TypeInfo
{
public:
	TypeInfoTemplate(const char* name, bool isTrivial) :
		TypeInfo(name, sizeof(T), isTrivial)
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
	TypeInfoVirtual(const char* name, size_t size, bool isTrivial) :
		TypeInfo(name, size, isTrivial)
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

	LUX_API static const Type Integer;
	LUX_API static const Type Float;
	LUX_API static const Type Boolean;
	LUX_API static const Type U32;

	LUX_API static const Type Texture;
	LUX_API static const Type Color;
	LUX_API static const Type Colorf;
	LUX_API static const Type Vector2;
	LUX_API static const Type Vector3;
	LUX_API static const Type Vector2Int;
	LUX_API static const Type Vector3Int;
	LUX_API static const Type Matrix;

public:
	Type() :
		m_Info(Type::Unknown.GetInfo()),
		m_IsConstant(false)
	{
	}

	//! Do _not_ call
	Type(const TypeInfo* info) :
		m_Info(info),
		m_IsConstant(false)
	{
	}

	bool operator==(const Type& other) const
	{
		return m_Info == other.m_Info;
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

	void Destruct(void* ptr)
	{
		m_Info->Destruct(ptr);
	}

	void Assign(void* ptr, const void* other)
	{
		m_Info->Assign(ptr, other);
	}

	size_t GetSize() const
	{
		return m_Info->GetSize();
	}

	bool IsConstant() const
	{
		return m_IsConstant;
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
Type GetTypeInfo() { return Type::Unknown; }

template <> inline Type GetTypeInfo<int>() { return Type::Integer; }
template <> inline Type GetTypeInfo<u32>() { return Type::U32; }
template <> inline Type GetTypeInfo<float>() { return Type::Float; }
template <> inline Type GetTypeInfo<bool>() { return Type::Boolean; }

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

	if(fromType == Type::Integer) {
		if(toType == Type::U32) {
			*((u32*)toData) = *((int*)fromData);
			return true;
		}
		if(toType == Type::Float) {
			*((float*)toData) = (float)*((int*)fromData);
			return true;
		}
		if(toType == Type::Boolean) {
			*((bool*)toData) = *((int*)fromData) ? true : false;
			return true;
		}
		return false;
	}

	if(fromType == Type::U32) {
		if(toType == Type::Integer) {
			*((int*)toData) = *((u32*)fromData);
			return true;
		}
		if(toType == Type::Float) {
			*((float*)toData) = (float)*((u32*)fromData);
			return true;
		}

		if(toType == Type::Boolean) {
			*((bool*)toData) = *((u32*)fromData) ? true : false;
			return true;
		}
		return false;
	}

	if(fromType == Type::Float) {
		if(toType == Type::Integer) {
			*((int*)toData) = (int)*((float*)fromData);
			return true;
		}
		if(toType == Type::U32) {
			*((u32*)toData) = (u32)*((float*)fromData);
			return true;
		}
		return false;
	}

	if(fromType == Type::Boolean) {
		if(toType == Type::Integer) {
			*((int*)toData) = *((bool*)fromData) ? 1 : 0;
			return true;
		}
		if(toType == Type::U32) {
			*((u32*)toData) = *((bool*)fromData) ? 1 : 0;
			return true;
		}
		if(toType == Type::Float) {
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

	if(fromType == Type::Integer) {
		if(toType == Type::U32) {
			return true;
		}
		if(toType == Type::Float) {
			return true;
		}
		if(toType == Type::Boolean) {
			return true;
		}
		return false;
	}

	if(fromType == Type::U32) {
		if(toType == Type::Integer) {
			return true;
		}
		if(toType == Type::Float) {
			return true;
		}

		if(toType == Type::Boolean) {
			return true;
		}
		return false;
	}

	if(fromType == Type::Float) {
		if(toType == Type::Integer) {
			return true;
		}
		if(toType == Type::U32) {
			return true;
		}
		return false;
	}

	if(fromType == Type::Boolean) {
		if(toType == Type::Integer) {
			return true;
		}
		if(toType == Type::U32) {
			return true;
		}
		if(toType == Type::Float) {
			return true;
		}
		return false;
	}

	return false;
}

} // !namespace core
} // !namespace lux

#endif // !INCLUDED_LXTYPES_H
