#ifndef INCLUDED_PATH_H
#define INCLUDED_PATH_H

#include "core/lxString.h"
#include "core/DateAndTime.h"

namespace lux
{
namespace io
{

typedef string path;

LUX_API path GetFileExtension(const path& p);
LUX_API path GetFileDir(const path& p);
LUX_API path GetFilenameOnly(const path& p, bool keepExtension = true);

//! Normalizes a path
/**
A normalized path contains only / seperators.
A normllaized directory path end with and / character
\param p The path to normalize
\param isDirectory Is the path a directory
\return The normalized path
*/
LUX_API path NormalizePath(const path& p, bool isDirectory = false);

//! Creates a new absolute path, concating a base path and a path relative to the base.
/*
REMARK:
The returned path is normalized.

\param base The base path, must be an absolute path.
\param rel The path relative to base, can contain /../ segements
\return The absolute path by concating base and rel
*/
LUX_API path MakeAbsolutePath(const path& base, const path& rel);

class FileSystem;
class Archive;

class FileDescription
{
public:
	enum class EType
	{
		File,
		Directory,
		Other
	};

public:
	FileDescription() :
		m_Archive(nullptr)
	{
	}

	FileDescription(const path& path,
		const string& name,
		u32 size,
		EType type,
		const core::DateAndTime& creationDate,
		bool isVirtual) :
		m_Path(path),
		m_Name(name),
		m_Archive(nullptr),
		m_Size(size),
		m_Type(type),
		m_Creation(creationDate),
		m_IsVirtual(isVirtual)
	{
	}

	bool operator==(const FileDescription& other) const
	{
		return (m_Archive == other.m_Archive && m_Path == other.m_Path && m_Name == other.m_Name);
	}

	bool operator!=(const FileDescription& other) const
	{
		return !(*this == other);
	}

	void SetPath(const path& p)
	{
		m_Path = NormalizePath(p, true);
	}

	void SetName(const string& n)
	{
		m_Name = n;
	}

	const path& GetPath() const
	{
		return m_Path;
	}

	const string& GetName() const
	{
		return m_Name;
	}

	Archive* GetArchive() const
	{
		return m_Archive;
	}

	void SetArchive(Archive* a)
	{
		m_Archive = a;
	}

	void SetSize(u32 size)
	{
		m_Size = size;
	}

	u32 GetSize() const
	{
		return m_Size;
	}

	void SetType(EType type)
	{
		m_Type = type;
	}

	EType GetType() const
	{
		return m_Type;
	}

	void SetCreationDate(const core::DateAndTime& date)
	{
		m_Creation = date;
	}

	const core::DateAndTime& GetCreationDate() const
	{
		return m_Creation;
	}

	void SetIsVirtual(bool v)
	{
		m_IsVirtual = v;
	}

	bool GetIsVirtual() const
	{
		return m_IsVirtual;
	}

private:
	path m_Path;
	string m_Name;
	Archive* m_Archive;

	u32 m_Size;
	EType m_Type;
	core::DateAndTime m_Creation;
	bool m_IsVirtual;
};

inline FileDescription ConcatFileDesc(const FileDescription& base, const io::path& relative)
{
	path absPath = MakeAbsolutePath(base.GetPath(), GetFileDir(relative));
	string fileName = GetFilenameOnly(relative);

	FileDescription out(absPath,
		fileName,
		0,
		FileDescription::EType::Other,
		core::DateAndTime(),
		false);

	return out;
}

} // io
} // lux

#endif
