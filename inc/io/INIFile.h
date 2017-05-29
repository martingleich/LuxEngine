#ifndef INCLUDED_INIFILE_H
#define INCLUDED_INIFILE_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "core/lxArray.h"
#include "io/path.h"

namespace lux
{
namespace io
{
class File;

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
	LUX_API INIFile(FileSystem* FileSys);
	LUX_API INIFile(FileSystem* FileSys, File* f);
	LUX_API INIFile(FileSystem* FileSys, const io::path& p);
	LUX_API ~INIFile();

	// Close this ini file
	LUX_API void Close(); // Reload the data from the file
	LUX_API void Reload();

	// Commit all changes to the file, automatic called on Close
	LUX_API bool Commit();

	LUX_API const string& GetCommentChars() const;
	LUX_API void SetCommentChars(const string& chars);
	LUX_API u32 GetCommentChar() const;

	LUX_API size_t GetSectionCount();
	LUX_API bool SortSections(ESorting sorting, bool recursive = false);
	LUX_API bool AddSection(const char* name, const char* comment = nullptr);
	LUX_API bool RemoveSection(const char* section);
	LUX_API bool SetSectionName(const char* section, const char* name);
	LUX_API bool SetSectionComment(const char* section, const char* comment);
	LUX_API const string& GetSectionName(SectionID id);
	LUX_API const string& GetSectionComment(const char* section);

	LUX_API size_t GetElementCount(const char* section);
	LUX_API bool SortElements(const char* section, ESorting sorting);
	LUX_API bool AddElement(const char* section, const char* name, const char* value, const char* comment = nullptr);
	LUX_API bool RemoveElement(const char* section, const char* element);
	LUX_API bool SetElementName(const char* section, const char* element, const char* name);
	LUX_API bool SetElementComment(const char* section, const char* element, const char* comment);
	LUX_API bool SetElementValue(const char* section, const char* element, const char* value);
	LUX_API const string& GetElementName(const char* section, ElementID id);
	LUX_API const string& GetElementComment(const char* section, const char* element);
	LUX_API const string& GetElementValue(const char* section, const char* element);
	LUX_API void SetElementCommentPos(ECommentPos pos);
	LUX_API ECommentPos GetElementCommentPos() const;
	LUX_API bool IsEmpty();

private:
	struct SINIElement
	{
		string name;
		string comment;
		string value;
		SectionID section;
	};

	struct SINISection
	{
		string name;
		string comment;
		ElementID firstElem;
		size_t elemCount;
		bool sorted : 1;
		ESorting sorting;
	};

private:
	StrongRef<FileSystem> m_FileSys;

	StrongRef<File> m_File;

	io::path m_FilePath;

	bool m_AutoReload;  // Check for reload on every Data access
	bool m_AutoCommit;  // Commit file to disc after each change to data
	bool m_UseFilePath; // Can the file be reopened using m_FilePath

	core::array<SINISection> m_Sections;
	core::array<SINIElement> m_Elements;

	SectionID m_CurrentSection;
	ElementID m_CurrentElement;

	string m_LastComment;
	string m_Work;
	string m_CommentChars;

	bool m_IsEOF;
	bool m_SectionSorted;
	bool m_SectionsAscending;

	ECommentPos m_ElementCommentPos;

private:
	bool LoadData();
	bool ReadLine(string& out);
	bool ParseSectionName(string& work, string& out);
	bool ReadSections();
	bool ReadElement(string& work, SINIElement& element);
	bool IsComment(const string& work, string::ConstIterator& CommentBegin);

	void WriteComment(const string& comment, size_t identDepth, ECommentPos pos);

	SectionID GetSectionID(const char* Section);
	ElementID GetElemID(const char* Section, const char* Element, SectionID& outSection);
	ElementID GetElemID(SectionID SectionID, const char* Element);

	void InitValues();
	
	void Init(File* file);
	void Init(const io::path& path);
};

} // namespace io
} // namespace lux

#endif // #ifndef INCLUDED_INIFILE_IMPL_H
