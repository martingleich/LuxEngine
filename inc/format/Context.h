#ifndef INCLUDED_FORMAT_CONTEXT_H
#define INCLUDED_FORMAT_CONTEXT_H
#include "format/FormatConfig.h"
#include "format/FormatMemoryFwd.h"
#include <string>

namespace format
{
class Sink;
class Locale;

struct Placeholder
{
	int argId;
	Slice type;
	Slice format;
};

class Context;
class FormatEntry
{
public:
	virtual void Convert(Context& ctx, Placeholder& placeholder) const = 0;
	virtual int AsInteger() const = 0;
};

class Context
{
public:
	struct SubContext;
	friend struct SubContext;
	struct SubContext
	{
		int curPlaceholderOffset;
		int curArgId;
		Context& _ctx;
		SubContext(SubContext&) = delete;
		SubContext& operator=(SubContext&) = delete;

		explicit SubContext(Context& ctx) :
			_ctx(ctx)
		{
			curPlaceholderOffset = _ctx.m_CurPlaceholderOffset;
			curArgId = _ctx.m_CurArgId;
			_ctx.m_CurPlaceholderOffset = 0;
			_ctx.m_CurArgId = 0;
		}
		~SubContext()
		{
			_ctx.m_CurPlaceholderOffset = curPlaceholderOffset;
			_ctx.m_CurArgId = curArgId;
		}
	};

	struct OutputStateContext;
	friend struct OutputStateContext;
	struct OutputStateContext
	{
		FormatMemoryState memState;
		Context& ctx;
		SliceMemoryIterator m_CurSlice;
		FORMAT_API OutputStateContext(Context& _ctx);
		FORMAT_API Slice CreateSlice();
		FORMAT_API ~OutputStateContext();
	};

	struct SlicesT
	{
		SliceMemoryIterator _begin;
		SliceMemoryIterator _end;
		SliceMemoryIterator begin() const { return _begin; }
		SliceMemoryIterator end() const { return _end; }
	};

public:
	Context(const Context& other) = delete;
	Context& operator=(const Context& other) = delete;
	FORMAT_API Context(const Locale& locale);
	FORMAT_API ~Context();

	// Add string data to the context.
	FORMAT_API void AddSlice(int size, const char* data, bool forceCopy = false);
	void AddSlice(Slice s, bool forceCopy = false)
	{
		AddSlice(s.size, s.data, forceCopy);
	}
	void AddSlice(const std::string& str, bool forceCopy = true)
	{
		AddSlice(int(str.size()), str.data(), forceCopy);
	}
	void AddTerminatedSlice(const char* data, bool forceCopy = false)
	{
		AddSlice(int(std::strlen(data)), data, forceCopy);
	}

	Slice CreateFreeSlice(int size, const char* data, bool forceCopy = false)
	{
		if(forceCopy) {
			char* newData = AllocByte(size);
			for(int i = 0; i < size; ++i)
				newData[i] = data[i];
			data = newData;
		}
		return Slice(size, data);
	}

	Slice CreateFreeTerminatedSlice(const char* data, bool forceCopy = false)
	{
		return CreateFreeSlice(int(std::strlen(data)), data, forceCopy);
	}
	Slice CreateFreeSlice(const std::string& str, bool forceCopy = true)
	{
		return CreateFreeSlice(int(str.size()), str.data(), forceCopy);
	}

	FORMAT_API Slice AddWorkingSlice(int size);
	FORMAT_API void EndWorkingSlice(Slice slice, int usedSize);

	FORMAT_API char* AllocByte(int len, int align = 1);

	FORMAT_API SlicesT Slices() const;

	const Locale* GetLocale() const { return &m_Locale; }
	int GetSize() const { return m_Size; }

	void SetCurPlaceholderOffset(int placeholderOffset)
	{
		m_CurPlaceholderOffset = placeholderOffset;
	}
	int GetCurPlaceholderOffset() const
	{
		return m_CurPlaceholderOffset;
	}

	void SetCurArgId(int argId)
	{
		m_CurArgId = argId;
	}
	int GetCurArgId() const
	{
		return m_CurArgId;
	}

	void SetFormatEntries(void* base, int count, int stride)
	{
		m_FormatEntries = base;
		m_FormatEntriesCount = count;
		m_FormatEntryStride = stride;
	}
	FormatEntry* GetFormatEntry(int id);

	// TODO: Add memory save/restore functions.

private:
	// Memory information
	FormatMemory& m_Memory;
	FormatMemoryState m_MemoryRestoreState;

	Slice* m_LastSlice;

	// Output information
	int m_Size;

	// Current state
	void* m_FormatEntries;
	int m_FormatEntriesCount;
	int m_FormatEntryStride;

	const Locale& m_Locale;

	int m_CurPlaceholderOffset;
	int m_CurArgId;

	bool m_HasWorkingSlice;
	bool m_ForceCopy=false;
};

}

#endif // #ifndef INCLUDED_FORMAT_CONTEXT_H
