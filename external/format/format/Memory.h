#pragma once
#include <vector>
#include <cstring>

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

			Block() :
				next(nullptr),
				used(0)
			{
			}

			char data[BLOCK_SIZE];
		};

		size_t used;
		size_t fixed_used;
		Block* firstBlock;
		Block* curBlock;
		std::vector<void*> freeAlloc;
		void* lastAlloc;
		size_t lastAllocSize;

		FormatMemory() :
			used(0),
			fixed_used(0),
			firstBlock(nullptr),
			curBlock(nullptr),
			lastAlloc(nullptr),
			lastAllocSize(0)
		{
		}

		void* Alloc(size_t bytes)
		{
			if(bytes > BLOCK_SIZE) {
				freeAlloc.push_back(malloc(bytes));
				used += bytes;
				lastAlloc = freeAlloc.back();
			} else if(fixed_used + bytes <= FIXED_MEMORY) {
				void* out = fixed + fixed_used;
				fixed_used += bytes;
				used += bytes;
				lastAlloc = out;
			} else {
				if(!curBlock)
					curBlock = firstBlock = new Block;

				if(curBlock->used + bytes > BLOCK_SIZE) {
					Block* b = new Block;
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

			if(!freeAlloc.empty() && p == freeAlloc.back()) {
				p = realloc(p, newSize);
				freeAlloc.back() = p;
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
				delete firstBlock;
				firstBlock = next;
			}

			for(auto it = freeAlloc.begin(); it != freeAlloc.end(); ++it)
				free(*it);

			freeAlloc.clear();

			used = 0;
			fixed_used = 0;
		}
	};
}
}
