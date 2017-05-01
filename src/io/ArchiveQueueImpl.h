#ifndef INCLUDED_ARCHIVE_QUEUE_IMPL_H
#define INCLUDED_ARCHIVE_QUEUE_IMPL_H
#include "io/ArchiveQueue.h"
#include "core/lxArray.h"

namespace lux
{
namespace io
{

class ArchiveQueueImpl : public ArchiveQueue
{
public:
	StrongRef<File> OpenFile(const path& p, EFileMode mode = EFileMode::Read, bool createIfNotExist = false)
	{
		StrongRef<File> out;
		for(auto it = m_Queue.Last(); it != m_Queue.Begin(); --it) {
			out = (*it)->OpenFile(p, mode, createIfNotExist);
			if(out)
				break;
		}

		return out;
	}

	StrongRef<File> OpenFile(const FileDescription& file, EFileMode mode = EFileMode::Read, bool createIfNotExist = false)
	{
		StrongRef<File> out;
		for(auto it = m_Queue.Last(); it != m_Queue.Begin(); --it) {
			out = (*it)->OpenFile(file, mode, createIfNotExist);
			if(out)
				break;
		}

		return out;

	}

	bool ExistFile(const path& p)
	{
		bool out = false;
		for(auto it = m_Queue.Last(); it != m_Queue.Begin(); --it) {
			out = (*it)->ExistFile(p);
			if(out)
				break;
		}

		return out;
	}

	StrongRef<FileEnumerator> EnumerateFiles(const path& subDir = wstring::EMPTY)
	{
		return nullptr;
	}

	EArchiveCapabilities GetCaps() const
	{
		return m_Caps;
	}

	bool IsValid() const
	{
		return true;
	}

	void AddArchive(Archive* a)
	{
		auto it = FindArchive(a);
		if(it != m_Queue.End())
			m_Queue.Erase(it, true);

		m_Queue.PushBack(a);

		UpdateCaps();
	}

	void RemoveArchive(Archive* a)
	{
		auto it = FindArchive(a);
		if(it != m_Queue.End())
			m_Queue.Erase(it, true);

		UpdateCaps();
	}

	u32 GetArchiveCount() const
	{
		return m_Queue.Size();
	}

	Archive* GetArchive(u32 i)
	{
		if(i < m_Queue.Size())
			return m_Queue[i];
		else
			return nullptr;
	}

	const Archive* GetArchive(u32 i) const
	{
		if(i < m_Queue.Size())
			return m_Queue[i];
		else
			return nullptr;
	}

private:
	core::array<StrongRef<Archive>>::Iterator FindArchive(Archive* a)
	{
		for(auto it = m_Queue.First(); it != m_Queue.End(); ++it) {
			if(*it == a)
				return it;
		}

		return m_Queue.End();
	}

	void UpdateCaps()
	{
		EArchiveCapabilities out = (EArchiveCapabilities)0;

		for(auto it = m_Queue.First(); it != m_Queue.Last(); ++it)
			SetFlag(out, (*it)->GetCaps());

		m_Caps = out;
	}

private:
	core::array<StrongRef<Archive>> m_Queue;
	EArchiveCapabilities m_Caps;
};

}
}


#endif // #ifndef INCLUDED_ARCHIVE_QUEUE_IMPL_H