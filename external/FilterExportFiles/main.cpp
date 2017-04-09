#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <DbgHelp.h>
#include <cstdio>
#include <vector>
#pragma comment(lib, "Dbghelp.lib")

/*
A export name is build like
[scope::]name
scope if the full scope of the object
name is the name of the exported object, this may be a global function, a class, or a global variable.
*/
std::vector<std::string> g_ExportNames;
char g_Buffer[512];

bool StringStartsWith(const std::string& a, const std::string& b)
{
	if(a.length() < b.length())
		return false;

	return (memcmp(a.c_str(), b.c_str(), b.length()) == 0);
}

void FilterObject(const std::string& object)
{
	size_t start = 0;
	while(object.size() != start && object[start] != '?')
		++start;

	if(start == object.size())
		return;

	const char* dec = object.c_str() + start;

	DWORD result = UnDecorateSymbolName(dec, g_Buffer, sizeof(g_Buffer), UNDNAME_NAME_ONLY);
	if(result == 0)
		return;

	std::string undec(g_Buffer, result);

	for(auto it = g_ExportNames.begin(); it != g_ExportNames.end(); ++it) {
		if(StringStartsWith(undec, *it)) {
			if(it->back() != ':') { // Must be the full object name.
				if(undec.length() != it->length())
					continue;
			}

			fputs(dec, stdout);
			break;
		}
	}
}

int main(int argc, const char* argv[])
{
	if(argc < 2) {
		std::fprintf(stderr, "Missing command line arguments.");
		return 1;
	}

	const char* export_file_name = argv[1];

	FILE* export_file = std::fopen(export_file_name, "r");
	if(!export_file) {
		std::fprintf(stderr, "Invalid export file.");
		return 1;
	}

	g_ExportNames.clear();
	while(std::fgets(g_Buffer, sizeof(g_Buffer), export_file)) {
		if(g_Buffer[0] == '\0' || g_Buffer[0] == '#')
			continue;

		g_ExportNames.push_back(std::string(g_Buffer, strlen(g_Buffer) - 1));
	}

	std::fclose(export_file);

	while(fgets(g_Buffer, sizeof(g_Buffer), stdin))
		FilterObject(g_Buffer);

	return 0;
}
