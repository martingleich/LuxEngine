#include "io/path.h"

namespace lux
{
namespace io
{

path GetFileExtension(const path& filename)
{
	auto lastDot = filename.FindReverse(".");
	if(lastDot == filename.End())
		return path::EMPTY;
	else
		return filename.SubString(lastDot.Next(), filename.End());
}

static path::ConstIterator FindLastSlash(const path& filename)
{
	auto it = filename.Last();
	for(; it != filename.First(); --it) {
		if(*it == '/' || *it == '\\')
			return it;
	}
	if(*it == '/' || *it == '//')
		return it;
	return filename.End();
}

path GetFileDir(const path& filename)
{
	auto lastSlash = FindLastSlash(filename);
	if(lastSlash == filename.End())
		return path::EMPTY;
	else
		return NormalizePath(filename.SubString(filename.First(), lastSlash), true);
}

string GetFilenameOnly(const path& filename, bool keepExtension)
{
	auto lastSlash = FindLastSlash(filename);
	path::ConstIterator cut;
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

path NormalizePath(const path& filename, bool isDirectory)
{
	if(filename.IsEmpty())
		return path::EMPTY;

	path out(filename);
	out.Replace("/", "\\");
	if(isDirectory && !out.EndsWith("/"))
		out += "/";

	return out;
}

io::path MakeAbsolutePath(const io::path& base, const io::path& rel)
{
	if(rel.IsEmpty())
		return base;

	io::path out;
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
					return path::EMPTY;
				auto lastSlash = out.FindReverse("/"); // Find the last slash...
				if(lastSlash == out.End())
					return path::EMPTY;
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