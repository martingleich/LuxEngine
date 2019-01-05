#ifndef INCLUDED_LUX_HARDWAREBUFFERMANAGERNULL_H
#define INCLUDED_LUX_HARDWAREBUFFERMANAGERNULL_H
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

	void AddBuffer(HardwareBuffer* buffer) override;
	void RemoveBuffer(HardwareBuffer* buffer) override;
	void UpdateBuffer(HardwareBuffer* buffer) override;
	void EnableBuffer(const HardwareBuffer* buffer) override;
	StrongRef<IndexBuffer> CreateIndexBuffer() override;
	StrongRef<VertexBuffer> CreateVertexBuffer() override;

protected:
	virtual void* UpdateInternalBuffer(HardwareBuffer* buffer, void* handle) = 0;
	virtual void RemoveInternalBuffer(HardwareBuffer* buffer, void* handle) = 0;
	virtual void EnableHardwareBuffer(const HardwareBuffer* buffer, const void* handle) = 0;

private:
	void ForceBufferUpdate(HardwareBuffer* buffer);

protected:
	VideoDriver* m_Driver;
};

} // namespace video
} // namespace lux

#endif // !INCLUDED_LUX_HARDWAREBUFFERMANAGERNULL_H
