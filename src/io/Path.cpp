#include "io/Path.h"
#include "io/Archive.h"

namespace lux
{
namespace io
{

Path Path::EMPTY = Path();

core::String Path::MakeStringCanonic(const core::StringView& str)
{
	core::String out;
	if(str.IsEmpty())
		return out;

	out.Reserve(str.Size());

	int cur = 0;
	int size = 0;
	for(int i = 0; i < str.Size(); ++i) {
		char c = str[i];
		if(c == '\\') {
			out.Append(str.SubString(cur, size));
			out.Append("/");
			cur = i+1;
			size = 0;
		} else {
			++size;
		}
	}
	out.Append(str.SubString(cur, size));

	// Strips spaces
	out.Strip();

	// Strip trailing slashes.
	int i = out.Size()-1;
	while(i > 0 && out[i] == '/')
		--i;
	out.Resize(i+1);

	return out;
}

void Path::Set(core::StringView str)
{
	m_RawData = MakeStringCanonic(str);
}

Path Path::GetFileDir() const
{
	int lastSlash = m_RawData.FindReverse("/");
	if(lastSlash == -1)
		return Path::EMPTY;
	else
		return io::Path(m_RawData.BeginSubStringView(lastSlash), m_Archive);
}

core::String Path::GetFileName(bool keepExtension) const
{
	int lastSlash = m_RawData.FindReverse("/");
	int begin = lastSlash+1; // 0 if couldn't be found.

	if(keepExtension) {
		return m_RawData.EndSubString(begin);
	} else {
		int lastDot = begin + m_RawData.EndSubStringView(begin).FindReverse(".");
		return m_RawData.SubStringView(begin, lastDot-begin);
	}
}

core::String Path::GetFileExtension() const
{
	int lastDot = m_RawData.FindReverse(".");
	if(lastDot == -1)
		return core::StringView::EMPTY;
	else
		return m_RawData.EndSubString(lastDot+1);
}

bool Path::IsAbsolute() const
{
	return (m_RawData.Size() > 0 && m_RawData[0] == '/') || (m_RawData.Size() > 1 && m_RawData[1] == ':');
}

Path Path::GetResolved(const Path& base) const
{
	// Copy the base path to the output.
	Path out;
	auto& outStr = out.m_RawData;
	if(!IsAbsolute()) {
		outStr.Reserve(base.AsView().Size() + 1);
		outStr.Append(base.AsView());
		outStr.Append("/");
		out.m_Archive = base.m_Archive;
	} else {
		out.m_Archive = m_Archive;
	}

	// 0 = Empty Directory
	// 1 = Dot - Directory
	// 2 = Two - Dot - Directory
	// 3 = Normal Directory
	int state = 0;
	for(char c : m_RawData.Bytes()) {
		if(c == '/') {
			if(state == 0 || state == 3) // Empty or normal dictonary
				outStr.Append("/");
			else if(state == 1) // Single dot
				outStr.Pop(); // Erase the dot
			else if(state == 2) { // Two dots
				int removed = outStr.Pop(3); // Erase the two dots and the last slash
				if(removed != 3)
					return Path::EMPTY;
				int lastSlash = outStr.FindReverse("/"); // Find the last slash...
				if(lastSlash == -1)
					return Path::EMPTY;
				outStr.Remove(lastSlash + 1, outStr.Size() - lastSlash - 1); //...and remove all after it.
			}
			state = 0;
			continue;	// Don't append last character
		} else if(c == '.') {
			if(state == 0)
				state = 1;
			else if(state == 1)
				state = 2;
			else
				state = 3;
		} else {
			state = 3;
		}
		outStr.Append(&c, 1);
	}

	if(outStr[outStr.Size()-1] == '/')
		outStr.Pop();

	return out;
}

void fmtPrint(format::Context& ctx, const Path& p, format::Placeholder& pl)
{
	ctx.AddSlice(p.AsView().Size(), p.AsView().Data());
	if(p.GetArchive()) {
		ctx.AddTerminatedSlice(" @");
		fmtPrint(ctx, p.GetArchive()->GetPath(), pl);
	}
}

}
}
