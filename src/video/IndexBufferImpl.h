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
	u32 AddIndex(void* index);
	u32 AddIndices(void* indices, u32 count);
	void SetIndex(void* index, u32 n);
	void SetIndices(void* indices, u32 count, u32 n);
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
		assertNeverReach("Unknown index format.");
		return 0;
	}

	void SetHandle(void* handle) { m_Handle = handle; }
	void* GetHandle() const { return m_Handle; }
	BufferManager* GetManager() { return m_Manager; }
	bool UpdateByManager(u32 group) { return m_Manager->UpdateBuffer(this, group); }

private:
	static u32 CalcStride(EIndexFormat type);

private:
	EIndexFormat m_Type;
	void* m_Handle;
	BufferManager* m_Manager;
};

}
}

#endif // #ifndef INCLUDED_INDEXBUFFER_IMPL