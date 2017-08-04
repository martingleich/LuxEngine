#include "format/Context.h"
#include "format/Sink.h"
#include <limits>

namespace format
{
Context::Context() :
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

Slice* Context::AddSlice(size_t size, const char* data, bool forceCopy, const Cursor* curDiff)
{
	Slice* out;
	if(!m_ForceSlice && !forceCopy && m_LastSlice && m_LastSlice->data + m_LastSlice->size == data) {
		out = m_LastSlice;
		out->size += size;
	} else if(!m_ForceSlice && m_LastSlice && (size < sizeof(Slice) || forceCopy) && m_Memory.TryExpand(m_LastSlice->data, size)) {
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

Context::SubContext Context::SaveSubContext() const
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

void Context::RestoreSubContext(const SubContext& ctx)
{
	fstrLastArgPos = ctx.fstrLastArgPos;
	fstrPos = ctx.fstrPos;
	stringType = ctx.stringType;
	fstrType = ctx.fstrType;
	fstr = ctx.fstr;
	argId = ctx.argId;
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

size_t Context::GetCollumn() const
{
	if(m_Line == 0) {
		if(m_SinkCollumn == std::numeric_limits<size_t>::max())
			m_SinkCollumn = dstSink->GetCollumn();
		return m_SinkCollumn + m_Collumn;
	} else {
		return m_Collumn;
	}
}

}
