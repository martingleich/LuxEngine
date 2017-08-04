#ifndef INCLUDED_FORMAT_SLICE_H
#define INCLUDED_FORMAT_SLICE_H
#include <stddef.h>

namespace format
{
//! A string in the format system.
/**
slices are the fundamental element of the format system.
Slices just contain normal stringdata, and a pointer to the next slice in the output
All the connected slices of a series create a full output string.
*/
struct Slice
{
	friend class Context;

	size_t size;  //!< Number of bytes in the string
	const char* data; //!< Data of the string, is not null-termainted

	Slice() :
		size(0),
		data(nullptr),
		next(nullptr)
	{
	}

	Slice(size_t len, const char* d) :
		size(len),
		data(d),
		next(nullptr)
	{
	}

	//! Get the next string in the list
	Slice* GetNext()
	{
		return next;
	}

	//! Get the next string in the list
	const Slice* GetNext() const
	{
		return next;
	}

private:
	Slice* next;
};

}
#endif // #ifndef INCLUDED_FORMAT_SLICE_H
