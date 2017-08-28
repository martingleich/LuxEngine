#ifndef INCLUDED_INDEXBUFFER_IMPL
#define INCLUDED_INDEXBUFFER_IMPL
#include "video/IndexBuffer.h"
#include "video/HardwareBufferManager.h"

namespace lux
{
namespace video
{

class IndexBufferImpl : public IndexBuffer
{
public:
	IndexBufferImpl(BufferManager* mgr);
	~IndexBufferImpl();

	EIndexFormat GetType() const;
	void SetType(EIndexFormat type, bool moveOld = true, void* init = nullptr);
	u32 AddIndex(const void* index);
	u32 AddIndices(const void* indices, u32 count);
	u32 AddIndices32(const u32* indices, u32 count);
	void SetIndex(const void* index, u32 n);
	void SetIndices(const void* indices, u32 count, u32 n);
	void SetIndices32(const u32* indices, u32 count, u32 n);
	void GetIndex(void* ptr, u32 n) const;
	void GetIndices(void* ptr, u32 count, u32 n) const;

	u32 GetIndex(u32 n) const
	{
		if(GetType() == EIndexFormat::Bit32) {
			u32 data;
			GetIndex(&data, n);
			return data;
		} else if(GetType() == EIndexFormat::Bit16) {
			u16 data;
			GetIndex(&data, n);
			return data;
		}
		lxAssertNeverReach("Unknown index format.");
		return 0;
	}

	void SetHandle(void* handle) { m_Handle = handle; }
	void* GetHandle() const { return m_Handle; }
	BufferManager* GetManager() { return m_Manager; }
	void UpdateByManager(u32 group) { m_Manager->UpdateBuffer(this, group); }

private:
	static u32 CalcStride(EIndexFormat type);

private:
	BufferManager* m_Manager;
	EIndexFormat m_Type;
	void* m_Handle;
};

}
}

#endif // #ifndef INCLUDED_INDEXBUFFER_IMPL
