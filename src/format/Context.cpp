#include "format/Context.h"
#include "format/StringBasics.h"
#include "format/FormatLocale.h"

namespace format
{
Context::Context(
	const Locale* locale,
	size_t startCollumn,
	size_t startLine) :
	fstrLastArgPos(0),
	argId(0),
	m_FmtString(nullptr),
	m_LastSlice(nullptr),
	m_Line(startLine),
	m_Collumn(startCollumn),
	m_CharacterCount(0),
	m_Counting(0),
	m_Size(0),
	m_CursorSlice(m_SliceMemory.Last()),
	m_Locale(locale ? locale : format::GetLocale()),
	m_ForceSlice(true)
{
}

void Context::Reset(
	const Locale* locale,
	size_t startCollumn,
	size_t startLine)
{
	fstrLastArgPos = 0;
	argId = 0;

	m_Memory.Clear();
	m_SliceMemory.Clear();
	m_LastSlice = nullptr;

	m_FmtString = nullptr;

	m_Line = startLine;
	m_Collumn = startCollumn;
	m_CharacterCount = 0;
	m_Counting = 0;
	m_Size = 0;

	m_CursorSlice = m_SliceMemory.Last();
	m_CursorPos = 0;

	if(locale)
		m_Locale = locale;

	m_ForceSlice = false;
}

void Context::AddSlice(size_t size, const char* data, bool forceCopy)
{
	if(size == 0)
		return;

	if(!m_ForceSlice && m_LastSlice && (size < sizeof(Slice) || forceCopy) && m_Memory.TryExpand(m_LastSlice->data, size)) {
		// If copying the data of the slice is less memory than a new slice or we must copy the data.
		// And we can expand the last slice.
		// We expand the last slice and copy the new data at it's end.
		memcpy(const_cast<char*>(m_LastSlice->data) + m_LastSlice->size, data, size);
		m_LastSlice->size += size;
	} else {
		m_LastSlice = m_SliceMemory.Alloc();
		if(forceCopy) {
			char* newData = AllocByte(size);
			memcpy(newData, data, size);
			data = newData;
		}
		m_LastSlice->size = size;
		m_LastSlice->data = data;
		m_ForceSlice = false;
	}

	m_Size += size;

	// Update cursor.
	if(m_Counting)
		m_CharacterCount += StringLength(data, data + size);
}

Slice* Context::AddLockedSlice()
{
	m_ForceSlice = true;
	Slice* out = m_SliceMemory.Alloc();
	out->data = nullptr;
	out->size = 0;
	return out;
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

void Context::EnsureCursor() const
{
	auto last = m_SliceMemory.Last();
	if(m_CursorSlice == last && m_CursorPos == last->size)
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
		if(m_CursorSlice == last)
			break;
		++m_CursorSlice;
		m_CursorPos = 0;
	}
}

}
