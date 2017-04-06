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
	u32 m_MinHardwareBufferBytes;

protected:
	VideoDriver* m_Driver;

public:
	BufferManagerNull(VideoDriver* driver);

	void SetMinHardwareBufferBytes(u32 bytes);

	u32 GetMinHardwareBufferBytes() const;

	VideoDriver* GetDriver();
	void Update(u32 updateGroup);

	virtual void AddBuffer(HardwareBuffer* buffer);

	virtual void RemoveBuffer(HardwareBuffer* buffer);

	bool UpdateBuffer(HardwareBuffer* buffer, u32 group = 0);

	bool ShouldCreateHardwareBuffer(HardwareBuffer* buffer);
	bool ForceBufferUpdate(HardwareBuffer* buffer);

	IndexBuffer* CreateIndexBuffer();
	VertexBuffer* CreateVertexBuffer();

	bool EnableBuffer(const HardwareBuffer* buffer, u32 streamID);

	virtual void* UpdateInternalBuffer(HardwareBuffer* buffer, void* handle) = 0;
	virtual void RemoveInternalBuffer(HardwareBuffer* buffer, void* handle) = 0;
	virtual bool EnableHardwareBuffer(u32 streamID, const HardwareBuffer* buffer, const void* handle) = 0;
};

}
}

#endif // !INCLUDED_HARDWAREBUFFERMANAGERNULL_H
