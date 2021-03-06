#ifndef INCLUDED_LUX_PATH_H
#define INCLUDED_LUX_PATH_H
#include "core/lxString.h"

namespace lux
{
namespace io
{
class Archive;
//! A Path
/**
Pathes identify objects in the virtual file-system.
They remember the path and the archive.
This class saves the path in a canonic representation, the canoic representation uses slashes to seperate folders and never ends in a slash.
*/
class Path
{
public:
	static LUX_API Path EMPTY;

	Path() :
		m_Archive(nullptr)
	{
	}
	Path(const char* str, Archive* archive = nullptr)
	{
		Set(core::StringView(str, strlen(str)));
		m_Archive = archive;
	}
	Path(const core::String& str, Archive* archive = nullptr)
	{
		Set((core::StringView)(str));
		m_Archive = archive;
	}
	Path(core::StringView str, Archive* archive = nullptr)
	{
		Set(str);
		m_Archive = archive;
	}

	Path(const Path&) = default;
	Path(Path&&) = default;

	Path& operator=(const Path&) = default;
	Path& operator=(Path&&) = default;

	LUX_API static core::String MakeStringCanonic(const core::StringView& view);

	LUX_API void Set(core::StringView str);
	LUX_API Path GetFileDir() const;
	LUX_API core::String GetFileName(bool keepExtension = true) const;
	LUX_API core::String GetFileExtension() const;
	LUX_API bool IsAbsolute() const;

	/**
	\param base The base path, must be an absolute path.
	\param rel The path relative to base, can contain /../ segements
	\return The absolute path by concating base and rel
	*/
	LUX_API Path GetResolved(const Path& base) const;

	core::StringView AsView() const { return m_RawData.AsView(); }
	bool IsEmpty() const { return m_RawData.IsEmpty(); }
	Archive* GetArchive() const { return m_Archive; }

	bool operator==(const Path& other) const { return m_RawData == other.m_RawData; }
	bool operator!=(const Path& other) const { return m_RawData != other.m_RawData; }

	core::String&& TakeString() { return std::move(m_RawData); }
	const core::String& GetString() const { return m_RawData; }
	void PutString(core::String&& str) { m_RawData = std::move(str); }

private:
	core::String m_RawData;
	Archive* m_Archive;
};

struct FileInfo
{
public:
	enum class EType
	{
		File,
		Directory,
		VirtualFile,
		Other
	};

public:
	FileInfo() :
		m_Size(-1),
		m_Type(EType::Other)
	{
	}
	FileInfo(s64 size, EType type) :
		m_Size(size),
		m_Type(type)
	{
	}

	s64 GetSize() const { return m_Size; }
	void SetSize(s64 s) { m_Size = s; }
	EType GetType() const { return m_Type; }
	bool IsVirtual() const { return m_Type == EType::VirtualFile; }
	bool IsFile() const { return m_Type == EType::VirtualFile || m_Type == EType::File; }
	bool IsDirectory() const { return m_Type == EType::Directory; }

private:
	s64 m_Size;
	EType m_Type;
};

void fmtPrint(format::Context& ctx, const Path& p, format::Placeholder&);
} // namespace io
namespace core
{
template <>
struct HashType<io::Path>
{
	unsigned int operator()(const io::Path& path) const
	{
		return HashType<String>()(path.AsView());
	}
};
}

} // namespace lux

#endif
