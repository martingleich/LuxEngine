#ifndef INCLUDED_HARDWAREBUFFERMANAGERNULL_H
#define INCLUDED_HARDWAREBUFFERMANAGERNULL_H
#include "video/HardwareBufferManager.h"
#include "core/lxArray.h"

namespace lux
{
namespace video
{

class BufferManagerNull : public BufferManager
{
public:
	BufferManagerNull(VideoDriver* driver);

	VideoDriver* GetDriver();
	void AddBuffer(HardwareBuffer* buffer);
	void RemoveBuffer(HardwareBuffer* buffer);
	void UpdateBuffer(HardwareBuffer* buffer);
	void ForceBufferUpdate(HardwareBuffer* buffer);
	StrongRef<IndexBuffer> CreateIndexBuffer();
	StrongRef<VertexBuffer> CreateVertexBuffer();
	void EnableBuffer(const HardwareBuffer* buffer, int streamID);

	virtual void* UpdateInternalBuffer(HardwareBuffer* buffer, void* handle) = 0;
	virtual void RemoveInternalBuffer(HardwareBuffer* buffer, void* handle) = 0;
	virtual void EnableHardwareBuffer(
		int streamID,
		const HardwareBuffer* buffer,
		const void* handle) = 0;

protected:
	VideoDriver* m_Driver;
};

} // namespace video
} // namespace lux

#endif // !INCLUDED_HARDWAREBUFFERMANAGERNULL_H
