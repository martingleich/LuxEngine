#include "format/Context.h"
#include "format/Sink.h"
#include <limits>

#include <iostream>

namespace format
{
Context::Context(
	const Locale* locale,
	size_t startCollumn,
	size_t startLine) :
	fstrLastArgPos(0),
	argId(0),
	m_FmtString(nullptr),
	m_FirstSlice(nullptr),
	m_LastSlice(nullptr),
	m_CursorSlice(nullptr),
	m_Line(startLine),
	m_Collumn(startCollumn),
	m_CharacterCount(0),
	m_Counting(0),
	m_Size(0),
	m_Locale(locale),
	m_ForceSlice(true)
{
	if(!m_Locale)
		m_Locale = format::GetLocale();
}

void Context::AddSlice(size_t size, const char* data, bool forceCopy)
{
	if(size == 0)
		return;

	Slice* out;
	if(!m_ForceSlice && !forceCopy && m_LastSlice && m_LastSlice->data + m_LastSlice->size == data) {
		// Is the new slice just more data at the end of the new slice
		out = m_LastSlice;
		out->size += size;
	} else if(!m_ForceSlice && m_LastSlice && (size < sizeof(Slice) || forceCopy) && m_Memory.TryExpand(m_LastSlice->data, size)) {
		// If copying the data of the slice is less memory than a new slice or we must copy the data.
		// And we can expand the last slice.
		// We expand the last slice and copy the new data at it's end.
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

	// Update cursor.
	if(m_Counting) {
		const char* ptr = data;
		const char* end = ptr + size;
		while(ptr <= end) {
			AdvanceCursor(ptr);
			m_CharacterCount++;
		}
	}

	m_Size += size;
}

Context::SubContext Context::SaveSubContext(const char* fmtString)
{
	SubContext out;
	out.fstrLastArgPos = fstrLastArgPos;
	out.fstr = m_FmtString;
	out.argId = argId;

	m_FmtString = fmtString;
	fstrLastArgPos = 0;
	argId = 0;

	return out;
}

void Context::RestoreSubContext(const SubContext& ctx)
{
	fstrLastArgPos = ctx.fstrLastArgPos;
	m_FmtString = ctx.fstr;
	argId = ctx.argId;
}

size_t Context::GetLine() const
{
	EnsureCursor();
	return m_Line;
}

size_t Context::GetCollumn() const
{
	EnsureCursor();
	return m_Line;
}

Slice* Context::InsertSlice(Slice* prev, Slice sl)
{
	Slice* s = (Slice*)m_SliceMemory.Alloc(sizeof(Slice));
	*s = sl;

	if(!prev) {
		s->next = m_FirstSlice;
		m_FirstSlice = s;
	} else {
		if(prev == m_LastSlice) {
			m_LastSlice->next = s;
			m_LastSlice = s;
		} else {
			Slice* tmp = prev->next;
			prev->next = s;
			s->next = tmp;
		}
	}

	m_Size = sl.size;

	return s;
}

Slice* Context::AddSlice()
{
	Slice* s = (Slice*)m_SliceMemory.Alloc(sizeof(Slice));

	if(m_LastSlice)
		m_LastSlice->next = s;
	else
		m_FirstSlice = s;
	m_LastSlice = s;
	m_LastSlice->next = nullptr;
	return m_LastSlice;
}

void Context::EnsureCursor() const
{
	if(m_CursorSlice == m_LastSlice && m_CursorPos == m_LastSlice->size)
		return;

	while(true) {
		const char* cur = m_CursorSlice->data + m_CursorPos;
		for(size_t i = m_CursorPos; i < m_CursorSlice->size; ++i) {
			uint32_t c = AdvanceCursor(cur);
			if(c == '\n') {
				m_Collumn = 0;
				++m_Line;
			} else {
				++m_Collumn;
			}
		}
		if(!m_CursorSlice->next)
			break;
		m_CursorSlice = m_CursorSlice->next;
		m_CursorPos = 0;
	}
}

}
