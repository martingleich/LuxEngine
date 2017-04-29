#ifndef INCLUDED_INIFILE_H
#define INCLUDED_INIFILE_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"

namespace lux
{
namespace io
{
class FileSystem;

class INIFile : public ReferenceCounted
{
public:
	typedef size_t SectionID;
	typedef size_t ElementID;
	static const size_t InvalidID = 0xFFFFFFFF;

	enum class ECommentPos
	{
		Before,
		After
	};

	enum class ESorting
	{
		Ascending,
		Descending
	};

public:
	virtual ~INIFile()
	{
	}

	// Close this ini file, all uncommited changes are lost.
	virtual void Close() = 0;

	// Reload the data from the file
	virtual void Reload() = 0;

	// Commit all changes to the file, automatic called when the file is destroyed.
	virtual bool Commit() = 0;

	virtual const string& GetCommentChars() const = 0;
	virtual void SetCommentChars(const string& chars) = 0;
	virtual u32 GetCommentChar() const = 0;

	virtual size_t GetSectionCount() = 0;
	virtual bool SortSections(ESorting sorting, bool recursive = false) = 0;
	virtual bool AddSection(const char* name, const char* comment = nullptr) = 0;
	virtual bool RemoveSection(const char* section) = 0;
	virtual bool SetSectionName(const char* section, const char* name) = 0;
	virtual bool SetSectionComment(const char* section, const char* comment) = 0;
	virtual const string& GetSectionName(SectionID id) = 0;
	virtual const string& GetSectionComment(const char* section) = 0;

	virtual size_t GetElementCount(const char* section) = 0;
	virtual bool SortElements(const char* section, ESorting sorting) = 0;
	virtual bool AddElement(const char* section, const char* name, const char* value, const char* comment = nullptr) = 0;
	virtual bool RemoveElement(const char* section, const char* element) = 0;
	virtual bool SetElementName(const char* section, const char* element, const char* name) = 0;
	virtual bool SetElementComment(const char* section, const char* element, const char* comment) = 0;
	virtual bool SetElementValue(const char* section, const char* element, const char* value) = 0;
	virtual const string& GetElementName(const char* section, ElementID id) = 0;
	virtual const string& GetElementComment(const char* section, const char* element) = 0;
	virtual const string& GetElementValue(const char* section, const char* element) = 0;
	virtual void SetElementCommentPos(ECommentPos pos) = 0;
	virtual ECommentPos GetElementCommentPos() const = 0;
	virtual bool IsEmpty() = 0;
};

}
}

#endif