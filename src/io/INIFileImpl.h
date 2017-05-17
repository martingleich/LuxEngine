#ifndef INCLUDED_INIFILE_IMPL_H
#define INCLUDED_INIFILE_IMPL_H
#include "io/INIFile.h"

#include "core/lxArray.h"
#include "io/path.h"

namespace lux
{
namespace io
{
class File;
class INIFileImpl : public INIFile
{
public:
	INIFileImpl(FileSystem* FileSys) :
		m_FileSys(FileSys),
		m_File(nullptr),
		m_AutoReload(true),
		m_AutoCommit(true),
		m_CurrentSection(0),
		m_CurrentElement(0),
		m_CommentChars(";#")
	{
	}

	INIFileImpl(FileSystem* FileSys, File* f) :
		INIFileImpl(FileSys)
	{
		Init(f);
	}

	INIFileImpl(FileSystem* FileSys, const io::path& p) :
		INIFileImpl(FileSys)
	{
		Init(p);
	}

	~INIFileImpl()
	{
		Close();
	}

	void Init(File* file);
	void Init(const io::path& path);

	// Close this ini file
	void Close();

	// Reload the data from the file
	void Reload();

	// Commit all changes to the file, automatic called on Close
	bool Commit();

	const string& GetCommentChars() const;
	void SetCommentChars(const string& chars);
	u32 GetCommentChar() const;

	size_t GetSectionCount();
	bool SortSections(ESorting sorting, bool recursive = false);
	bool AddSection(const char* name, const char* comment = nullptr);
	bool RemoveSection(const char* section);
	bool SetSectionName(const char* section, const char* name);
	bool SetSectionComment(const char* section, const char* comment);
	const string& GetSectionName(SectionID id);
	const string& GetSectionComment(const char* section);

	size_t GetElementCount(const char* section);
	bool SortElements(const char* section, ESorting sorting);
	bool AddElement(const char* section, const char* name, const char* value, const char* comment = nullptr);
	bool RemoveElement(const char* section, const char* element);
	bool SetElementName(const char* section, const char* element, const char* name);
	bool SetElementComment(const char* section, const char* element, const char* comment);
	bool SetElementValue(const char* section, const char* element, const char* value);
	const string& GetElementName(const char* section, ElementID id);
	const string& GetElementComment(const char* section, const char* element);
	const string& GetElementValue(const char* section, const char* element);
	void SetElementCommentPos(ECommentPos pos);
	ECommentPos GetElementCommentPos() const;
	bool IsEmpty();

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
};

}
}

#endif // #ifndef INCLUDED_INIFILE_IMPL_H
