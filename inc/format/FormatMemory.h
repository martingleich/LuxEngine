#ifndef INCLUDED_FORMAT_FORMAT_MEMORY_H
#define INCLUDED_FORMAT_FORMAT_MEMORY_H
#include "FormatMemoryFwd.h"
#include <cstring>
#include <cstdlib>
#include <vector>
#include <new>
#include <cassert>

namespace format
{
class BlockBasedAllocator
{
public:
	BlockBasedAllocator(const BlockBasedAllocator&) = delete;
	BlockBasedAllocator& operator=(const BlockBasedAllocator&) = delete;

	BlockBasedAllocator() :
		firstBlock(nullptr, nullptr, 0),
		curBlock(&firstBlock),
		lastAlloc(firstBlock.data)
	{
	}
	~BlockBasedAllocator()
	{
		auto b = firstBlock.next;
		while(b) {
			auto next = b->next;
			// Begin Exception safe.
			delete b;
			b = next;
			// End Exception safe.
		}
	}

	void Clear()
	{
		BlockBaseAllocatorState clearState;
		clearState.curBlock = &firstBlock;
		clearState.used = 0;
		RestoreState(clearState);
	}

	void* Alloc(int bytes, int align)
	{
		if(bytes > Block::SIZE)
			return nullptr;
		int alignBytes = curBlock->used % align != 0 ? align - curBlock->used % align : 0;
		bytes += alignBytes;
		auto prevBlock = curBlock;
		while(curBlock && curBlock->used + bytes > Block::SIZE) {
			prevBlock = curBlock;
			curBlock = curBlock->next;
		}

		if(!curBlock) {
			Block* b = new Block(nullptr, prevBlock, 0);
			// Begin Exception safe.
			prevBlock->next = b;
			curBlock = b;
			// End Exception safe.
		}

		void* out = curBlock->data + curBlock->used + alignBytes;
		curBlock->used += bytes;
		lastAlloc = out;
		return out;
	}

	bool TryExpand(const void* p, int expand)
	{
		if(p != lastAlloc)
			return false;
		assert(p >= curBlock->data && p <= curBlock->data + Block::SIZE);

		if(curBlock->used + expand <= Block::SIZE) {
			curBlock->used += expand;
			return true;
		}

		return false;
	}

	const Block* GetFirst() const { return &firstBlock; }
	const Block* GetCur() const { return curBlock; }

	BlockBaseAllocatorState GetState() const
	{
		BlockBaseAllocatorState state;
		state.curBlock = curBlock;
		state.used = curBlock->used;
		return state;
	}
	void RestoreState(BlockBaseAllocatorState state)
	{
		static_assert(std::is_trivially_destructible<Block>::value, "Block must be trivially destructible");

		Block* b = curBlock;
		while(b != state.curBlock) {
			b->used = 0;
			b = b->prev;
		}
		b->used = state.used;
		curBlock = state.curBlock;
		lastAlloc = nullptr;
	}

private:
	Block firstBlock;
	Block* curBlock;

	const void* lastAlloc;
};

class FormatMemory
{
public:
	FormatMemory() = default;
	FormatMemory(const FormatMemory&) = delete;
	~FormatMemory() = default;
	FormatMemory& operator=(const FormatMemory&) = delete;

	void Clear()
	{
		m_SliceAllocator.Clear();
		m_TextAllocator.Clear();
		m_TextFreeAlloc.clear();
	}

	void* AllocBytes(int bytes, int align)
	{
		if(bytes > Block::SIZE) {
			auto defaultAlign = alignof(std::max_align_t);
			if(defaultAlign % align != 0) {
				m_TextFreeAlloc.emplace_back(new char[bytes + align - 1]);
				auto ptr = m_TextFreeAlloc.back().ptr;
				uintptr_t intPtr = (uintptr_t)ptr;
				intPtr = ((intPtr + align - 1) / align)*align;
				return (void*)intPtr;
			} else {
				m_TextFreeAlloc.emplace_back(new char[bytes]);
				return m_TextFreeAlloc.back().ptr;
			}
		} else {
			return m_TextAllocator.Alloc(bytes, align);
		}
	}

	bool TryExpand(const void* p, int expand)
	{
		return m_TextAllocator.TryExpand(p, expand);
	}

	Slice* AllocSlice(int size, const char* data)
	{
		auto out = (Slice*)m_SliceAllocator.Alloc(sizeof(Slice), alignof(Slice));
		*out = Slice(size, data);
		return out;
	}

	SliceMemoryIterator BeginSlice() const
	{
		auto firstBlock = m_SliceAllocator.GetFirst();
		return SliceMemoryIterator(
			(Slice*)firstBlock->data,
			firstBlock);
	}
	SliceMemoryIterator EndSlice() const
	{
		return SliceMemoryIterator(
			(Slice*)(((char*)nullptr) + offsetof(Block, data)),
			nullptr);
	}
	SliceMemoryIterator LastSlice() const
	{
		auto curBlock = m_SliceAllocator.GetCur();
		return SliceMemoryIterator(
			(Slice*)(curBlock->data + curBlock->used - sizeof(Slice)),
			curBlock);
	}

	FormatMemoryState GetState() const
	{
		FormatMemoryState state;
		state.sliceState = m_SliceAllocator.GetState();
		state.textState = m_TextAllocator.GetState();
		state.freeState = int(m_TextFreeAlloc.size());
		return state;
	}
	void RestoreState(const FormatMemoryState& state)
	{
		m_TextAllocator.RestoreState(state.textState);
		m_SliceAllocator.RestoreState(state.sliceState);
		m_TextFreeAlloc.resize(state.freeState);
	}

private:
	BlockBasedAllocator m_TextAllocator;
	BlockBasedAllocator m_SliceAllocator;

	struct FreeAlloc
	{
		void* ptr;
		FreeAlloc() :
			ptr(nullptr)
		{
		}
		FreeAlloc(void* _ptr) :
			ptr(_ptr)
		{
		}
		FreeAlloc(const FreeAlloc&) = delete;
		~FreeAlloc()
		{
			delete[] (char*)ptr;
		}
		FreeAlloc(FreeAlloc&& old) :
			ptr(old.ptr)
		{
		}
		FreeAlloc& operator=(FreeAlloc& old) = delete;
		FreeAlloc& operator=(FreeAlloc&& old)
		{
			delete[] (char*)ptr;
			ptr = old.ptr;
			return *this;
		}
	};
	std::vector<FreeAlloc> m_TextFreeAlloc;
};

}

#endif // #ifndef INCLUDED_FORMAT_FORMAT_MEMORY_H
