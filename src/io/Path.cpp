#include "io/Path.h"
#include "io/Archive.h"

namespace lux
{
namespace io
{

Path Path::EMPTY = Path();

void Path::Set(core::StringView str)
{
	m_RawData.Clear();
	if(str.IsEmpty())
		return;

	m_RawData.Reserve(str.Size());

	int cur = 0;
	int size = 0;
	for(int i = 0; i < str.Size(); ++i) {
		char c = str[i];
		if(c == '\\') {
			m_RawData.Append(str.SubString(cur, size));
			m_RawData.Append("/", 1);
			cur = i+1;
			size = 0;
		} else {
			++size;
		}
	}
	m_RawData.Append(str.SubString(cur, size));

	// Strips spaces
	m_RawData.Strip();

	// Strip trailing slashes.
	int i = m_RawData.Size()-1;
	while(i > 0 && m_RawData[i] == '/')
		--i;
	m_RawData.Resize(i+1);
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

Path Path::GetResolved(const Path& base) const
{
	core::String out;
	auto relP = m_RawData.Data();
	auto relS = m_RawData.Size();
	if(relS > 0 && relP[0] == '/')
		(void)0; // rel is already absolute
	else if(relS > 1 && relP[1] == ':')
		(void)0; // rel is already absolute
	else {
		out = base.AsView();
		out.Append("/");
	}

	// 0 = Empty Directory
	// 1 = Dot - Directory
	// 2 = Two - Dot - Directory
	// 3 = Normal Directory
	int state = 0;
	for(char c : m_RawData.Bytes()) {
		if(c == '/') {
			if(state == 0 || state == 3) // Empty or normal dictonary
				out.Append("/");
			else if(state == 1) // Single dot
				out.Pop(); // Erase the dot
			else if(state == 2) { // Two dots
				int removed = out.Pop(3); // Erase the two dots and the last slash
				if(removed != 3)
					return Path::EMPTY;
				int lastSlash = out.FindReverse("/"); // Find the last slash...
				if(lastSlash == -1)
					return Path::EMPTY;
				out.Remove(lastSlash + 1, out.Size() - lastSlash - 1); //...and remove all after it.
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
		out.Append(&c, 1);
	}

	// out is always a valid path, so don't create a copy
	Path pout("", base.GetArchive());
	pout.PutString(std::move(out));
	return pout;
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
