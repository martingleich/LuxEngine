#ifndef INCLUDED_HARDWAREBUFFERMANAGERD3D9_H
#define INCLUDED_HARDWAREBUFFERMANAGERD3D9_H
#include "HardwareBufferManagerNull.h"
#include "StrippedD3D9.h"

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
		{}
	};

	struct IndexStream
	{
		const void* data;
		u32 offset;

		IndexStream() :
			data(nullptr),
			offset(0)
		{}
	};

private:
	IDirect3DDevice9* m_D3DDevice;

	core::array<VertexStream> m_VStreams;
	u32 m_UsedStreams;
	IndexStream m_IStream;
	bool m_AllowStreamOffset;
	u32  m_MaxStreamCount;

public:
	BufferManagerD3D9(VideoDriver* driver);
	~BufferManagerD3D9();
	void RemoveInternalBuffer(HardwareBuffer* buffer, void* handle);
	void* UpdateVertexBuffer(VertexBuffer* buffer, void* handle);
	void* UpdateIndexBuffer(IndexBuffer* buffer, void* handle);
	void* UpdateInternalBuffer(HardwareBuffer* buffer, void* handle);
	bool EnableHardwareBuffer(u32 streamID, const HardwareBuffer* buffer, const void* handle);
	bool GetVertexStream(u32 streamID, VertexStream& vs) const;
	bool GetIndexStream(IndexStream& is) const;
	void ResetStreams();
};

}
}

#endif // !INCLUDED_HARDWAREBUFFERMANAGERD3D9_H
