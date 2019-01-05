#include "video/VertexBuffer.h"

namespace lux
{
namespace video
{

VertexBuffer::VertexBuffer(BufferManager* mgr) :
	HardwareBuffer(mgr, EHardwareBufferType::Vertex),
	m_Format(VertexFormat::STANDARD)
{
	m_Stride = m_Format.GetStride();
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::SetFormat(const VertexFormat& format, bool moveOld, const void* init)
{
	int stride = format.GetStride();
	if(stride == 0)
		throw core::GenericInvalidArgumentException("format", "Format is invalid");

	if(m_Data) {
		if(init) {
			for(int i = 0; i < m_Size; ++i)
				memcpy(m_Data + i*stride, init, stride);
		} else {
			if(moveOld) {
				auto newData = LUX_NEW_ARRAY(u8, m_Size*stride);
				int elemCount = format.GetElemCount();
				struct CopyData { int newOff, oldOff, size;};
				core::Array<CopyData> copyPos;
				copyPos.Resize(elemCount);
				for(int i = 0; i < elemCount; ++i) {
					auto elem = format.GetElement(i);
					auto oldElem = m_Format.GetElement(elem.GetUsage());
					if(oldElem.IsValid() && oldElem.GetType() == elem.GetType())
						copyPos[i] = CopyData{elem.GetOffset(), oldElem.GetOffset(), elem.GetSize()};
					else
						copyPos[i] = CopyData{-1, -1, -1};
				}

				u8* newPtr = newData;
				u8* oldPtr = m_Data;
				for(int i = 0; i < m_Size; ++i) {
					for(int j = 0; j < elemCount; ++j) {
						if(copyPos[j].oldOff != -1)
							memcpy(newPtr + copyPos[j].newOff, oldPtr + copyPos[j].oldOff, copyPos[j].size);
					}
					newPtr += stride;
					oldPtr += m_Stride;
				}

				LUX_FREE_ARRAY(m_Data);
				m_Data = newData;
			}
		}
	}

	m_Format = format;
	m_Stride = stride;

	m_ChangeId++;
}

}
}

