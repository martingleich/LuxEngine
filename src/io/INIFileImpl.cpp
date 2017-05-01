#include "INIFileImpl.h"
#include "io/FileSystem.h"

#include "io/File.h"

#include "core/Logger.h"
#include "core/lxAlgorithm.h"
#include "core/lxUnicodeConversion.h"

#include <algorithm>

#ifdef _WIN32
#define NEWLINE "\r\n"
#elif defined macintosh
#define NEWLINE "\r"
#else
#define NEWLINE "\n"
#endif


// TODO: Only use lxAlgorithm

namespace lux
{
namespace io
{

void INIFileImpl::InitValues()
{
	m_SectionSorted = true;
	m_SectionsAscending = true;
	m_CurrentSection = 0;
	m_CurrentElement = 0;
}

bool INIFileImpl::Init(File* File)
{
	m_File = File;
	m_AutoCommit = false;
	m_AutoReload = false;
	InitValues();

	return LoadData();
}

bool INIFileImpl::Init(const io::path& File)
{
	m_FilePath = File;
	m_File = nullptr;
	m_AutoCommit = true;
	m_AutoReload = true;
	InitValues();

	bool result = LoadData();
	LUX_UNUSED(result); // if file couldn't be loaded, handle as new empty file.
	return true;
}

bool INIFileImpl::ReadSections()
{
	SINIElement element;
	SINISection* section = nullptr;
	string sectionName;
	SectionID sectionID = InvalidID;

	bool readSection = true;
	bool useOldLine = false;
	m_Work.Clear();

	while(useOldLine || ReadLine(m_Work)) {
		if(*m_Work.First() == '[') {
			useOldLine = false;

			// Its the name of the section
			if(!ParseSectionName(m_Work, sectionName)) {
				// Invalid section name
				// Goto next section
				log::Debug("Invalid INI-section name: ~s.", m_Work);
				readSection = true;
			} else {
				sectionID = GetSectionID(sectionName.Data());
				if(sectionID != InvalidID) {
					if(!m_LastComment.IsEmpty() && !m_Sections[sectionID].comment.IsEmpty())
						m_Sections[sectionID].comment.Append("\n");
					m_Sections[sectionID].comment.Append(m_LastComment);
					section = &m_Sections[sectionID];
				} else {
					section = &*m_Sections.PushBack(SINISection());
					section->name = sectionName;
					section->sorted = false;
					section->elemCount = 0;
					section->firstElem = InvalidID;
					sectionID = section - m_Sections.Data();
					section->comment = m_LastComment;
				}

				element.section = sectionID;
				m_LastComment.Clear();
				readSection = false;
			}
		} else if(!readSection && !useOldLine) {
			useOldLine = false;
			if(ReadElement(m_Work, element)) {
				//Test if element already exists
				// If yes add new comment to old comment
				// overwrite old value and proceed.
				ElementID elemID = GetElemID(sectionID, element.name.Data());
				if(elemID != InvalidID) {
					if(!m_LastComment.IsEmpty() && !m_Elements[elemID].comment.IsEmpty())
						m_Elements[elemID].comment.Append("\n");
					m_Elements[elemID].comment.Append(m_LastComment);
					m_Elements[elemID].value = element.value;
				} else {
					element.comment = m_LastComment;
					elemID = core::IteratorDistance(m_Elements.First(), m_Elements.PushBack(element));
				}

				m_LastComment.Clear();

				if(section->elemCount == 0)
					section->firstElem = elemID;

				++section->elemCount;
			} else {
				useOldLine = true;
			}
		} else {
			useOldLine = false;
			// Ignore line        
		}
	}

	return !m_IsEOF;
}

bool INIFileImpl::ReadElement(string& work, SINIElement& element)
{
	/*
	element: Ident [Whitespace] = [Whitespace] value
	WhiteSpace = SPACE | TAB
	Ident = FirstAlpha { FollowAlpha }
	FirstAlpha = (a-z) | (A-Z) | _
	FollowAlpha = (FirstAlpha) | (0-9)
	value = *
	*/
	element.name.Clear();
	element.comment.Clear();
	element.value.Clear();

	int state = 0;
	auto it = work.First();
	for(state = 0; state < 5; ++state) {
		switch(state) {
		case 1:
		case 3:
			while(it != work.End() && (*it == ' ' || *it == '\t'))
				it++;
			break;

		case 0:
			// Read name
			if(core::IsAlpha(*it) || *it == '_') {
				element.name.Append(it);
				++it;
				while(it != work.End() && (core::IsAlpha(*it) || *it == '_' || core::IsDigit(*it))) {
					element.name.Append(it);
					++it;
				}
			} else {
				log::Debug("Invalid INI element name.");
				return false;
			}
			break;
		case 2:
			// Check for assingment
			if(*it != '=') {
				log::Debug("Missing INI assingment.");
				return false;
			}
			++it;
			break;
		case 4:
			// Read value
			for(; it != work.End(); ++it)
				element.value.Append(it);

			return true;
			break;
		}

		if(it == work.End()) {
			log::Debug("Unexpected end of INI line.");
			return false;
		}
	}

	return false;
}

bool INIFileImpl::LoadData()
{
	if(!m_File) {
		if(!m_FilePath.IsEmpty()) {
			m_File = m_FileSys->OpenFile(m_FilePath);
			if(!m_File) {
				log::Error("Can't open file: ~s.", m_FilePath);
				m_IsEOF = true;
				return false;
			}
		} else {
			m_IsEOF = true;
			return false;
		}
	}

	m_Sections.Clear();
	m_Elements.Clear();

	m_IsEOF = false;
	SetCommentChars(";#");
	m_ElementCommentPos = ECommentPos::Before;

	ReadSections();

	if(!m_FilePath.IsEmpty())
		m_File = nullptr;

	return (m_Sections.Size() > 0);
}

bool INIFileImpl::ParseSectionName(string& work, string& out)
{
	out.Clear();
	auto it = work.First();
	if(*it == '[') {
		for(it = it.Next(); it != work.End(); ++it) {
			if(*it == ']')
				return true;
			else
				out.Append(it);
		}
	}

	return false;
}

bool INIFileImpl::IsComment(const string& work, string::ConstIterator& commentBegin)
{
	if(work.IsEmpty())
		return false;

	bool isComment = false;
	const u32 character = *work.First();
	for(auto it = m_CommentChars.First(); it != m_CommentChars.End(); ++it) {
		if(character == *it) {
			isComment = true;
			break;
		}
	}

	if(isComment) {
		for(auto it = work.First().Next(); it != work.End(); ++it) {
			if(!core::IsSpace(*it)) {
				commentBegin = it;
				return true;
			}
		}
	}

	return false;
}

bool INIFileImpl::ReadLine(string& out)
{
	out.Clear();

	if(m_IsEOF)
		return false;

	char c;
	u32 readBytes;

	while(!m_IsEOF) {
		size_t spaceCount = 0;
		size_t count;
		do {
			out.Clear();
			count = 0;
			spaceCount = 0;
			while(!m_IsEOF) {
				do {
					readBytes = m_File->ReadBinary(1, &c);
				} while(readBytes == 1 && out.IsEmpty() && (c == ' ' || c == '\t'));

				if(readBytes == 0) {
					m_IsEOF = true;
					break;
				} else {
					if(core::IsSpace(c))
						++spaceCount;
					if(c == '\n' || c == '\r') {
						--spaceCount;
						break;
					}
					out.PushByte(c);
					++count;
				}
			}
		} while(spaceCount == count && !m_IsEOF);

		// Strip whitespaces at lineend
		out.RStrip();

		string::ConstIterator commentBegin;
		if(IsComment(out, commentBegin)) {
			if(m_LastComment.IsEmpty() == false)
				m_LastComment.Append("\n");
			m_LastComment.Append(commentBegin, out.End());
		} else {
			return !m_IsEOF;
		}
	}

	return !m_IsEOF;
}

void INIFileImpl::Close()
{
	m_File = nullptr;

	m_Sections.Clear();
	m_Elements.Clear();
}

INIFileImpl::SectionID INIFileImpl::GetSectionID(const char* section)
{
	if(m_Sections.Size() == 0)
		return InvalidID;

	if(m_Sections[m_CurrentSection].name == section)
		return m_CurrentSection;

	for(size_t i = m_CurrentSection + 1; i < m_Sections.Size(); ++i) {
		if(m_Sections[i].name == section) {
			m_CurrentSection = i;
			m_CurrentElement = 0;
			return (SectionID)i;
		}
	}

	for(size_t i = 0; i < m_CurrentSection; ++i) {
		if(m_Sections[i].name == section) {
			m_CurrentSection = i;
			m_CurrentElement = 0;
			return (SectionID)i;
		}
	}

	return InvalidID;
}
INIFileImpl::ElementID INIFileImpl::GetElemID(const char* section, const char* element, INIFileImpl::SectionID& outSection)
{
	SectionID sectionID = GetSectionID(section);

	outSection = sectionID;

	if(sectionID == InvalidID)
		return InvalidID;

	return GetElemID(sectionID, element);
}

INIFileImpl::ElementID INIFileImpl::GetElemID(INIFileImpl::SectionID sectionID, const char* element)
{
	if(sectionID == InvalidID)
		return InvalidID;

	if(m_Sections[sectionID].elemCount == 0)
		return InvalidID;

	ElementID FirstElem = m_Sections[sectionID].firstElem;
	size_t ElemCount = m_Sections[sectionID].elemCount;
	if(m_CurrentSection != sectionID)
		m_CurrentElement = 0;

	if(m_Elements[FirstElem + m_CurrentElement].name == element)
		return (ElementID)(m_CurrentElement + FirstElem);

	for(size_t i = m_CurrentElement + 1; i < ElemCount; ++i) {
		if(m_Elements[FirstElem + i].name == element) {
			m_CurrentElement = i;
			return (ElementID)i;
		}
	}

	for(size_t i = 0; i < m_CurrentElement; ++i) {
		if(m_Elements[FirstElem + i].name == element) {
			m_CurrentElement = i;
			return (ElementID)i;
		}
	}

	// TODO: If sorted use this to own advantage
	return InvalidID;
}

void INIFileImpl::Reload()
{
	LoadData();
}

void INIFileImpl::WriteComment(const string& comment, size_t identDepth, INIFileImpl::ECommentPos pos)
{
	if(comment.IsEmpty())
		return;

	u8 utf8Buffer[6];
	size_t utf8Size;
	u32 commentChar = GetCommentChar();

	for(size_t i = 0; i < identDepth; ++i)
		m_File->WriteBinary("\t", 1);

	utf8Size = core::CodePointToUTF8(commentChar, utf8Buffer) - utf8Buffer;

	m_File->WriteBinary(utf8Buffer, utf8Size);
	m_File->WriteBinary(" ", 1);

	for(auto it = comment.First(); it != comment.End(); ++it) {
		u32 c = *it;
		if(c == '\n') {
			if(it != comment.Last()) {
				for(size_t j = 0; j < identDepth; ++j)
					m_File->WriteBinary("\t", 1);
			}
			m_File->WriteBinary(NEWLINE, sizeof(NEWLINE) - 1);
			if(it != comment.Last()) {
				m_File->WriteBinary(" ", 1);
			}
		} else {
			utf8Size = core::CodePointToUTF8(c, utf8Buffer) - utf8Buffer;
			m_File->WriteBinary(utf8Buffer, utf8Size);
		}
	}

	if(pos == ECommentPos::Before)
		m_File->WriteBinary(NEWLINE, sizeof(NEWLINE) - 1);
}

bool INIFileImpl::Commit()
{
	if(!m_File) {
		if(!m_FilePath.IsEmpty()) {
			m_File = m_FileSys->OpenFile(m_FilePath, io::EFileMode::Write, true);
			if(!m_File) {
				log::Error("Can't open file: ~s.", m_FilePath);
				return false;
			}
		} else {
			return false;
		}
	}

	m_File->Seek(0, io::ESeekOrigin::Start);

	for(size_t i = 0; i < m_Elements.Size(); ++i) {
		SINIElement& element = m_Elements[i];
		SINISection& section = m_Sections[element.section];

		if(section.firstElem == i) {
			if(i != 0)
				m_File->WriteBinary(NEWLINE, sizeof(NEWLINE) - 1);

			// Write section
			WriteComment(section.comment, 0, ECommentPos::Before);
			m_File->WriteBinary("[", 1);
			m_File->WriteBinary(section.name.Data(), (u32)section.name.Size());
			m_File->WriteBinary("]" NEWLINE, sizeof(NEWLINE) + 1 - 1);
		}

		// Write element
		if(m_ElementCommentPos == ECommentPos::Before)
			WriteComment(element.comment, 1, ECommentPos::Before);
		m_File->WriteBinary("\t", 1);
		m_File->WriteBinary(element.name.Data(), (u32)element.name.Size());
		m_File->WriteBinary("=", 1);
		m_File->WriteBinary(element.value.Data(), (u32)element.value.Size());
		if(m_ElementCommentPos == ECommentPos::After)
			WriteComment(element.comment, 1, ECommentPos::After);
		m_File->WriteBinary(NEWLINE, sizeof(NEWLINE) - 1);
	}

	if(!m_FilePath.IsEmpty())
		m_File = nullptr;

	return true;
}


size_t INIFileImpl::GetSectionCount()
{
	return m_Sections.Size();
}

bool INIFileImpl::AddSection(const char* name, const char* comment)
{
	SectionID sectionID = GetSectionID(name);
	if(sectionID != InvalidID)
		m_Sections[sectionID].comment = comment;

	SINISection section;
	section.name = name;
	section.elemCount = 0;
	section.firstElem = InvalidID;
	section.comment = comment;
	section.sorted = true;
	m_Sections.PushBack(section);

	m_SectionSorted = false;

	return true;
}

bool INIFileImpl::RemoveSection(const char* section)
{
	SectionID sectionID = GetSectionID(section);
	if(sectionID == InvalidID)
		return false;

	SINISection& sec = m_Sections[sectionID];
	if(sec.firstElem != InvalidID)
		m_Elements.Erase(m_Elements.First() + sec.firstElem, sec.elemCount, true);

	for(size_t i = 0; i < m_Elements.Size(); ++i) {
		SINIElement& elem = m_Elements[i];
		if(elem.section > sectionID)
			elem.section--;
	}

	m_Sections.Erase(m_Sections.First() + sectionID, true);

	m_CurrentSection = sectionID + 1;
	if(m_CurrentSection >= m_Sections.Size())
		m_CurrentSection = m_Sections.Size() - 1;
	m_CurrentElement = 0;

	return true;
}

bool INIFileImpl::SortSections(INIFileImpl::ESorting sorting, bool recursive)
{
	if(m_Sections.Size() <= 1) {
		if(recursive) {
			for(size_t i = 0; i < m_Sections.Size(); ++i)
				SortElements(m_Sections[i].name.Data(), sorting);
		}

		return true;
	}

	auto sortAscending = [this](const size_t& a, const size_t& b) ->bool { return m_Sections[a].name < m_Sections[b].name; };
	auto sortDescending = [this](const size_t& a, const size_t& b) ->bool { return m_Sections[b].name < m_Sections[a].name; };

	struct DummyGenerator
	{
		size_t i = 0;
		size_t operator()()
		{
			return i++;
		}
	};

	std::vector<size_t> dummy(m_Sections.Size());
	std::generate(dummy.begin(), dummy.end(), DummyGenerator());
	if(sorting == ESorting::Ascending)
		std::stable_sort(dummy.begin(), dummy.end(), sortAscending);
	else
		std::stable_sort(dummy.begin(), dummy.end(), sortDescending);

	core::array<SINISection> newSections;
	newSections.Resize(m_Sections.Size());
	for(size_t i = 0; i < dummy.size(); ++i) {
		newSections[i] = m_Sections[dummy[i]];
		if(i != dummy[i]) {
			for(size_t j = newSections[i].firstElem; j < newSections[i].firstElem + newSections[i].elemCount; ++j) {
				m_Elements[j].section = i;
			}
		}
	}

	m_Sections = std::move(newSections);

	if(recursive) {
		for(size_t i = 0; i < m_Sections.Size(); ++i)
			SortElements(m_Sections[i].name.Data(), sorting);
	}
	return true;
}

bool INIFileImpl::SetSectionName(const char* section, const char* name)
{
	SectionID sectionID = GetSectionID(section);
	if(sectionID == InvalidID)
		return false;

	if(!name) {
		return false;
	} else {
		m_SectionSorted = false;
		m_Sections[sectionID].name = name;
	}
	return true;
}

bool INIFileImpl::SetSectionComment(const char* section, const char* comment)
{
	SectionID sectionID = GetSectionID(section);
	if(sectionID == InvalidID)
		return false;

	if(!comment)
		m_Sections[sectionID].comment.Clear();
	else
		m_Sections[sectionID].comment = comment;

	return true;
}

const string& INIFileImpl::GetSectionName(INIFileImpl::SectionID id)
{
	return m_Sections[id].name;
}

const string& INIFileImpl::GetSectionComment(const char* section)
{
	SectionID sectionID = GetSectionID(section);
	if(sectionID == InvalidID)
		return string::EMPTY;
	return m_Sections[sectionID].name;
}

size_t INIFileImpl::GetElementCount(const char* section)
{
	ElementID elementID = GetSectionID(section);
	if(elementID == InvalidID)
		return 0;
	return m_Sections[elementID].elemCount;
}

bool INIFileImpl::AddElement(const char* section, const char* name, const char* value, const char* comment)
{
	if(!value)
		return false;

	SectionID sectionID;
	ElementID elementID = GetElemID(section, name, sectionID);
	if(elementID != InvalidID) {
		m_Elements[elementID].value = value;
		m_Elements[elementID].comment = comment;
		return true;
	}

	if(sectionID == InvalidID) {
		if(AddSection(section))
			return AddElement(section, name, value, comment);
		return false;
	}


	SINIElement elem;
	elem.name = name;
	elem.section = sectionID;
	elem.value = value;
	elem.comment = comment;

	SectionID currentSec = sectionID;
	ElementID firstElem;
	if(m_Sections[sectionID].elemCount == 0)
		firstElem = InvalidID;
	else
		firstElem = m_Sections[sectionID].firstElem;

	for(size_t i = firstElem != InvalidID ? firstElem + 1 : 0; i < m_Elements.Size(); ++i) {
		SINIElement& e = m_Elements[i];
		if(currentSec < e.section) {
			m_Sections[e.section].firstElem++;
			i += m_Sections[e.section].elemCount;
		}
	}

	if(m_Sections[sectionID].elemCount == 0)
		m_Sections[sectionID].firstElem = m_Elements.Size();

	m_Sections[sectionID].sorted = false;

	m_Elements.Insert(elem, m_Elements.First() + m_Sections[sectionID].firstElem + m_Sections[sectionID].elemCount);
	m_Sections[sectionID].elemCount++;

	return true;
}

bool INIFileImpl::RemoveElement(const char* section, const char* element)
{
	SectionID sectionID;
	ElementID elementID = GetElemID(section, element, sectionID);
	if(elementID == InvalidID)
		return false;

	// For each elem group
	// Goto section of group
	// Decrement section.FirstElem
	SectionID currentSec = sectionID;
	for(size_t i = elementID + 1; i < m_Elements.Size(); ++i) {
		SINIElement& Elem = m_Elements[i];
		if(currentSec != Elem.section) {
			m_Sections[Elem.section].firstElem--;
			currentSec = Elem.section;
			i += m_Sections[Elem.section].elemCount;
		}
	}

	m_Elements.Erase(m_Elements.First() + elementID, true);
	m_Sections[sectionID].elemCount--;

	m_CurrentElement = 0;

	return false;
}

bool INIFileImpl::SortElements(const char* sectionName, INIFileImpl::ESorting sorting)
{
	SectionID sectionID = GetSectionID(sectionName);
	if(sectionID == InvalidID)
		return false;

	SINISection& section = m_Sections[sectionID];

	if(section.elemCount <= 1)
		return true;

	if(section.sorted) {
		if(sorting == section.sorting)
			return true;
		else {
			// Reverse section array and invert section.Ascending
			(void)0;
		}
	}

	auto sortAscending = [](const SINIElement& a, const SINIElement& b) ->bool { return a.name < b.name; };
	auto sortDescending = [](const SINIElement& a, const SINIElement& b) ->bool { return b.name < a.name; };

	if(sorting == ESorting::Ascending)
		std::stable_sort(&m_Elements[section.firstElem], &m_Elements[section.firstElem + section.elemCount], sortAscending);
	else
		std::stable_sort(&m_Elements[section.firstElem], &m_Elements[section.firstElem + section.elemCount], sortDescending);

	section.sorted = true;
	section.sorting = sorting;

	return false;
}

bool INIFileImpl::SetElementName(const char* section, const char* element, const char* name)
{
	SectionID sectionID;
	ElementID elementID = GetElemID(section, element, sectionID);
	if(elementID == -1)
		return false;

	m_Elements[elementID].name = name;

	m_Sections[sectionID].sorted = false;

	return true;
}

bool INIFileImpl::SetElementComment(const char* section, const char* element, const char* comment)
{
	SectionID sectionID;
	ElementID elementID = GetElemID(section, element, sectionID);
	if(elementID == InvalidID)
		return false;

	m_Elements[elementID].comment = comment;
	return true;
}

bool INIFileImpl::SetElementValue(const char* section, const char* element, const char* value)
{
	SectionID sectionID;
	ElementID elementID = GetElemID(section, element, sectionID);
	if(elementID == InvalidID)
		return AddElement(section, element, value);

	m_Elements[elementID].value = value;
	return true;
}

const string& INIFileImpl::GetElementName(const char* section, INIFileImpl::ElementID id)
{
	SectionID sectionID = GetSectionID(section);
	if(sectionID == InvalidID)
		return string::EMPTY;

	if(id >= m_Sections[sectionID].elemCount)
		return string::EMPTY;

	m_CurrentSection = sectionID;
	m_CurrentElement = id;

	return m_Elements[m_Sections[sectionID].firstElem + id].name;
}

const string& INIFileImpl::GetElementComment(const char* section, const char* element)
{
	SectionID sectionID;
	ElementID elementID = GetElemID(section, element, sectionID);
	if(elementID == InvalidID)
		return string::EMPTY;

	m_CurrentElement = elementID;
	m_CurrentSection = sectionID;

	return m_Elements[elementID].comment;
}

const string& INIFileImpl::GetElementValue(const char* section, const char* element)
{
	SectionID sectionID;
	ElementID elementID = GetElemID(section, element, sectionID);
	if(elementID == InvalidID)
		return string::EMPTY;

	m_CurrentElement = elementID;
	m_CurrentSection = sectionID;

	return m_Elements[elementID].value;
}

const string& INIFileImpl::GetCommentChars() const
{
	return m_CommentChars;
}

void INIFileImpl::SetCommentChars(const string& chars)
{
	lxAssert(!chars.IsEmpty());

	m_CommentChars = chars;
}

u32 INIFileImpl::GetCommentChar() const
{
	return *m_CommentChars.First();
}

void INIFileImpl::SetElementCommentPos(ECommentPos pos)
{
	m_ElementCommentPos = pos;
}

INIFile::ECommentPos INIFileImpl::GetElementCommentPos() const
{
	return m_ElementCommentPos;
}

bool INIFileImpl::IsEmpty()
{
	return (m_Sections.Size() == 0);
}

}
}