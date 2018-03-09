#ifndef INCLUDED_FORMAT_FORMAT_MEMORY_H
#define INCLUDED_FORMAT_FORMAT_MEMORY_H
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <new>

namespace format
{
namespace internal
{
	struct FormatMemory
	{
		static const size_t FIXED_MEMORY = 64;
		static const size_t BLOCK_SIZE = 256;
		static const size_t THRESHOLD = BLOCK_SIZE / 4;

		struct Block
		{
			Block* prev;
			size_t used;

			char data[BLOCK_SIZE];
		};

		size_t fixedUsed;
		char fixed[FIXED_MEMORY];
		Block* curBlock;

		std::vector<void*> freeAlloc;

		void* lastAlloc;
		size_t lastAllocSize;

		// Statistics
		size_t used;

		FormatMemory() :
			used(0),
			fixedUsed(0),
			curBlock(nullptr),
			lastAlloc(nullptr),
			lastAllocSize(0)
		{
		}

		~FormatMemory()
		{
			Clear();
		}

		void Clear()
		{
			while(curBlock) {
				Block* next = curBlock->prev;
				free(curBlock);
				curBlock = next;
			}

			for(auto p : freeAlloc)
				free(p);
			freeAlloc.clear();

			used = 0;
			fixedUsed = 0;
			lastAlloc = nullptr;
			lastAllocSize = 0;
		}

		void* Alloc(size_t bytes)
		{
			if(bytes > THRESHOLD) {
				void* newBytes = malloc(bytes);
				freeAlloc.push_back(newBytes);
				lastAlloc = newBytes;
			} else if(fixedUsed + bytes <= FIXED_MEMORY) {
				void* out = fixed + fixedUsed;
				fixedUsed += bytes;
				lastAlloc = out;
			} else {
				if(!curBlock || curBlock->used + bytes > BLOCK_SIZE) {
					Block* b = (Block*)malloc(sizeof(Block));
					if(!b)
						throw std::bad_alloc();
					b->prev = curBlock;
					b->used = 0;
					curBlock = b;
				}

				lastAlloc = curBlock->data + curBlock->used;
				curBlock->used += bytes;
			}

			used += bytes;
			lastAllocSize = bytes;
			return lastAlloc;
		}

		bool TryExpand(const void* p, size_t expand)
		{
			if(p != lastAlloc)
				return false;

			if(p >= fixed && p <= fixed + FIXED_MEMORY) {
				if(fixedUsed + expand <= FIXED_MEMORY) {
					fixedUsed += expand;
					used += expand;
					return true;
				}
			} else if(p >= curBlock->data && p <= curBlock->data + BLOCK_SIZE) {
				if(curBlock->used + expand <= BLOCK_SIZE) {
					curBlock->used += expand;
					used += expand;
					return true;
				}
			}

			return false;
		}

	};
}
}

#endif // #ifndef INCLUDED_FORMAT_FORMAT_MEMORY_H
