#ifndef INCLUDED_LUX_INIFILE_H
#define INCLUDED_LUX_INIFILE_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "core/lxArray.h"
#include "io/Path.h"

namespace lux
{
namespace io
{
class File;

class INIFile : public ReferenceCounted
{
public:
	typedef int SectionID;
	typedef int ElementID;
	static const int InvalidID = -1;

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
	LUX_API INIFile(FileSystem* FileSys);
	LUX_API INIFile(FileSystem* FileSys, File* f);
	LUX_API INIFile(FileSystem* FileSys, const io::Path& p);
	LUX_API ~INIFile();

	// Close this ini file
	LUX_API void Close(); // Reload the data from the file
	LUX_API void Reload();

	// Commit all changes to the file, automatic called on Close
	LUX_API bool Commit();

	LUX_API const core::String& GetCommentChars() const;
	LUX_API void SetCommentChars(const core::String& chars);
	LUX_API u32 GetCommentChar() const;

	LUX_API int GetSectionCount();
	LUX_API bool SortSections(ESorting sorting, bool recursive = false);
	LUX_API bool AddSection(const char* name, const char* comment = nullptr);
	LUX_API bool RemoveSection(const char* section);
	LUX_API bool SetSectionName(const char* section, const char* name);
	LUX_API bool SetSectionComment(const char* section, const char* comment);
	LUX_API const core::String& GetSectionName(SectionID id);
	LUX_API const core::String& GetSectionComment(const char* section);

	LUX_API int GetElementCount(const char* section);
	LUX_API bool SortElements(const char* section, ESorting sorting);
	LUX_API bool AddElement(const char* section, const char* name, const char* value, const char* comment = nullptr);
	LUX_API bool RemoveElement(const char* section, const char* element);
	LUX_API bool SetElementName(const char* section, const char* element, const char* name);
	LUX_API bool SetElementComment(const char* section, const char* element, const char* comment);
	LUX_API bool SetElementValue(const char* section, const char* element, const char* value);
	LUX_API const core::String& GetElementName(const char* section, ElementID id);
	LUX_API const core::String& GetElementComment(const char* section, const char* element);
	LUX_API const core::String& GetElementValue(const char* section, const char* element);
	LUX_API void SetElementCommentPos(ECommentPos pos);
	LUX_API ECommentPos GetElementCommentPos() const;
	LUX_API bool IsEmpty();

private:
	struct SINIElement
	{
		core::String name;
		core::String comment;
		core::String value;
		SectionID section;
	};

	struct SINISection
	{
		core::String name;
		core::String comment;
		ElementID firstElem;
		int elemCount;
		bool sorted : 1;
		ESorting sorting;
	};

private:
	StrongRef<FileSystem> m_FileSys;

	StrongRef<File> m_File;

	io::Path m_FilePath;

	bool m_AutoReload;  // Check for reload on every Data access
	bool m_AutoCommit;  // Commit file to disc after each change to data
	bool m_UseFilePath; // Can the file be reopened using m_FilePath

	core::Array<SINISection> m_Sections;
	core::Array<SINIElement> m_Elements;

	SectionID m_CurrentSection;
	ElementID m_CurrentElement;

	core::String m_LastComment;
	core::String m_Work;
	core::String m_CommentChars;

	bool m_IsEOF;
	bool m_SectionSorted;
	bool m_SectionsAscending;

	ECommentPos m_ElementCommentPos;

private:
	bool LoadData();
	void ReadLine(core::String& out);
	bool ParseSectionName(core::String& work, core::String& out);
	bool ReadSections();
	bool ReadElement(core::String& work, SINIElement& element);
	bool IsComment(const core::String& work, core::String::ConstIterator& CommentBegin);

	void WriteComment(const core::String& comment, int identDepth, ECommentPos pos);

	SectionID GetSectionID(const char* Section);
	ElementID GetElemID(const char* Section, const char* Element, SectionID& outSection);
	ElementID GetElemID(SectionID SectionID, const char* Element);

	void InitValues();
	
	void Init(File* file);
	void Init(const io::Path& path);
};

} // namespace io
} // namespace lux

#endif // #ifndef INCLUDED_LUX_INIFILE_IMPL_H
