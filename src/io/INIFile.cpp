#include "io/INIFile.h"
#include "io/FileSystem.h"

#include "io/File.h"
#include "io/FileUtilities.h"

#include "core/Logger.h"
#include "core/lxAlgorithm.h"
#include "core/lxUnicodeConversion.h"

#include <algorithm>
#include <vector>


namespace lux
{
namespace io
{

INIFile::INIFile(File* f) :
	m_File(f),
	m_CurrentSection(0),
	m_CurrentElement(0),
	m_CommentChars{';', '#'},
	m_LineEnding(GetSystemLineEnding()),
	m_SectionSorted(true),
	m_SectionsAscending(true)
{
	LoadData(); // if file couldn't be loaded, handle as new empty file.
}

INIFile::INIFile(const Path& p) :
	m_File(nullptr),
	m_FilePath(p),
	m_CurrentSection(0),
	m_CurrentElement(0),
	m_CommentChars{';', '#'},
	m_LineEnding(GetSystemLineEnding()),
	m_SectionSorted(true),
	m_SectionsAscending(true)
{
	LoadData(); // if file couldn't be loaded, handle as new empty file.
}

INIFile::~INIFile()
{
}

void INIFile::Reload()
{
	LoadData();
}

bool INIFile::Commit()
{
	const char* newline = GetLineEndingChars(m_LineEnding);
	int newlineLen = (int)strlen(newline) - 1;

	if(!m_File) {
		if(!m_FilePath.IsEmpty()) {
			m_File = FileSystem::Instance()->OpenFile(m_FilePath, EFileModeFlag::Write, true);
			if(!m_File) {
				log::Error("Can't open file: ~s.", m_FilePath);
				return false;
			}
		} else {
			return false;
		}
	}

	m_File->Seek(0, ESeekOrigin::Start);

	for(int i = 0; i < m_Elements.Size(); ++i) {
		SINIElement& element = m_Elements[i];
		SINISection& section = m_Sections[element.section];

		if(section.firstElem == i) {
			if(i != 0)
				m_File->WriteBinary(newline, newlineLen);

			// Write section
			WriteComment(section.comment, 0);
			m_File->WriteBinary("[", 1);
			m_File->WriteBinary(section.name.Data(), (u32)section.name.Size());
			m_File->WriteBinary("]", 1);
			m_File->WriteBinary(newline, newlineLen);
		}

		// Write element
		WriteComment(element.comment, 1);
		m_File->WriteBinary("\t", 1);
		m_File->WriteBinary(element.name.Data(), (u32)element.name.Size());
		m_File->WriteBinary("=", 1);
		m_File->WriteBinary(element.value.Data(), (u32)element.value.Size());
		m_File->WriteBinary(newline, newlineLen);
	}

	if(!m_FilePath.IsEmpty())
		m_File = nullptr;

	return true;
}

int INIFile::GetSectionCount() const
{
	return m_Sections.Size();
}

void INIFile::SortSections(INIFile::ESorting sorting, bool recursive)
{
	if(m_Sections.Size() <= 1) {
		if(recursive)
			SortElements(0, sorting);
		return;
	}

	auto sortAscending = [this](const int& a, const int& b) ->bool { return m_Sections[a].name < m_Sections[b].name; };
	auto sortDescending = [this](const int& a, const int& b) ->bool { return m_Sections[b].name < m_Sections[a].name; };

	struct DummyGenerator
	{
		int i = 0;
		int operator()()
		{
			return i++;
		}
	};

	std::vector<int> dummy(m_Sections.Size());
	std::generate(dummy.begin(), dummy.end(), DummyGenerator());
	if(sorting == ESorting::Ascending)
		std::stable_sort(dummy.begin(), dummy.end(), sortAscending);
	else
		std::stable_sort(dummy.begin(), dummy.end(), sortDescending);

	core::Array<SINISection> newSections;
	newSections.Resize(m_Sections.Size());
	for(int i = 0; i < (int)dummy.size(); ++i) {
		newSections[i] = m_Sections[dummy[i]];
		if(i != dummy[i]) {
			for(int j = newSections[i].firstElem; j < newSections[i].firstElem + newSections[i].elemCount; ++j) {
				m_Elements[j].section = i;
			}
		}
	}

	m_Sections = std::move(newSections);

	if(recursive) {
		for(int i = 0; i < m_Sections.Size(); ++i)
			SortElements(i, sorting);
	}
}

INIFile::Section INIFile::AddSection(const core::StringView& name, const core::StringView& comment)
{
	int sectionID = GetSectionID(name);
	if(sectionID != InvalidID) {
		m_Sections[sectionID].comment = comment;
		return Section(this, sectionID);
	}

	SINISection section;
	section.name = name;
	section.elemCount = 0;
	section.firstElem = InvalidID;
	section.comment = comment;
	section.sorted = true;
	m_Sections.PushBack(section);

	m_SectionSorted = false;
	return Section(this, sectionID);
}

void INIFile::RemoveSection(int sectionID)
{
	SINISection& sec = m_Sections.At(sectionID);
	if(sec.firstElem != InvalidID)
		m_Elements.Erase(m_Elements.First() + sec.firstElem, sec.elemCount, true);

	for(int i = 0; i < m_Elements.Size(); ++i) {
		SINIElement& elem = m_Elements[i];
		if(elem.section > sectionID)
			elem.section--;
	}

	m_Sections.Erase(m_Sections.First() + sectionID, true);

	m_CurrentSection = sectionID + 1;
	if(m_CurrentSection >= m_Sections.Size())
		m_CurrentSection = m_Sections.Size() - 1;
	m_CurrentElement = 0;
}

void INIFile::SetSectionName(int sectionID, const core::StringView& name)
{
	m_Sections.At(sectionID).name = name;
	m_SectionSorted = false;
}

void INIFile::SetSectionComment(int sectionID, const core::StringView& comment)
{
	m_Sections.At(sectionID).comment = comment;
}

const core::String& INIFile::GetSectionName(int id) const
{
	return m_Sections.At(id).name;
}

const core::String& INIFile::GetSectionComment(int id) const
{
	return m_Sections.At(id).name;
}

INIFile::Section INIFile::GetFirstSection()
{
	return Section(this, 0);
}

INIFile::Section INIFile::GetSection(const core::StringView& section)
{
	auto sectionID = GetSectionID(section);
	if(sectionID == InvalidID)
		return Section();
	return Section(this, sectionID);
}

int INIFile::GetSectionID(const core::StringView& section) const
{
	if(m_Sections.Size() == 0)
		return InvalidID;

	if(m_Sections[m_CurrentSection].name == section)
		return m_CurrentSection;

	for(int i = m_CurrentSection + 1; i < m_Sections.Size(); ++i) {
		if(m_Sections[i].name == section) {
			m_CurrentSection = i;
			m_CurrentElement = 0;
			return (int)i;
		}
	}

	for(int i = 0; i < m_CurrentSection; ++i) {
		if(m_Sections[i].name == section) {
			m_CurrentSection = i;
			m_CurrentElement = 0;
			return (int)i;
		}
	}

	return InvalidID;
}

int INIFile::GetElementCount(int section) const
{
	return m_Sections.At(section).elemCount;
}

void INIFile::SortElements(int sectionID, INIFile::ESorting sorting)
{
	SINISection& section = m_Sections[sectionID];

	if(section.elemCount <= 1)
		return;

	if(section.sorted) {
		if(sorting == section.sorting)
			return;
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
}

INIFile::Element INIFile::AddElement(int sectionID, const core::StringView& name, const core::StringView& value, const core::StringView& comment)
{
	if(value.IsEmpty())
		return Element();

	auto elementID = GetElemID(sectionID, name);
	if(elementID != InvalidID) {
		auto& elem = GetElement(sectionID, elementID);
		elem.value = value;
		elem.comment = comment;
		return Element(this, sectionID, elementID);
	}

	SINIElement elem;
	elem.name = name;
	elem.section = sectionID;
	elem.value = value;
	elem.comment = comment;

	int currentSec = sectionID;
	int firstElem;
	if(m_Sections[sectionID].elemCount == 0)
		firstElem = m_Elements.Size();
	else
		firstElem = m_Sections[sectionID].firstElem;

	for(int i = firstElem + 1; i < m_Elements.Size(); ++i) {
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

	return Element(this, sectionID, elementID);
}

void INIFile::RemoveElement(int sectionID, int elementID)
{
	auto firstElem = m_Sections[sectionID].firstElem;

	// For each elem group
	// Goto section of group
	// Decrement section.firstElem
	int currentSec = sectionID;
	for(int i = firstElem + elementID + 1; i < m_Elements.Size(); ++i) {
		SINIElement& Elem = m_Elements[i];
		if(currentSec != Elem.section) {
			m_Sections[Elem.section].firstElem--;
			currentSec = Elem.section;
			i += m_Sections[Elem.section].elemCount;
		}
	}

	m_Elements.Erase(m_Elements.First() + firstElem + elementID, true);
	m_Sections[sectionID].elemCount--;

	if(m_CurrentElement > 0)
		m_CurrentElement--;
}

void INIFile::SetElementName(int sectionID, int elementID, const core::StringView& name)
{
	GetElement(sectionID, elementID).name = name;
	m_Sections[sectionID].sorted = false;
}

void INIFile::SetElementComment(int sectionID, int elementID, const core::StringView& comment)
{
	GetElement(sectionID, elementID).comment = comment;
}

void INIFile::SetElementValue(int sectionID, int elementID, const core::StringView& value)
{
	GetElement(sectionID, elementID).value = value;
}

const core::String& INIFile::GetElementName(int sectionID, int id) const
{
	return GetElement(sectionID, id).name;
}

const core::String& INIFile::GetElementComment(int sectionID, int elementID) const
{
	return GetElement(sectionID, elementID).comment;
}

const core::String& INIFile::GetElementValue(int sectionID, int elementID) const
{
	return GetElement(sectionID, elementID).value;
}

INIFile::Element INIFile::GetElement(const core::StringView& section, const core::StringView& element)
{
	int sectionID;
	auto elementID = GetElemID(section, element, sectionID);
	if(elementID == InvalidID)
		return Element();
	return Element(this, sectionID, elementID);
}

int INIFile::GetElemID(const core::StringView& section, const core::StringView& element, int& outSection) const
{
	outSection = GetSectionID(section);

	if(outSection == InvalidID)
		return InvalidID;

	return GetElemID(outSection, element);
}

int INIFile::GetElemID(int sectionID, const core::StringView& element) const
{
	if(sectionID == InvalidID)
		return InvalidID;

	if(m_Sections[sectionID].elemCount == 0)
		return InvalidID;

	int firstElem = m_Sections[sectionID].firstElem;
	int elemCount = m_Sections[sectionID].elemCount;
	if(m_CurrentSection != sectionID)
		m_CurrentElement = 0;

	for(int i = m_CurrentElement; i < elemCount; ++i) {
		if(m_Elements[firstElem + i].name == element) {
			m_CurrentElement = i;
			return i;
		}
	}

	for(int i = 0; i < m_CurrentElement; ++i) {
		if(m_Elements[firstElem + i].name == element) {
			m_CurrentElement = i;
			return i;
		}
	}

	return InvalidID;
}

bool INIFile::IsEmpty() const
{
	return (m_Sections.Size() == 0);
}

char INIFile::GetCommentChar() const
{
	return m_CommentChars[0];
}

bool INIFile::LoadData()
{
	if(!m_File) {
		if(!m_FilePath.IsEmpty()) {
			m_File = FileSystem::Instance()->OpenFile(m_FilePath);
			if(!m_File) {
				log::Error("Can't open file: ~s.", m_FilePath);
				return false;
			}
		} else {
			return false;
		}
	}

	m_Sections.Clear();
	m_Elements.Clear();

	ReadSections();

	if(!m_FilePath.IsEmpty())
		m_File = nullptr;

	return (m_Sections.Size() > 0);
}

bool INIFile::ParseSectionName(const core::String& work, core::String& out)
{
	out.Clear();
	auto it = work.Bytes().First();
	if(*it == '[') {
		for(it++; it != work.Bytes().End(); ++it) {
			if(*it == ']')
				return true;
			else
				out.PushByte(*it);
		}
	}

	return false;
}

bool INIFile::ReadSections()
{
	SINIElement element;
	core::String sectionName;
	core::String lastComment;
	int sectionID = InvalidID;

	int commentBegin = 0;
	m_LineEnding = GetLineEnding(m_File);
	for(auto& line : Lines(m_File, m_LineEnding)) {
		line.RStrip();
		line.LStrip();
		if(line.IsWhitespace())
			continue;

		if(IsComment(line, commentBegin)) {
			if(lastComment.IsEmpty() == false)
				lastComment.Append("\n");
			lastComment.Append(line.SubString(commentBegin, line.Size()-1));
			continue;
		}

		if(line.Data()[0] == '[') {
			// It's the name of the section
			if(!ParseSectionName(line, sectionName)) {
				log::Debug("Invalid INI-section name: ~s.", line);
				continue;
			}

			sectionID = GetSectionID(sectionName.Data());
			if(sectionID != InvalidID) {
				if(!lastComment.IsEmpty() && !m_Sections[sectionID].comment.IsEmpty())
					m_Sections[sectionID].comment.Append("\n");
				m_Sections[sectionID].comment.Append(lastComment);
			} else {
				sectionID = m_Sections.Size();
				auto& section = m_Sections.EmplaceBack();
				section.name = sectionName;
				section.sorted = false;
				section.elemCount = 0;
				section.firstElem = InvalidID;
				section.comment = lastComment;
			}
			element.section = sectionID;
			lastComment.Clear();
		} else if(sectionID != InvalidID) {
			if(!ReadElement(line, element)) {
				log::Debug("Invalid INI-element name: ~s.", line);
				continue;
			}
			element.comment = lastComment;
			lastComment.Clear();

			m_Elements.PushBack(element);

			auto& section = m_Sections[element.section];
			if(section.elemCount == 0)
				section.firstElem = m_Elements.Size() - 1;

			++section.elemCount;
		}
	}

	return true;
}

bool INIFile::ReadElement(const core::String& work, SINIElement& element)
{
	/*
	element: Ident [Whitespace] = [Whitespace] value
	WhiteSpace = SPACE | TAB
	Ident = Alpha { Alpha }
	Alpha = (0-9) | (a-z) | (A-Z) | _
	value = *
	*/
	element.name.Clear();
	element.comment.Clear();
	element.value.Clear();

	int state = 0;
	auto it = work.Bytes().First();
	for(state = 0; state < 5; ++state) {
		switch(state) {
		case 1:
		case 3:
			while(it != work.End() && (*it == ' ' || *it == '\t'))
				it++;
			break;

		case 0:
			// Read name
			if(*it != '=' && !core::IsSpace(*it)) {
				element.name.Append(it);
				++it;
				while(it != work.End() && (*it != '=' && !core::IsSpace(*it))) {
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

bool INIFile::IsComment(const core::String& work, int& commentBegin)
{
	if(work.IsEmpty())
		return false;

	bool isComment = false;
	auto wit = work.Bytes().First();
	for(auto c : m_CommentChars) {
		if(*wit == c) {
			++wit;
			isComment = true;
			break;
		}
	}

	if(isComment) {
		for(; wit != work.Bytes().End(); ++wit) {
			if(!core::IsSpace(*wit)) {
				commentBegin = wit-work.Bytes().First();
				return true;
			}
		}
	}

	return false;
}

void INIFile::WriteComment(const core::String& comment, int identDepth)
{
	if(comment.IsEmpty())
		return;

	static char TABS[] = "\t\t\t\t\t\t";
	const char* newline = GetLineEndingChars(m_LineEnding);
	int newlineLen = (int)strlen(newline) - 1;

	u8 commentChar = GetCommentChar();

	if(identDepth)
		m_File->WriteBinary(TABS, identDepth);

	m_File->WriteBinary(&commentChar, 1);
	m_File->WriteBinary(" ", 1);

	int start = 0;
	int count = 0;
	for(auto c : comment.Bytes()) {
		if(c == '\n') {
			if(count != 0) {
				m_File->WriteBinary(comment.Data() + start, count);
				start += count + 1;
				count = 0;
			}
			if(identDepth)
				m_File->WriteBinary(TABS, identDepth);
			m_File->WriteBinary(newline, newlineLen);
			m_File->WriteBinary(&commentChar, 1);
			m_File->WriteBinary(" ", 1);
		} else {
			++count;
		}
	}

	m_File->WriteBinary(newline, newlineLen);
}

INIFile::SINIElement& INIFile::GetElement(int sectionID, int elementID)
{
	return m_Elements[m_Sections[sectionID].firstElem + elementID];
}

const INIFile::SINIElement& INIFile::GetElement(int sectionID, int elementID) const
{
	return m_Elements[m_Sections[sectionID].firstElem + elementID];
}

}
}
