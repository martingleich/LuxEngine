#ifndef INCLUDED_FORMAT_FORMAT_MEMORY_H
#define INCLUDED_FORMAT_FORMAT_MEMORY_H
#include <string.h>
#include <stdlib.h>
#include <new>

namespace format
{
namespace internal
{
	struct FormatMemory
	{
		static const size_t FIXED_MEMORY = 64;
		static const size_t BLOCK_SIZE = 256;

		char fixed[FIXED_MEMORY];
		struct Block
		{
			Block* next;
			size_t used;

			char data[BLOCK_SIZE];
		};

		size_t used;
		size_t fixed_used;
		Block* firstBlock;
		Block* curBlock;
		void** freeAlloc;
		size_t freeAllocCount;
		void* lastAlloc;
		size_t lastAllocSize;

		FormatMemory() :
			used(0),
			fixed_used(0),
			firstBlock(nullptr),
			curBlock(nullptr),
			freeAlloc(nullptr),
			freeAllocCount(0),
			lastAlloc(nullptr),
			lastAllocSize(0)
		{
		}

		~FormatMemory()
		{
			Clear();
		}

		void* Alloc(size_t bytes)
		{
			if(bytes > BLOCK_SIZE) {
				void** newData = (void**)realloc((void*)freeAlloc, sizeof(void*)*(freeAllocCount + 1));
				if(!newData)
					throw std::bad_alloc();
				freeAlloc = newData;
				void* newBytes = malloc(bytes);
				freeAlloc[freeAllocCount] = newBytes;
				++freeAllocCount;

				used += bytes;
				lastAlloc = newBytes;
			} else if(fixed_used + bytes <= FIXED_MEMORY) {
				void* out = fixed + fixed_used;
				fixed_used += bytes;
				used += bytes;
				lastAlloc = out;
			} else {
				if(!curBlock) {
					Block* b = (Block*)malloc(sizeof(Block));
					if(!b)
						throw std::bad_alloc();
					curBlock = firstBlock = b;
					curBlock->next = nullptr;
					curBlock->used = 0;
				}


				if(curBlock->used + bytes > BLOCK_SIZE) {
					Block* b = (Block*)malloc(sizeof(Block));
					if(!b)
						throw std::bad_alloc();
					b->next = nullptr;
					b->used = 0;
					curBlock->next = b;
					curBlock = b;
				}

				void* out = curBlock->data + curBlock->used;
				curBlock->used += bytes;
				used += bytes;
				lastAlloc = out;
			}

			lastAllocSize = bytes;

			return lastAlloc;
		}

		bool Unalloc(const void* p, size_t bytes)
		{
			if(p != lastAlloc)
				return false;

			if(p >= fixed && p < fixed + FIXED_MEMORY && bytes <= fixed_used)
				fixed_used -= bytes;
			else if(bytes <= curBlock->used)
				curBlock->used -= bytes;
			else
				return false;

			used -= bytes;

			return true;
		}

		void* Realloc(void* p, size_t newSize)
		{
			if(p != lastAlloc)
				return nullptr;

			if(freeAllocCount && p == freeAlloc[freeAllocCount - 1]) {
				void* n = realloc(p, newSize);
				if(!n)
					throw std::bad_alloc();
				p = n;
				freeAlloc[freeAllocCount - 1] = p;
				return p;
			}

			size_t blockSize = lastAllocSize;
			Unalloc(p, blockSize);
			void* newP = Alloc(newSize);
			if(newP != p) {
				size_t copy = blockSize < newSize ? blockSize : newSize;
				memcpy(newP, p, copy);
			}

			return newP;
		}

		bool TryExpand(const void* p, size_t expand)
		{
			if(p != lastAlloc)
				return false;

			if(p >= fixed && p <= fixed + FIXED_MEMORY) {
				if(fixed_used + expand <= FIXED_MEMORY) {
					fixed_used += expand;
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

		void Clear()
		{
			while(firstBlock) {
				Block* next = firstBlock->next;
				free(firstBlock);
				firstBlock = next;
			}

			for(size_t i = 0; i < freeAllocCount; ++i)
				free(freeAlloc[i]);

			free(freeAlloc);
			freeAlloc = nullptr;

			used = 0;
			fixed_used = 0;
		}
	};
}
}

#endif // #ifndef INCLUDED_FORMAT_FORMAT_MEMORY_H
