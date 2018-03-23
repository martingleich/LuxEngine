#ifndef INCLUDED_LUX_HARDWAREBUFFERMANAGER_D3D9_H
#define INCLUDED_LUX_HARDWAREBUFFERMANAGER_D3D9_H
#include "video/HardwareBufferManagerNull.h"

#ifdef LUX_COMPILE_WITH_D3D9
#include "platform/StrippedD3D9.h"
#include "platform/UnknownRefCounted.h"

namespace lux
{
namespace video
{

class BufferManagerD3D9 : public BufferManagerNull
{
public:
	struct VertexStream
	{
		const void* data;
		u32 offset;

		VertexStream() :
			data(nullptr),
			offset(0)
		{
		}
	};

	struct IndexStream
	{
		const void* data;
		u32 offset;

		IndexStream() :
			data(nullptr),
			offset(0)
		{
		}
	};

public:
	BufferManagerD3D9(VideoDriver* driver);
	~BufferManagerD3D9();
	void RemoveInternalBuffer(HardwareBuffer* buffer, void* handle);
	void* UpdateVertexBuffer(VertexBuffer* buffer, void* handle);
	void* UpdateIndexBuffer(IndexBuffer* buffer, void* handle);
	void* UpdateInternalBuffer(HardwareBuffer* buffer, void* handle);
	void EnableHardwareBuffer(int streamID, const HardwareBuffer* buffer, const void* handle);
	bool GetVertexStream(int streamID, VertexStream& vs) const;
	bool GetIndexStream(IndexStream& is) const;
	void ResetStreams();

	void ReleaseHardwareBuffers();
	void RestoreHardwareBuffers();

private:
	UnknownRefCounted<IDirect3DDevice9> m_D3DDevice;

	core::Array<VertexStream> m_VStreams;
	u32 m_UsedStreams;
	IndexStream m_IStream;
	bool m_AllowStreamOffset;
	UINT  m_MaxStreamCount;

	core::Array<HardwareBuffer*> m_HardwareBuffers;
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
#endif // !INCLUDED_LUX_HARDWAREBUFFERMANAGERD3D9_H
