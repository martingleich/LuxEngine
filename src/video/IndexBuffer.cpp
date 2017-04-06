#include "video/IndexBuffer.h"

namespace lux
{
namespace video
{

u32 IndexBufferImpl::CalcStride(EIndexFormat type)
{
	if(type == EIndexFormat::Bit16)
		return 2;
	if(type == EIndexFormat::Bit32)
		return 4;

	return 0;
}

IndexBufferImpl::IndexBufferImpl(BufferManager* mgr) : IndexBuffer(mgr)
{
	m_Type = EIndexFormat::Bit16;
	m_Stride = CalcStride(m_Type);
}

EIndexFormat IndexBufferImpl::GetType() const
{
	return m_Type;
}

void IndexBufferImpl::SetType(EIndexFormat type, bool moveOld, void* init)
{
	u32 stride = CalcStride(type);

	if(!m_Data) {
		m_Type = type;
		return;
	}

	if(type == m_Type) {
		if(moveOld)
			return;

		if(init)
			for(u32 i = 0; i < m_Size; ++i)
				memcpy(m_Data + i*m_Stride, init, m_Stride);
		return;
	}

	if(m_Type == EIndexFormat::Bit16 && type == EIndexFormat::Bit32) {
		u8* old = m_Data;
		m_Data = LUX_NEW_ARRAY(u8, stride*m_Allocated);

		if(moveOld) {
			for(u32 i = 0; i < m_Size; ++i)
				*((u32*)m_Data + i*stride) = *((u16*)old + i*m_Stride);
		} else {
			if(init) {
				for(u32 i = 0; i < m_Size; ++i)
					memcpy(m_Data + i*stride, init, stride);
			}
		}

		LUX_FREE_ARRAY(old);
	} else {
		u8* old = m_Data;
		m_Data = LUX_NEW_ARRAY(u8, stride*m_Allocated);

		if(moveOld) {
			for(u32 i = 0; i < m_Size; ++i)
				*((u16*)m_Data + i*stride) = (u16)*((u32*)old + i*m_Stride);
		} else {
			if(init) {
				for(u32 i = 0; i < m_Size; ++i)
					memcpy(m_Data + i*stride, init, stride);
			}
		}
		
		LUX_FREE_ARRAY(old);
	}

	m_Type = type;
	m_Stride = stride;
}

u32 IndexBufferImpl::AddIndex(void* index)
{
	return AddIndices(index, 1);
}
u32 IndexBufferImpl::AddIndices(void* indices, u32 count)
{
	if(m_Cursor + count - 1 >= m_Size) {
		Reserve(m_Cursor + count);
		m_Size += count;
	}

	SetIndices(indices, count, m_Cursor);
	u32 ret = m_Cursor;
	m_Cursor += count;
	return ret;
}
void IndexBufferImpl::SetIndex(void* index, u32 n)
{
	SetIndices(index, 1, n);
}
void IndexBufferImpl::SetIndices(void* indices, u32 count, u32 n)
{
	memcpy(Pointer(n, count), indices, count*m_Stride);
}
void IndexBufferImpl::GetIndex(void* ptr, u32 n) const
{
	GetIndices(ptr, 1, n);
}
void IndexBufferImpl::GetIndices(void* ptr, u32 count, u32 n) const
{
	memcpy(ptr, Pointer(n, count), count*m_Stride);
}

}    // namespace scene
}    // namespace lux
