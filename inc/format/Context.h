#ifndef INCLUDED_FORMAT_CONTEXT_H
#define INCLUDED_FORMAT_CONTEXT_H
#include "format/FormatConfig.h"
#include "format/Slice.h"
#include "format/FormatMemory.h"
#include "format/StringType.h"
#include "format/StringBasics.h"
#include <limits.h>

namespace format
{
class Sink;
class Locale;

struct Cursor
{
	size_t count;
	size_t line;
	size_t collumn;
};

class Context
{
public:
	struct SubContext
	{
		StringType stringType;
		StringType fstrType;
		size_t fstrPos;
		size_t fstrLastArgPos;
		size_t argId;
		const char* fstr;
	};

public:
	StringType stringType;
	StringType fstrType;

	Sink* dstSink;

	size_t fstrPos;
	size_t fstrLastArgPos;
	size_t argId;
	const char* fstr;

public:
	FORMAT_API Context();

	FORMAT_API Slice* AddSlice(size_t size, const char* data, bool forceCopy = false, const Cursor* curDiff = nullptr);
	FORMAT_API Slice* InsertSlice(Slice* prev, Slice sl);

	void SetLocale(const Locale* loc)
	{
		m_Locale = loc;
	}

	const Locale* GetLocale() const
	{
		return m_Locale;
	}

	void SetDstStringType(StringType type)
	{
		stringType = type;
	}

	void SetFmtStringType(StringType type)
	{
		fstrType = type;
	}

	size_t GetLine() const
	{
		return m_Line;
	}

	size_t GetCharacterCount() const
	{
		return m_CharacterCount;
	}

	FORMAT_API size_t GetCollumn() const;

	char* AllocByte(size_t len)
	{
		return (char*)m_Memory.Alloc(len);
	}

	char* ReallocByte(char* ptr, size_t newlen)
	{
		return (char*)m_Memory.Realloc(ptr, newlen);
	}

	void UnallocByte(char* ptr)
	{
		m_Memory.Unalloc(ptr, m_Memory.lastAllocSize);
	}

	Slice* AddSlice(Slice s)
	{
		return AddSlice(s.size, s.data);
	}

	const Slice* GetFirstSlice() const
	{
		return m_FirstSlice;
	}

	const Slice* GetLastSlice() const
	{
		return m_LastSlice;
	}

	Slice* GetLastSlice()
	{
		m_ForceSlice = true;
		return m_LastSlice;
	}

	void ClearMemory()
	{
		m_Memory.Clear();
		m_SliceMemory.Clear();
	}

	FORMAT_API SubContext SaveSubContext() const;
	FORMAT_API void RestoreSubContext(const SubContext& ctx);

private:
	Slice* AddSlice();

private:
	// Memory information
	Slice* m_FirstSlice;
	Slice* m_LastSlice;
	bool m_ForceSlice;
	internal::FormatMemory m_Memory;
	internal::FormatMemory m_SliceMemory;

	// Output information
	size_t m_Line;
	size_t m_Collumn;
	size_t m_CharacterCount;

	mutable size_t m_SinkCollumn; // Cache for collumn returned by sink.

	const Locale* m_Locale;
};

}

#endif // #ifndef INCLUDED_FORMAT_CONTEXT_H
