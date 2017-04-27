#pragma once
#include "Slice.h"
#include "Memory.h"
#include "StringType.h"
#include "StringBasics.h"
#include <limits>

namespace format
{
class sink;
namespace locale
{
class Locale;
}

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

	sink* dstSink;

	size_t fstrPos;
	size_t fstrLastArgPos;
	size_t argId;
	const char* fstr;

public:
	Context() :
		stringType(Ascii),
                fstrType(Ascii),
                dstSink(nullptr),
                fstrPos(0),
                fstrLastArgPos(0),
                argId(0),
                fstr(nullptr),
		m_FirstSlice(nullptr),
		m_LastSlice(nullptr),
		m_ForceSlice(true),
		m_Line(0),
		m_Collumn(0),
		m_CharacterCount(0),
		m_SinkCollumn(std::numeric_limits<size_t>::max()),
		m_Locale(nullptr)
	{
	}

	void SetLocale(const locale::Locale* loc)
	{
		m_Locale = loc;
	}

	const locale::Locale* GetLocale() const
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

	size_t GetCollumn() const;

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

	slice* AddSlice(slice s)
	{
		return AddSlice(s.size, s.data);
	}

	slice* AddSlice(size_t size, const char* data, bool forceCopy = false, const Cursor* curDiff = nullptr)
	{
		slice* out;
		if(!m_ForceSlice && !forceCopy && m_LastSlice && m_LastSlice->data + m_LastSlice->size == data) {
			out = m_LastSlice;
			out->size += size;
		} else if(!m_ForceSlice && m_LastSlice && (size < sizeof(slice) || forceCopy) && m_Memory.TryExpand(m_LastSlice->data, size)) {
			memcpy(const_cast<char*>(m_LastSlice->data) + m_LastSlice->size, data, size);
			m_LastSlice->size += size;
			out = m_LastSlice;
		} else {
			out = AddSlice();
			if(forceCopy) {
				char* newData = AllocByte(size);
				memcpy(newData, data, size);
				data = newData;
			}
			out->size = size;
			out->data = data;
			out->next = nullptr;
		}

		m_ForceSlice = false;

		if(!curDiff) {
			const char* cur = data;
			while(size) {
				uint32_t c = AdvanceCursor(stringType, cur);
				--size;

				if(c == '\n') {
					m_Collumn = 0;
					m_Line++;
				} else {
					m_Collumn++;
				}
				++m_CharacterCount;
			}
		} else {
			if(curDiff->line) {
				m_Collumn = curDiff->collumn;
				m_Line = curDiff->line;
			} else {
				m_Collumn += curDiff->collumn;
			}
			m_CharacterCount += curDiff->count;
		}

		return out;
	}

	const slice* GetFirstSlice() const
	{
		return m_FirstSlice;
	}

	const slice* GetLastSlice() const
	{
		return m_LastSlice;
	}

	slice* GetLastSlice()
	{
		m_ForceSlice = true;
		return m_LastSlice;
	}

	void ClearMemory()
	{
		m_Memory.Clear();
		m_SliceMemory.Clear();
	}

	SubContext SaveSubContext() const
	{
		SubContext out;
		out.fstrLastArgPos = fstrLastArgPos;
		out.fstrPos = fstrPos;
		out.stringType = stringType;
		out.fstrType = fstrType;
		out.fstr = fstr;
		out.argId = argId;

		return out;
	}

	void RestoreSubContext(const SubContext& ctx)
	{
		fstrLastArgPos = ctx.fstrLastArgPos;
		fstrPos = ctx.fstrPos;
		stringType = ctx.stringType;
		fstrType = ctx.fstrType;
		fstr = ctx.fstr;
		argId = ctx.argId;
	}

	slice* InsertSlice(slice* prev, slice sl)
	{
		slice* s = (slice*)m_SliceMemory.Alloc(sizeof(slice));
		*s = sl;

		if(!prev) {
			s->next = m_FirstSlice;
			m_FirstSlice = s;
		} else {
			if(prev == m_LastSlice) {
				m_LastSlice->next = s;
				m_LastSlice = s;
			} else {
				slice* tmp = prev->next;
				prev->next = s;
				s->next = tmp;
			}
		}

		return s;
	}

private:
	slice* AddSlice()
	{
		slice* s = (slice*)m_SliceMemory.Alloc(sizeof(slice));

		if(m_LastSlice)
			m_LastSlice->next = s;
		else
			m_FirstSlice = s;
		m_LastSlice = s;
		m_LastSlice->next = nullptr;
		return m_LastSlice;
	}

private:
	// Memory information
	slice* m_FirstSlice;
	slice* m_LastSlice;
	bool m_ForceSlice;
	internal::FormatMemory m_Memory;
	internal::FormatMemory m_SliceMemory;

	// Output information
	size_t m_Line;
	size_t m_Collumn;
	size_t m_CharacterCount;

	mutable size_t m_SinkCollumn; // Cache for collumn returned by sink.

	const locale::Locale* m_Locale;
};

}
