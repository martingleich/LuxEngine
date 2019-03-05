#pragma once
#include <intrin.h>
#include "lxIterator.h"

namespace lux
{
namespace core
{

template <typename T>
class EnumSet
{
	static_assert(std::is_enum<T>::value || std::is_integral<T>::value, "T must an enum or integral type.");
	using BaseType = typename core::Choose<std::is_enum<T>::value, typename std::underlying_type<T>::type, T>::type;
	using BitsType = u32;

private:
	constexpr EnumSet(BitsType t) :
		m_Flags(t)
	{
	}

	static constexpr BitsType ToFlag(T value)
	{
#if defined(_DEBUG) | defined(LX_DEBUG_ENUM_SET)
		if(BaseType(value) < 0 || BaseType(value) > 32) {
			if(::lux::impl_assert::Report(__FILE__, __LINE__, "0<value<32", "All enum values used with EnumSet must be bigger than 0 and smaller than 32."))
				LUX_DEBUG_BREAK;
		}
#endif
		return BitsType(1) << BaseType(value);
	}

public:
	class ConstIterator : public lux::core::BaseIterator<lux::core::ForwardIteratorTag, T>
	{
		friend class EnumSet;
		explicit ConstIterator(BitsType _flags) :
			m_Flags(_flags),
			m_Value(0)
		{
			IncrementValueUntilHit();
		}
	public:
		ConstIterator() :
			m_Flags(0),
			m_Value(0)
		{}

		ConstIterator& operator++()
		{
			m_Flags ^= BitsType(1) << m_Value;
			IncrementValueUntilHit();
			return *this;
		}

		ConstIterator operator++(int)
		{
			ConstIterator tmp(*this);
			++(*this);
			return tmp;
		}

		bool operator==(const ConstIterator& other) const { return m_Flags == other.m_Flags; }
		bool operator!=(const ConstIterator& other) const { return m_Flags != other.m_Flags; }
		T operator*() const { return T(m_Value); }
	private:
		// Increment value until (1<<m_Value&m_Flags)!=0
		void IncrementValueUntilHit()
		{
#if defined(_MSC_VER)
			unsigned long index;
			unsigned char isNotZero = _BitScanForward(&index, m_Flags);
			m_Value = isNotZero?index:0;
#else
			// m_Value can never be bigger than the number of bits in BitsType.
			// Since all the bits left of the bit m_Value are 0(see operator++) there must be a 1 to the right of the bit m_Value.
			// Since when m_Flags is 0 the function is aborted.
			if(m_Flags != BitsType(0)) {
				while(((BitsType(1) << m_Value) & m_Flags) == BitsType(0))
					++m_Value;
			}
#endif
		}
	private:
		BitsType m_Flags;
		BaseType m_Value;
	};

	constexpr EnumSet() : m_Flags(0) {}
	constexpr EnumSet(T v1) : m_Flags(ToFlag(v1)) {}
	constexpr EnumSet(T v1, T v2) : m_Flags(ToFlag(v1) | ToFlag(v2)) {}
	constexpr EnumSet(T v1, T v2, T v3) : m_Flags(ToFlag(v1) | ToFlag(v2) | ToFlag(v3)) {}
	constexpr EnumSet(T v1, T v2, T v3, T v4) : m_Flags(ToFlag(v1) | ToFlag(v2) | ToFlag(v3) | ToFlag(v4)) {}

	template <typename T1, typename... TRest>
	constexpr EnumSet(T1 v1, TRest... vrest) :
		EnumSet(ToFlag(v1) | EnumSet(vrest...).m_Flags)
	{
	}

	constexpr bool IsEmpty() const { return m_Flags == 0; }

	int Count() const {
#if defined(_MSC_VER)
		// Expands to nothing if not on an x86 or x64 machine, otherwise to argument.
		__MACHINEX86_X64(return __popcnt(m_Flags));
#endif
		int count = 0;
		for(T v : *this)
			++count;
		return count;
	}

	constexpr EnumSet Intersect(EnumSet other) const {  return EnumSet(m_Flags & other.m_Flags); }
	constexpr EnumSet Intersect(T v1) const {  return Intersect(EnumSet(v1)); }
	constexpr EnumSet Intersect(T v1, T v2) const {  return Intersect(EnumSet(v1, v2)); }
	constexpr EnumSet Intersect(T v1, T v2, T v3) const {  return Intersect(EnumSet(v1, v2, v3)); }
	constexpr EnumSet Intersect(T v1, T v2, T v3, T v4) const {  return Intersect(EnumSet(v1, v2, v3, v4)); }

	constexpr EnumSet Join(EnumSet other) const {  return EnumSet(m_Flags | other.m_Flags); }
	constexpr EnumSet Join(T v1) const {  return Join(EnumSet(v1)); }
	constexpr EnumSet Join(T v1, T v2) const {  return Join(EnumSet(v1, v2)); }
	constexpr EnumSet Join(T v1, T v2, T v3) const {  return Join(EnumSet(v1, v2, v3)); }
	constexpr EnumSet Join(T v1, T v2, T v3, T v4) const { return Join(EnumSet(v1, v2, v3, v4)); }

	constexpr EnumSet Difference(EnumSet other) const {  return EnumSet(m_Flags & ~other.m_Flags); }
	constexpr EnumSet Difference(T v1) const {  return Difference(EnumSet(v1)); }
	constexpr EnumSet Difference(T v1, T v2) const {  return Difference(EnumSet(v1, v2)); }
	constexpr EnumSet Difference(T v1, T v2, T v3) const {  return Difference(EnumSet(v1, v2, v3)); }
	constexpr EnumSet Difference(T v1, T v2, T v3, T v4) const {  return Difference(EnumSet(v1, v2, v3, v4)); }

	constexpr bool Contains(T v1) const { return !Intersect(v1).IsEmpty(); }

	ConstIterator begin() const { return ConstIterator(m_Flags); }
	ConstIterator end() const { return ConstIterator(0); }

private:
	BitsType m_Flags;
};

} // namespace core
} // namespace lux
