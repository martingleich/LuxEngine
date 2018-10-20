#ifndef INCLUDED_LUX_INIFILE_H
#define INCLUDED_LUX_INIFILE_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "core/lxArray.h"
#include "io/Path.h"
#include "io/ioConstants.h"

namespace lux
{
namespace io
{
class File;

class INIFile : public ReferenceCounted
{
public:
	static const int InvalidID = -1;

	enum class ESorting
	{
		Ascending,
		Descending
	};

	class Element
	{
	public:
		Element() :
			m_File(nullptr)
		{
		}
		Element(INIFile* file, int section, int element) :
			m_File(file),
			m_Section(section),
			m_Element(element)
		{
			if(section < 0 || element < 0)
				m_File = nullptr;
		}
		const core::String& Name() const
		{
			return m_File->GetElementName(m_Section, m_Element);
		}
		const core::String& Comment() const
		{
			return m_File->GetElementComment(m_Section, m_Element);
		}
		const core::String& Value() const
		{
			return m_File->GetElementValue(m_Section, m_Element);
		}
		void SetName(core::StringView name)
		{
			return m_File->SetElementName(m_Section, m_Element, name);
		}
		void SetComment(core::StringView comment)
		{
			return m_File->SetElementComment(m_Section, m_Element, comment);
		}
		void SetValue(core::StringView value) const
		{
			m_File->SetElementValue(m_Section, m_Element, value);
		}
		
		int GetSectionId() const { return m_Section; }
		int GetElementId() const { return m_Element; }

		bool Next()
		{
			++m_Element;
			if(m_Element == m_File->GetElementCount(m_Section))
				m_File = nullptr;
			return m_File != nullptr;
		}
		
		bool IsValid() const { return m_File != nullptr; }

	private:
		INIFile* m_File;
		int m_Section;
		int m_Element;
	};

	class Section
	{
	public:
		Section() :
			m_File(nullptr),
			m_ID(InvalidID)
		{
		}
		Section(INIFile* file, int ID) :
			m_File(file),
			m_ID(ID)
		{
			if(ID < 0)
				m_File = nullptr;
		}

		const core::String& Name() const
		{
			return m_File->GetSectionName(m_ID);
		}
		const core::String& Comment() const
		{
			return m_File->GetSectionComment(m_ID);
		}
		void SetName(core::StringView name)
		{
			return m_File->SetSectionName(m_ID, name);
		}
		void SetComment(core::StringView comment)
		{
			return m_File->SetSectionComment(m_ID, comment);
		}

		int GetSectionId() const { return m_ID; }
		bool Next()
		{
			++m_ID;
			if(m_ID == m_File->GetSectionCount())
				m_File = nullptr;
			return m_File != nullptr;
		}

		Element GetElement(core::StringView str)
		{
			int elemID = m_File->GetElemID(m_ID, str);
			if(elemID < 0)
				return Element();
			else
				return Element(m_File, m_ID, elemID);
		}
		Element GetElementByID(int i)
		{
			int count = m_File->GetElementCount(m_ID);
			if(i >= count)
				return Element();
			else
				return Element(m_File, m_ID, i);
		}

		Element GetFirstElement()
		{
			return GetElementByID(0);
		}

		bool IsValid() const { return m_File != nullptr; }

	private:
		INIFile* m_File;
		int m_ID;
	};

private:
	struct SINIElement
	{
		core::String name;
		core::String comment;
		core::String value;
		int section;
	};

	struct SINISection
	{
		core::String name;
		core::String comment;
		int firstElem;
		int elemCount;
		bool sorted : 1;
		ESorting sorting;
	};

public:
	LUX_API INIFile(File* f);
	LUX_API INIFile(const io::Path& p);
	LUX_API ~INIFile();

	//! Reload the data from the file
	LUX_API void Reload();

	//! Commit all changes to the file.
	LUX_API bool Commit();

	LUX_API int GetSectionCount() const;
	LUX_API void SortSections(ESorting sorting, bool recursive = false);
	LUX_API Section AddSection(core::StringView name, core::StringView comment = core::StringView::EMPTY);
	LUX_API void RemoveSection(int sectionID);
	LUX_API void SetSectionName(int sectionID, core::StringView name);
	LUX_API void SetSectionComment(int sectionID, core::StringView comment);
	LUX_API const core::String& GetSectionName(int id) const;
	LUX_API const core::String& GetSectionComment(int id) const;
	LUX_API Section GetFirstSection();
	LUX_API Section GetSection(core::StringView section);
	LUX_API int GetSectionID(core::StringView name) const;

	LUX_API int GetElementCount(int sectionID) const;
	LUX_API void SortElements(int sectionID, ESorting sorting);
	LUX_API Element AddElement(int sectionID, core::StringView name, core::StringView value, core::StringView comment = core::StringView::EMPTY);
	LUX_API void RemoveElement(int section, int element);
	LUX_API void SetElementName(int section, int element, core::StringView name);
	LUX_API void SetElementComment(int section, int element, core::StringView comment);
	LUX_API void SetElementValue(int section, int element, core::StringView value);
	LUX_API const core::String& GetElementName(int section, int id) const;
	LUX_API const core::String& GetElementComment(int section, int element) const;
	LUX_API const core::String& GetElementValue(int section, int element) const;
	LUX_API Element GetElement(core::StringView section, core::StringView element);
	LUX_API int GetElemID(int sectionId, core::StringView element) const;
	LUX_API int GetElemID(core::StringView section, core::StringView element, int& outSection) const;

	LUX_API bool IsEmpty() const;

	LUX_API char GetCommentChar() const;

private:
	bool LoadData();
	bool ParseSectionName(const core::String& work, core::String& out);
	bool ReadSections();
	bool ReadElement(const core::String& work, SINIElement& element);

	bool IsComment(const core::String& work, int& CommentBegin);
	void WriteComment(const core::String& comment, int identDepth);

	SINIElement& GetElement(int sectionID, int elementID);
	const SINIElement& GetElement(int sectionID, int elementID) const;

private:
	StrongRef<File> m_File;
	io::Path m_FilePath;

	core::Array<SINISection> m_Sections;
	core::Array<SINIElement> m_Elements;

	// The last accessed section and element.
	mutable int m_CurrentSection;
	mutable int m_CurrentElement;

	core::Array<char> m_CommentChars;

	ELineEnding m_LineEnding;

	bool m_UseFilePath; // Can the file be reopened using m_FilePath
	bool m_SectionSorted;
	bool m_SectionsAscending;
};

} // namespace io
} // namespace lux

#endif // #ifndef INCLUDED_LUX_INIFILE_IMPL_H
