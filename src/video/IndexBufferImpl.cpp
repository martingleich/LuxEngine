#include "video/IndexBufferImpl.h"

namespace lux
{
namespace video
{

static int CalcStride(EIndexFormat type)
{
	if(type == EIndexFormat::Bit16)
		return 2;
	if(type == EIndexFormat::Bit32)
		return 4;

	return 0;
}

IndexBufferImpl::IndexBufferImpl(BufferManager* mgr) :
	m_Manager(mgr),
	m_Format(EIndexFormat::Bit16)
{
	m_Stride = CalcStride(m_Format);

	m_Manager->AddBuffer(this);
}

IndexBufferImpl::~IndexBufferImpl()
{
	m_Manager->RemoveBuffer(this);
}

EIndexFormat IndexBufferImpl::GetFormat() const
{
	return m_Format;
}

void IndexBufferImpl::SetFormat(EIndexFormat type, bool moveOld, void* init)
{
	int stride = CalcStride(type);

	if(!m_Data) {
		m_Format = type;
		m_Stride = stride;
		return;
	}

	m_ChangeId++;

	if(type == m_Format) {
		if(moveOld)
			return;

		if(init)
			for(int i = 0; i < m_Size; ++i)
				memcpy(m_Data + i*m_Stride, init, m_Stride);
		return;
	}

	if(m_Format == EIndexFormat::Bit16 && type == EIndexFormat::Bit32) {
		u8* old = m_Data;
		m_Data = LUX_NEW_ARRAY(u8, stride*m_Allocated);

		if(moveOld) {
			for(int i = 0; i < m_Size; ++i)
				*((int*)m_Data + i*stride) = *((u16*)old + i*m_Stride);
		} else {
			if(init) {
				for(int i = 0; i < m_Size; ++i)
					memcpy(m_Data + i*stride, init, stride);
			}
		}

		LUX_FREE_ARRAY(old);
	} else {
		u8* old = m_Data;
		m_Data = LUX_NEW_ARRAY(u8, stride*m_Allocated);

		if(moveOld) {
			for(int i = 0; i < m_Size; ++i)
				*((u16*)m_Data + i*stride) = (u16)*((int*)old + i*m_Stride);
		} else {
			if(init) {
				for(int i = 0; i < m_Size; ++i)
					memcpy(m_Data + i*stride, init, stride);
			}
		}

		LUX_FREE_ARRAY(old);
	}

	m_Format = type;
	m_Stride = stride;
}

int IndexBufferImpl::AddIndex(const void* index)
{
	return AddIndices(index, 1);
}
int IndexBufferImpl::AddIndices(const void* indices, int count)
{
	if(m_Cursor + count - 1 >= m_Size) {
		Reserve(m_Cursor + count);
		m_Size += count;
	}

	SetIndices(indices, count, m_Cursor);
	int ret = m_Cursor;
	m_Cursor += count;
	return ret;
}
int IndexBufferImpl::AddIndices32(const u32* indices, int count)
{
	if(m_Cursor + count - 1 >= m_Size) {
		Reserve(m_Cursor + count);
		m_Size += count;
	}

	SetIndices32(indices, count, m_Cursor);
	int ret = m_Cursor;
	m_Cursor += count;
	return ret;
}

void IndexBufferImpl::SetIndex(const void* index, int n)
{
	SetIndices(index, 1, n);
}
void IndexBufferImpl::SetIndices(const void* indices, int count, int n)
{
	memcpy(Pointer(n, count), indices, count*m_Stride);
}
void IndexBufferImpl::SetIndices32(const u32* indices, int count, int n)
{
	if(m_Format == EIndexFormat::Bit16) {
		u16* ptr = (u16*)Pointer(n, count);
		for(int i = 0; i < count; ++i)
			*ptr++ = (u16)*indices++;
	} else if(m_Format == EIndexFormat::Bit32) {
		memcpy(Pointer(n, count), indices, count*m_Stride);
	} else {
		lxAssertNeverReach("Unknown index format.");
	}
}

void IndexBufferImpl::GetIndex(void* ptr, int n) const
{
	GetIndices(ptr, 1, n);
}
void IndexBufferImpl::GetIndices(void* ptr, int count, int n) const
{
	memcpy(ptr, Pointer(n, count), count*m_Stride);
}

}
}
