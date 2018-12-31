#include "format/Context.h"
#include "format/FormatMemory.h"
#include "format/UnicodeConversion.h"
#include "format/FormatLocale.h"

namespace format
{

static thread_local FormatMemory g_threadMemory;
static Slice g_EmptySlice(0, "");

Context::OutputStateContext::OutputStateContext(Context& _ctx) :
	memState(_ctx.m_Memory.GetState()),
	ctx(_ctx),
	m_CurSlice(_ctx.m_Memory.LastSlice())
{
	ctx.m_LastSlice = &g_EmptySlice;
}

Slice Context::OutputStateContext::CreateSlice()
{
	SlicesT slices = ctx.Slices();
	slices._begin = m_CurSlice;
	slices._begin++;
	int length = 0;
	for(auto s : slices)
		length += s.size;
	ctx.m_Memory.RestoreState(memState);
	// Unsafe access to old memory.
	auto bytes = ctx.AllocByte(length);
	auto cur = bytes;
	for(auto s : slices) {
		std::memmove(cur, s.data, s.size);
		cur += s.size;
	}
	return Slice(length, bytes);
}

Context::OutputStateContext::~OutputStateContext()
{
}

Context::Context(const Locale& locale) :
	m_Memory(g_threadMemory),
	m_MemoryRestoreState(m_Memory.GetState()),
	m_LastSlice(&g_EmptySlice),
	m_Size(0),
	m_Locale(locale),
	m_HasWorkingSlice(false)
{
}

Context::~Context()
{
	m_Memory.RestoreState(m_MemoryRestoreState);
}

void Context::AddSlice(int size, const char* data, bool forceCopy)
{
	assert(!m_HasWorkingSlice);
	if(size == 0)
		return;

	forceCopy |= m_ForceCopy;

	if((size < sizeof(Slice) || forceCopy) && m_Memory.TryExpand(m_LastSlice->data, size)) {
		// If copying the data of the slice is less memory than a new slice or we must copy the data.
		// And we can expand the last slice.
		// We expand the last slice and copy the new data at it's end.
		auto dst = const_cast<char*>(m_LastSlice->data) + m_LastSlice->size;
		for(int i = 0; i < size; ++i)
			dst[i] = data[i];
		m_LastSlice->size += size;
	} else {
		if(forceCopy) {
			char* newData = AllocByte(size);
			for(int i = 0; i < size; ++i)
				newData[i] = data[i];
			data = newData;
		}
		m_LastSlice = m_Memory.AllocSlice(size, data);
	}

	m_Size += size;
}

Slice Context::AddWorkingSlice(int size)
{
	assert(!m_HasWorkingSlice);
	m_HasWorkingSlice = true;

	if(size == 0)
		return g_EmptySlice;

	if(m_Memory.TryExpand(m_LastSlice->data, size)) {
		Slice out(size, m_LastSlice->data + m_LastSlice->size);
		m_LastSlice->size += size;
		return out;
	} else {
		m_LastSlice = m_Memory.AllocSlice(size, AllocByte(size));
		return *m_LastSlice;
	}
}

void Context::EndWorkingSlice(Slice slice, int usedSize)
{
	assert(m_HasWorkingSlice);
	m_HasWorkingSlice = false;
	m_LastSlice->size -= slice.size - usedSize;
	m_Size += usedSize;
}

FormatEntry* Context::GetFormatEntry(int id)
{
	if(id > m_FormatEntriesCount)
		throw syntax_exception("Not enough arguments", size_t(id));
	return (FormatEntry*)((char*)m_FormatEntries + m_FormatEntryStride * id);
}

char* Context::AllocByte(int len, int align)
{
	return (char*)m_Memory.AllocBytes(len, align);
}

Context::SlicesT Context::Slices() const
{
	return {m_Memory.BeginSlice(), m_Memory.EndSlice()};
}

}
