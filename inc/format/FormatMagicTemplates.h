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
	Will force all floating-point types to double.
	Will force all integer types to intmax_t or uintmax_t
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

	template <>
	struct format_type_conv<float>
	{
		static double get(float x) { return (double)x; }
	};

	template <>
	struct format_type_conv<signed char>
	{
		static intmax_t get(signed char x) { return (intmax_t)x; }
	};

	template <>
	struct format_type_conv<short>
	{
		static intmax_t get(short x) { return (intmax_t)x; }
	};

	template <>
	struct format_type_conv<int>
	{
		static intmax_t get(int x) { return (intmax_t)x; }
	};

	template <>
	struct format_type_conv<long long>
	{
		static intmax_t get(long long x) { return (intmax_t)x; }
	};

	template <>
	struct format_type_conv<unsigned char>
	{
		static uintmax_t get(unsigned char x) { return (uintmax_t)x; }
	};

	template <>
	struct format_type_conv<unsigned short>
	{
		static uintmax_t get(unsigned short x) { return (uintmax_t)x; }
	};

	template <>
	struct format_type_conv<unsigned int>
	{
		static uintmax_t get(unsigned int x) { return (uintmax_t)x; }
	};

	template <>
	struct format_type_conv<unsigned long long>
	{
		static uintmax_t get(unsigned long long x) { return (uintmax_t)x; }
	};

	//! Converts the passed value to an integer.
	/**
	Currently convert integer data types(i.e. int, long long, etc.) to int.
	*/
	template <typename T>
	inline int GetAsInt(T)
	{
		throw value_exception("Passed placeholder value must be an integer.");
	}

	template <>
	inline int GetAsInt(long long arg)
	{
		assert(arg <= (long long)INT_MAX);
		return (int)arg;
	}

	template <>
	inline int GetAsInt(int arg)
	{
		return arg;
	}

	template <>
	inline int GetAsInt(short arg)
	{
		return arg;
	}

	template <>
	inline int GetAsInt(char arg)
	{
		return arg;
	}

	template <>
	inline int GetAsInt(unsigned long long arg)
	{
		assert(arg <= (unsigned long long)INT_MAX);
		return (int)arg;
	}

	template <>
	inline int GetAsInt(unsigned int arg)
	{
		assert(arg <= (unsigned int)INT_MAX);
		return arg;
	}

	template <>
	inline int GetAsInt(unsigned short arg)
	{
		return arg;
	}

	template <>
	inline int GetAsInt(unsigned char arg)
	{
		return arg;
	}

	template <>
	inline int GetAsInt(signed char arg)
	{
		return arg;
	}
}
}

#endif // #ifndef INCLUDED_FORMAT_FORMAT_MAGIC_TEMPLATES_H
