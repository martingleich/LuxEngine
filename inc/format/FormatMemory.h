#ifndef INCLUDED_FORMAT_FORMAT_MEMORY_H
#define INCLUDED_FORMAT_FORMAT_MEMORY_H
#include <cstring>
#include <cstdlib>
#include <vector>
#include <new>

namespace format
{
namespace internal
{
	struct FormatMemory
	{
		static const size_t BLOCK_SIZE = 64;
		static const size_t THRESHOLD = 64;

		struct Block
		{
			Block* prev;
			size_t used;

			char data[BLOCK_SIZE];
		};

		Block firstBlock;
		Block* curBlock;

		std::vector<void*> freeAlloc;

		void* lastAlloc;
		size_t lastAllocSize;

		FormatMemory() :
			curBlock(nullptr),
			lastAlloc(nullptr),
			lastAllocSize(0)
		{
			firstBlock.used = 0;
			firstBlock.prev = 0;
			curBlock = &firstBlock;
			curBlock->prev = nullptr;
		}

		~FormatMemory()
		{
			Clear();
		}

		void Clear()
		{
			while(curBlock != &firstBlock) {
				Block* prev = curBlock->prev;
				free(curBlock);
				curBlock = prev;
			}

			for(auto p : freeAlloc)
				free(p);
			freeAlloc.clear();

			lastAlloc = nullptr;
			lastAllocSize = 0;

			firstBlock.used = 0;
		}

		void* Alloc(size_t bytes)
		{
			void* out;
			if(bytes > THRESHOLD) {
				void* newBytes = malloc(bytes);
				freeAlloc.push_back(newBytes);
				out = newBytes;
				lastAlloc = nullptr; // Dont't record free allocs
			} else {
				if(!curBlock || curBlock->used + bytes > BLOCK_SIZE) {
					Block* b = (Block*)malloc(sizeof(Block));
					if(!b)
						throw std::bad_alloc();
					b->prev = curBlock;
					b->used = 0;
					curBlock = b;
				}

				out = curBlock->data + curBlock->used;
				curBlock->used += bytes;
				lastAlloc = out;
			}

			lastAllocSize = bytes;
			return out;
		}

		bool TryExpand(const void* p, size_t expand)
		{
			if(p != lastAlloc)
				return false;

			if(p >= curBlock->data && p <= curBlock->data + BLOCK_SIZE) {
				if(curBlock->used + expand <= BLOCK_SIZE) {
					curBlock->used += expand;
					return true;
				}
			}

			return false;
		}
	};
}
}

#endif // #ifndef INCLUDED_FORMAT_FORMAT_MEMORY_H
