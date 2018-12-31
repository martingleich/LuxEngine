#ifndef INCLUDED_FORMAT_CONTEXT_FWD_H
#define INCLUDED_FORMAT_CONTEXT_FWD_H

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
	int size;  //!< Number of bytes in the string
	const char* data; //!< Data of the string, is not null-termainted

	Slice() :
		size(0),
		data(nullptr)
	{
	}

	Slice(int len, const char* d) :
		size(len),
		data(d)
	{
	}
};

inline bool operator==(const Slice& slice, char c)
{
	return slice.size == 1 && slice.data[0] == c;
}
inline bool operator==(char c, const Slice& slice)
{
	return slice.size == 1 && slice.data[0] == c;
}
inline bool operator!=(const Slice& slice, char c) { return !(slice == c); }
inline bool operator!=(char c, const Slice& slice) { return !(slice == c); }

inline bool operator==(const Slice& a, const Slice& b)
{
	if(a.size != b.size)
		return false;
	for(int i = 0; i < a.size; ++i) {
		if(a.data[i] != b.data[i])
			return false;
	}
	return true;
}

struct Block
{
	static const int SIZE = 256;
	Block* next;
	Block* prev;
	int used;

	char data[SIZE];

	Block(Block* _next, Block* _prev, int _used) :
		next(_next),
		prev(_prev),
		used(_used)
	{
	}
};

class SliceMemoryIterator
{
public:
	SliceMemoryIterator(const Slice* _ptr, const Block* b) :
		ptr(_ptr),
		block(b)
	{
	}

	SliceMemoryIterator& operator++()
	{
		if(!block)
			return *this;
		auto next = ptr + 1;
		auto blockEnd = (Slice*)(block->data + block->used);
		if(next < blockEnd) {
			ptr = next;
		} else {
			block = block->next;
			ptr = (Slice*)block->data;
		}
		return *this;
	}

	SliceMemoryIterator operator++(int)
	{
		SliceMemoryIterator tmp(*this);
		++(*this);
		return tmp;
	}

	const Slice& operator*() { return *ptr; }
	const Slice* operator->() { return ptr; }
	bool operator==(const SliceMemoryIterator& other) const { return ptr == other.ptr; }
	bool operator!=(const SliceMemoryIterator& other) const { return ptr != other.ptr; }

private:
	const Slice* ptr;
	const Block* block;
};

struct BlockBaseAllocatorState
{
	Block* curBlock;
	int used;
};

struct FormatMemoryState
{
	BlockBaseAllocatorState sliceState;
	BlockBaseAllocatorState textState;
	int freeState;
};

class BlockBasedAllocator;
class FormatMemory;

}

#endif // #ifndef INCLUDED_FORMAT_CONTEXT_FWD_H
