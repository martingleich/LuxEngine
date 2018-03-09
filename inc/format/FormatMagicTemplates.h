#ifndef INCLUDED_FORMAT_FORMAT_MAGIC_TEMPLATES_H
#define INCLUDED_FORMAT_FORMAT_MAGIC_TEMPLATES_H
#include <limits.h>
#include <assert.h>

namespace format
{
namespace internal
{
	//! Type normalization for format types
	/**
	Will reduce number of template instanciations of format function.
	For example with map "const T[N]" to "const T*", for all T and N.
	*/
	template <typename T>
	struct format_type_conv
	{
		static const T& get(const T& x) { return x; }
	};

	template <typename T, size_t N>
	struct format_type_conv<T[N]>
	{
		static const T* get(const T* x) { return x; }
	};

	//! Converts the passed value to an integer.
	/**
	Currently convert integer data types(i.e. int, long long, etc.) to int.
	*/
	template <typename T>
	inline int GetAsInt(const T&)
	{
		throw value_exception("Passed placeholder value must be an integer.");
	}

	template <>
	inline int GetAsInt(const long long& arg)
	{
		assert(arg <= (long long)INT_MAX);
		return (int)arg;
	}

	template <>
	inline int GetAsInt(const int& arg)
	{
		return arg;
	}

	template <>
	inline int GetAsInt(const short& arg)
	{
		return arg;
	}

	template <>
	inline int GetAsInt(const char& arg)
	{
		return arg;
	}

	template <>
	inline int GetAsInt(const unsigned long long& arg)
	{
		assert(arg <= (unsigned long long)INT_MAX);
		return (int)arg;
	}

	template <>
	inline int GetAsInt(const unsigned int& arg)
	{
		assert(arg <= (unsigned int)INT_MAX);
		return arg;
	}

	template <>
	inline int GetAsInt(const unsigned short& arg)
	{
		return arg;
	}

	template <>
	inline int GetAsInt(const unsigned char& arg)
	{
		return arg;
	}

	template <>
	inline int GetAsInt(const signed char& arg)
	{
		return arg;
	}
}
}

#endif // #ifndef INCLUDED_FORMAT_FORMAT_MAGIC_TEMPLATES_H
