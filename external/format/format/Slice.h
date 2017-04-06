#pragma once

namespace format
{
//! A string in the format system.
/**
slices are the fundamental element of the format system.
Simply said a slice is a normal string.
It contains a pointer to the string-data and the number
of characters in the string.
The string doesn't have to be NULL-terminated.
*/
struct slice
{
	friend class Context;

	size_t size;  // Number of bytes in the string
	const char* data;

	slice() :
		size(0),
		data(nullptr),
		next(nullptr)
	{
	}

	slice(size_t len, const char* d) :
		size(len),
		data(d),
		next(nullptr)
	{
	}

	slice* GetNext()
	{
		return next;
	}

	slice* GetNext() const
	{
		return next;
	}

private:
	slice* next;
};

}