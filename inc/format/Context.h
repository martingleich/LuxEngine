#ifndef INCLUDED_FORMAT_CONTEXT_H
#define INCLUDED_FORMAT_CONTEXT_H
#include "format/FormatConfig.h"
#include "format/FormatMemory.h"
#include <string>

namespace format
{
class Sink;
class Locale;

struct Cursor
{
	size_t line;
	size_t collumn;
};

//! A string in the format system.
/**
slices are the fundamental element of the format system.
Slices just contain normal stringdata, and a pointer to the next slice in the output
All the connected slices of a series create a full output string.
*/
struct Slice
{
	size_t size;  //!< Number of bytes in the string
	const char* data; //!< Data of the string, is not null-termainted

	Slice() :
		size(0),
		data(nullptr)
	{
	}

	Slice(size_t len, const char* d) :
		size(len),
		data(d)
	{
	}
};

class Context
{
public:
	struct SubContext
	{
		size_t fstrLastArgPos;
		size_t argId;
		Slice fstr;
	};

	struct AutoRestoreSubContext : public SubContext
	{
		Context& _ctx;
		AutoRestoreSubContext(Context& ctx, Slice fmtString) :
			_ctx(ctx)
		{
			*(SubContext*)this = ctx.SaveSubContext(fmtString);
		}
		~AutoRestoreSubContext()
		{
			_ctx.RestoreSubContext(*this);
		}
	};

	struct SliceMemory
	{
		static const size_t BLOCK_SIZE = 64;

		struct Block
		{
			Block* next;
			size_t used;

			char data[BLOCK_SIZE];
		};

		class Iterator
		{
		public:
			Iterator() :
				ptr(nullptr)
			{
			}
			Iterator(const Slice* _ptr, const Block* b) :
				ptr(_ptr),
				block(b)
			{
			}

			void Next()
			{
				if((void*)(ptr + 1) < (void*)(block->data + BLOCK_SIZE)) {
					++ptr;
					return;
				} else {
					block = block->next;
					ptr = (Slice*)block->data;
				}
			}

			Iterator& operator++()
			{
				Next();
				return *this;
			}
			Iterator operator++(int)
			{
				Iterator tmp(*this);
				this->Next();
				return tmp;
			}

			const Slice& operator*()
			{
				return *ptr;
			}

			const Slice* operator->()
			{
				return ptr;
			}
			bool operator==(const Iterator& other) const
			{
				return ptr == other.ptr;
			}
			bool operator!=(const Iterator& other) const
			{
				return ptr != other.ptr;
			}

		private:
			const Slice* ptr;
			const Block* block;
		};

		Block firstBlock;
		Block* curBlock;

		SliceMemory()
		{
			firstBlock.next = 0;
			firstBlock.used = 0;
			curBlock = &firstBlock;
		}

		~SliceMemory()
		{
			Clear();
		}

		void Clear()
		{
			Block* b = firstBlock.next;
			while(b) {
				Block* next = b->next;
				free(b);
				b = next;
			}
			firstBlock.used = 0;
			firstBlock.next = nullptr;
			curBlock = &firstBlock;
		}

		Slice* Alloc()
		{
			const size_t bytes = sizeof(Slice);
			void* out;
			if(!curBlock || curBlock->used + bytes > BLOCK_SIZE) {
				Block* b = (Block*)malloc(sizeof(Block));
				if(!b)
					throw std::bad_alloc();
				b->used = 0;
				b->next = nullptr;

				curBlock = b;
				curBlock->next = b;
			}
			out = curBlock->data + curBlock->used;
			curBlock->used += bytes;

			return (Slice*)out;
		}

		Iterator Begin() const
		{
			return Iterator((Slice*)firstBlock.data, &firstBlock);
		}
		Iterator End() const
		{
			auto it = Last();
			++it;
			return it;
		}
		Iterator Last() const
		{
			return Iterator((Slice*)(curBlock->data + curBlock->used - sizeof(Slice)), curBlock);
		}
	};

public:
	// Parsing information, for error reporting
	size_t fstrLastArgPos; // Position of the last parsed argument
	size_t argId; // Index of the current arguments

public:
	FORMAT_API Context(
		const Locale* locale = nullptr,
		size_t startCollum = 0,
		size_t startLine = 0);

	FORMAT_API void Reset(
		const Locale* locale = nullptr,
		size_t startCollumn = 0,
		size_t startLine = 0);

	FORMAT_API void AddSlice(size_t size, const char* data, bool forceCopy = false);
	void AddSlice(Slice s, bool forceCopy = false)
	{
		AddSlice(s.size, s.data, forceCopy);
	}
	void AddSlice(const std::string& str, bool forceCopy = false)
	{
		AddSlice(str.size(), str.data(), forceCopy);
	}
	void AddTerminatedSlice(const char* data, bool forceCopy = false)
	{
		AddSlice(strlen(data), data, forceCopy);
	}
	FORMAT_API Slice* AddLockedSlice();

	FORMAT_API size_t GetLine() const;
	FORMAT_API size_t GetCollumn() const;

	size_t StartCounting()
	{
		++m_Counting;
		return m_CharacterCount;
	}

	size_t StopCounting()
	{
		--m_Counting;
		return m_CharacterCount;
	}

	char* AllocByte(size_t len)
	{
		return (char*)m_Memory.Alloc(len);
	}

	struct SlicesT
	{
		SliceMemory::Iterator _begin;
		SliceMemory::Iterator _end;
		SliceMemory::Iterator begin() const { return _begin; }
		SliceMemory::Iterator end() const { return _end; }
	};
	SlicesT Slices() const
	{
		return{m_SliceMemory.Begin(), m_SliceMemory.End()};
	}

	FORMAT_API SubContext SaveSubContext(Slice newFmtString);
	FORMAT_API void RestoreSubContext(const SubContext& ctx);

	Slice GetFormatString() const
	{
		return m_FmtString;
	}

	const Locale* GetLocale() const
	{
		return m_Locale;
	}

	size_t GetSize() const
	{
		return m_Size;
	}

private:
	void EnsureCursor() const;

private:
	Slice m_FmtString;

	// Memory information
	internal::FormatMemory m_Memory;
	SliceMemory m_SliceMemory;
	Slice* m_LastSlice;

	// Output information
	mutable size_t m_Line;
	mutable size_t m_Collumn;
	size_t m_CharacterCount;
	int m_Counting;
	size_t m_Size;

	mutable SliceMemory::Iterator m_CursorSlice;
	mutable size_t m_CursorPos;

	const Locale* m_Locale;

	bool m_ForceSlice;
};

}

#endif // #ifndef INCLUDED_FORMAT_CONTEXT_H
