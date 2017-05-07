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
	void Update(u32 updateGroup);
	virtual void AddBuffer(HardwareBuffer* buffer);
	virtual void RemoveBuffer(HardwareBuffer* buffer);
	void UpdateBuffer(HardwareBuffer* buffer, u32 group = 0);
	void ForceBufferUpdate(HardwareBuffer* buffer);
	StrongRef<IndexBuffer> CreateIndexBuffer();
	StrongRef<VertexBuffer> CreateVertexBuffer();
	void EnableBuffer(const HardwareBuffer* buffer, u32 streamID);
	virtual void* UpdateInternalBuffer(HardwareBuffer* buffer, void* handle) = 0;
	virtual void RemoveInternalBuffer(HardwareBuffer* buffer, void* handle) = 0;
	virtual void EnableHardwareBuffer(u32 streamID, const HardwareBuffer* buffer, const void* handle) = 0;

private:
	struct UpdateEntry
	{
		u32 group;
		HardwareBuffer* buffer;

		UpdateEntry(u32 g, HardwareBuffer* b);
		UpdateEntry(UpdateEntry&& old);
		UpdateEntry();
		UpdateEntry& operator=(const UpdateEntry&& old);
	};

	core::array<UpdateEntry> m_Updates;

protected:
	VideoDriver* m_Driver;
};

}
}

#endif // !INCLUDED_HARDWAREBUFFERMANAGERNULL_H
