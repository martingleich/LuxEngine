#ifndef INCLUDED_FORMAT_CONTEXT_H
#define INCLUDED_FORMAT_CONTEXT_H
#include "format/FormatConfig.h"
#include "format/FormatMemory.h"
#include "format/StringBasics.h"
#include "format/FormatLocale.h"
#include <string>

namespace format
{
class Sink;

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

class Context
{
public:
	struct SubContext
	{
		size_t fstrLastArgPos;
		size_t argId;
		const char* fstr;
	};

	struct AutoRestoreSubContext : public SubContext
	{
		Context& _ctx;
		AutoRestoreSubContext(Context& ctx, const char* fmtString) :
			_ctx(ctx)
		{
			*(SubContext*)this = ctx.SaveSubContext(fmtString);
		}
		~AutoRestoreSubContext()
		{
			_ctx.RestoreSubContext(*this);
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

	const Locale* GetLocale() const
	{
		return m_Locale;
	}

	size_t GetSize() const
	{
		return m_Size;
	}
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

	const char* GetFormatString() const
	{
		return m_FmtString;
	}

	const Slice* GetFirstSlice() const
	{
		return m_FirstSlice;
	}

	const Slice* GetLastSlice() const
	{
		return m_LastSlice;
	}

	Slice* LockLastSlice()
	{
		m_ForceSlice = true;
		return m_LastSlice;
	}

	void ClearMemory()
	{
		m_Memory.Clear();
		m_SliceMemory.Clear();
	}

	FORMAT_API SubContext SaveSubContext(const char* newFmtString);
	FORMAT_API void RestoreSubContext(const SubContext& ctx);

	FORMAT_API Slice* InsertSlice(Slice* prev, Slice sl);

private:
	Slice* AddSlice();
	void EnsureCursor() const;

private:
	const char* m_FmtString;

	// Memory information
	Slice* m_FirstSlice;
	Slice* m_LastSlice;

	internal::FormatMemory m_Memory;
	internal::FormatMemory m_SliceMemory;

	// Output information
	mutable size_t m_Line;
	mutable size_t m_Collumn;
	size_t m_CharacterCount;
	int m_Counting;
	size_t m_Size;

	mutable Slice* m_CursorSlice;
	mutable size_t m_CursorPos;

	const Locale* m_Locale;

	bool m_ForceSlice;
};

}

#endif // #ifndef INCLUDED_FORMAT_CONTEXT_H
