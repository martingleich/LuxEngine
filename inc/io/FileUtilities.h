#ifndef INCLUDED_FILE_UTITILIES_H
#define INCLUDED_FILE_UTITILIES_H
#include "io/File.h"
#include "io/ioConstants.h"
#include "core/lxString.h"

namespace lux
{
namespace io
{

class FileLineIterator : public core::BaseIterator<core::ForwardIteratorTag, core::String>
{
public:
	FileLineIterator() :
		m_File(nullptr)
	{
	}

	FileLineIterator(File* file, ELineEnding ending) :
		m_File(file),
		m_Ending(ending)
	{
		Next();
	}

	FileLineIterator GetEnd() const
	{
		return FileLineIterator();
	}

	const core::String& operator*() const
	{
		return m_Line;
	}
	const core::String* operator->() const
	{
		return &m_Line;
	}

	FileLineIterator& operator++()
	{
		Next();
		return *this;
	}

	FileLineIterator operator++(int)
	{
		FileLineIterator tmp(*this);
		Next();
		return tmp;
	}

	bool operator==(const FileLineIterator& other) const
	{
		return m_File == other.m_File;
	}
	bool operator!=(const FileLineIterator& other) const
	{
		return m_File != other.m_File;
	}

private:
	void Next()
	{
		if(m_File->IsEOF()) {
			m_File = nullptr;
			return;
		}
		m_Line.Clear();
		char c;
		int endCur = 0;
		while(true) {
			if(!m_File->ReadBinaryPart(1, &c)) {
				return;
			}
			if(m_Ending == ELineEnding::Unix) {
				if(c == '\n')
					return;
			} else if(m_Ending == ELineEnding::Macintosh) {
				if(c == '\r')
					return;
			} else if(m_Ending == ELineEnding::Window) {
				if(endCur == 0) {
					if(c == '\r') {
						++endCur;
						continue;
					}
				} else if(endCur == 1) {
					if(c == '\r') {
						m_Line.AppendRaw(&c, 1);
						endCur = 0;
					}
					if(c == '\n')
						return;
				}
			}
			m_Line.AppendRaw(&c, 1);
		}
	}

private:
	io::File* m_File;
	core::String m_Line;
	ELineEnding m_Ending;
};

inline core::Range<FileLineIterator> Lines(File* file, ELineEnding ending = ELineEnding::Unix)
{
	auto it = FileLineIterator(file, ending);
	return core::MakeRange(it, it.GetEnd());
}

inline ELineEnding GetLineEnding(File* file, s64 readBytes = 128)
{
	auto cursor = file->GetCursor();
	core::RawMemory data(core::SafeCast<size_t>(readBytes));
	readBytes = file->ReadBinaryPart(readBytes, data);
	if(readBytes == 0)
		return ELineEnding::Unknown;
	const char* chars = data;
	int winCount = 0;
	int rCount = 0;
	int nCount = 0;
	for(s64 i = 0; i < readBytes - 1; ++i) {
		if(chars[i] == '\r' && chars[i + 1] == '\n') {
			++winCount;
			++chars;
		} else if(chars[i] == '\r' && chars[i + 1] != '\n') {
			++rCount;
		} else if(chars[i] == '\n') {
			++nCount;
		}
	}
	ELineEnding out;
	if(winCount > rCount && winCount > nCount)
		out = ELineEnding::Window;
	else if(rCount > nCount)
		out = ELineEnding::Macintosh;
	else
		out = ELineEnding::Unix;
	file->Seek(cursor, io::ESeekOrigin::Start);
	return out;
}

} // namespace io
} // namespace lux

#endif // #ifndef INCLUDED_FILE_UTITILIES_H