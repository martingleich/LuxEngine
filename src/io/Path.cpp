#include "io/Path.h"

namespace lux
{
namespace io
{

Path GetFileExtension(const Path& filename)
{
	auto lastDot = filename.FindReverse(".");
	if(lastDot == filename.End())
		return Path::EMPTY;
	else
		return filename.SubString(lastDot.Next(), filename.End());
}

static Path::ConstIterator FindLastSlash(const Path& filename)
{
	auto it = filename.Last();
	for(; it != filename.First(); --it) {
		if(*it == '/' || *it == '\\')
			return it;
	}
	if(*it == '/' || *it == '\\')
		return it;
	return filename.End();
}

Path GetFileDir(const Path& filename)
{
	auto lastSlash = FindLastSlash(filename);
	if(lastSlash == filename.End())
		return Path::EMPTY;
	else
		return NormalizePath(filename.SubString(filename.First(), lastSlash), true);
}

core::String GetFilenameOnly(const Path& filename, bool keepExtension)
{
	auto lastSlash = FindLastSlash(filename);
	Path::ConstIterator cut;
	if(lastSlash == filename.End())
		cut = filename.First();
	else
		cut = lastSlash+1;

	auto lastDot = filename.FindReverse(".", cut);

	if(keepExtension)
		return filename.SubString(cut, filename.End());
	else
		return filename.SubString(cut, lastDot);
}

Path NormalizePath(const Path& filename, bool isDirectory)
{
	if(filename.IsEmpty())
		return Path::EMPTY;

	Path out(filename);
	out.Replace("/", "\\");
	if(isDirectory && !out.EndsWith("/"))
		out += "/";

	return out;
}

io::Path MakeAbsolutePath(const io::Path& base, const io::Path& rel)
{
	if(rel.IsEmpty())
		return base;

	io::Path out;
	auto first = rel.First();
	auto second = first + 1;
	if(*first == '/' || *first == '\\')
		(void)0; // rel is already absolute
	else if(second != rel.End() && *second == ':')
		(void)0; // rel is already absolute
	else
		out = NormalizePath(base, true);

	// 0 = Empty Directory
	// 1 = Dot - Directory
	// 2 = Two - Dot - Directory
	// 3 = Normal Directory
	int state = 0;
	for(auto it = rel.First(); it != rel.End(); ++it) {
		u32 c = *it;
		if(c == '/' || c == '\\') {
			if(state == 0 || state == 3) // Empty or normal dictonary
				out.Append("/");
			else if(state == 1) // Single dot
				out.Pop(); // Erase the dot
			else if(state == 2) { // Two dots
				size_t removed = out.Pop(3); // Erase the two dots and the last slash
				if(removed != 3)
					return Path::EMPTY;
				auto lastSlash = out.FindReverse("/"); // Find the last slash...
				if(lastSlash == out.End())
					return Path::EMPTY;
				out.Remove(lastSlash + 1, out.End()); //...and remove all after it.
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
		out.Append(it);
	}

	return out;
}

}
}
