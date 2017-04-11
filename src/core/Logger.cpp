#include "core/Logger.h"
//#include <fstream>
//#include <iostream>
#include "core/lxSTDIO.h"
#include "core/lxUnicodeConversion.h"
#ifdef LUX_WINDOWS
#include "StrippedWindows.h"
#endif

#ifdef LUX_WINDOWS
#define LUX_HAS_WIN32_DEBUG_PRINTER
#endif

namespace lux
{
namespace log
{

// TODO: Use the current locale to convert the charcter correctly
void fputs(FILE* file, const char* str)
{
	char buffer[64];
	char* bc = buffer;
	for(const char* cur = str; *cur; ++cur) {
		char c = *cur;
		if(c <= 0x7F) {
			*bc++ = (u8)(c & 0x7F);
		} else {
			*bc++ = (c >> 6) | 0xC0;
			*bc++ = (c & 0x3F) | 0x80;
		}

		if(buffer + sizeof(buffer) - bc < 2) {
			fwrite(buffer, bc - buffer, 1, file);
			bc = buffer;
		}
	}

	if(bc - buffer)
		fwrite(buffer, bc - buffer, 1, file);
}

void WriteWideCharAsUTF8ToFile(FILE* file, const wchar_t* str)
{
	char buffer[64];
	char* bc = buffer;
	for(const wchar_t* cur = str; *cur; ++cur) {
		wchar_t c = *cur;
		if(c <= 0x7F) {
			*bc++ = (u8)(c & 0x7F);
		} else if(c <= 0x7FF) {
			*bc++ = ((c >> 6) | 0xC0) & 0xFF;
			*bc++ = (c & 0x3F) | 0x80;
		} else {
			*bc++ = (c >> 12) | 0xE0;
			*bc++ = ((c >> 6) & 0x3F) | 0x80;
			*bc++ = (c & 0x3F) | 0x80;
		}

		if(buffer + sizeof(buffer) - bc < 3) {
			fwrite(buffer, bc - buffer, 1, file);
			bc = buffer;
		}
	}

	if(bc - buffer)
		fwrite(buffer, bc - buffer, 1, file);
}

extern LogSystem EngineLog(ELogLevel::Info);
extern Logger Debug(EngineLog, ELogLevel::Debug, FilePrinter);
extern Logger Info(EngineLog, ELogLevel::Info, FilePrinter);
extern Logger Warning(EngineLog, ELogLevel::Warning, FilePrinter);
extern Logger Error(EngineLog, ELogLevel::Error, FilePrinter);
extern Logger Log(EngineLog, ELogLevel::None, FilePrinter);

bool LogSystem::HasUnsetLogs() const
{
	bool result = false;
	for(auto it = m_Loggers.First(); it != m_Loggers.End(); ++it) {
		if((*it)->GetCurrentPrinter() == nullptr)
			result = true;
	}

	return result;
}

void LogSystem::Exit()
{
	for(auto it = m_Loggers.First(); it != m_Loggers.End(); ++it) {
		Logger* logger = *it;
		logger->SetNewPrinter(nullptr);
	}
	m_Loggers.Clear();
}

void LogSystem::SetNewPrinter(Printer* p, bool OnlyIfNULL)
{
	if(OnlyIfNULL) {
		for(auto it = m_Loggers.First(); it != m_Loggers.End(); ++it) {
			Logger* logger = *it;
			Printer* currentPrinter = logger->GetCurrentPrinter();
			if(!currentPrinter)
				logger->SetNewPrinter(p);
		}
	} else {
		for(auto it = m_Loggers.First(); it != m_Loggers.End(); ++it) {
			Logger* logger = *it;
			logger->SetNewPrinter(p);
		}
	}
}

namespace Impl
{

static const char* GetLogLevelName(ELogLevel ll)
{
	switch(ll) {
	case ELogLevel::Debug:
		return "DEBUG";
	case ELogLevel::Info:
		return "INFO";
	case ELogLevel::Warning:
		return "WARNING";
	case ELogLevel::Error:
		return "ERROR";
	case ELogLevel::None:
		return "";
	default:
		return "";
	}
}

class HTMLPrinter : public Printer
{
public:
	HTMLPrinter() :
		m_Settings("Log.html")
	{
	}
	virtual void SetSettings(const Printer::Settings& settings)
	{
		const HTMLPrinterSettings* data = dynamic_cast<const HTMLPrinterSettings*>(&settings);
		if(data)
			m_Settings = *data;
	}

	virtual bool Init()
	{
		Exit();

		m_File = core::FOpenUTF8(m_Settings.m_Path.Data(), "wb");
		if(!m_File)
			return false;

		fputs(m_File,
			"<!DOCTYPE html>\n"
			"<head>\n"
			"<title>Lux-Logfile</title>\n"
			"<meta charset=\"utf-8\"/>"
			"</head>\n"
			"<body>\n"
			"<font face=\"Arial\" size=\"2\">\n"
			"<table>\n");

		return Printer::Init();
	}

	const char* GetLogColor(ELogLevel ll)
	{
		switch(ll) {
		case ELogLevel::Debug:
			return "000000";
		case ELogLevel::Info:
			return "000080";
		case ELogLevel::Warning:
			return "FFFF00";
		case ELogLevel::Error:
			return "FF0000";
		default:
			return "000000";
		}
	}

	void Print(const string& s, ELogLevel ll)
	{
		if(ll != ELogLevel::None) {
			fputs(m_File, "<tr><td><font size=\"2\"><b><font color=\"#");
			fputs(m_File, GetLogColor(ll));
			fputs(m_File, "\">");
			fputs(m_File, GetLogLevelName(ll));
			fputs(m_File, ": </font></b>");
			fputs(m_File, "</td><td>");
		} else {
			fputs(m_File, "<tr><td><font size=\"2\">");
		}

		fputs(m_File, s.Data());
		fputs(m_File, "</font></td></tr>\n");

		fflush(m_File);
	}

	const string& EscapeString(const string& str)
	{
		assert(false);
		/*
		m_ConversionBuffer.Clear();
		for(size_t i = 0; i < str.Length(); ++i)
			ConvertChar(str[i], m_ConversionBuffer);
		return m_ConversionBuffer.GetString();
		*/
	}
/*
	void ConvertChar(wchar_t c, core::WStringBuffer& e)
	{
		switch((char)c) {
		case '\n':
			e << "<br/>";
			break;
		case '\"':
			e << "&quot;";
			break;
		case '&':
			e << "&amp;";
			break;
		case '<':
			e << "&lt;";
			break;
		case '>':
			e << "&gt;";
			break;
		case '\'':
			e << "&apos;";
			break;
		default:
			e << c;
		}
	}
*/
	void Exit()
	{
		if(IsInit() == false)
			return;

		fputs(m_File, "</table>\n"
			"</font>\n"
			"</body>\n"
			"</html>\n");
		fclose(m_File);
		Printer::Exit();
	}

private:
	HTMLPrinterSettings m_Settings;
	FILE* m_File;
};

class FilePrinter : public Printer
{
public:
	FilePrinter() :
		m_Settings("Log.txt")
	{
	}
	virtual void SetSettings(const Printer::Settings& settings)
	{
		const FilePrinterSettings* data = dynamic_cast<const FilePrinterSettings*>(&settings);
		if(data)
			m_Settings = *data;
	}

	virtual bool Init()
	{
		Exit();

		m_File = core::FOpenUTF8(m_Settings.m_Path.Data(), "wb");
		if(!m_File)
			return false;

		return Printer::Init();
	}

	void Print(const string& s, ELogLevel ll)
	{
		if(ll != ELogLevel::None) {
			fputs(m_File, GetLogLevelName(ll));
			fputs(m_File, ": ");
		}

		fputs(m_File, s.Data());
		fputs(m_File, "\n");

		fflush(m_File);
	}

	void Exit()
	{
		if(IsInit() == false)
			return;
		fclose(m_File);
		Printer::Exit();
	}

private:
	FILE* m_File;
	FilePrinterSettings m_Settings;
};

class ConsolePrinter : public Printer
{
public:
	bool Init()
	{
		Exit();
		return Printer::Init();
	}

	void Print(const string& s, ELogLevel ll)
	{
		if(ll != ELogLevel::None) {
			fputs(stdout, GetLogLevelName(ll));
			fputs(stdout, ": ");
		}

		puts(s.Data());
	}
};

#ifdef LUX_WINDOWS

class Win32DebugPrinter : public Printer
{
public:
	bool Init()
	{
		Exit();
#if WINVER >= 0x0A00
		// Used to enable unicode support of OutputDebugStringW
		DEBUG_EVENT event;
		WaitForDebugEventEx(&event, 0);
#else
		// No unicode support available
#endif

		return Printer::Init();
	}

	void Print(const string& s, ELogLevel ll)
	{
		string out;
		if(ll != ELogLevel::None) {
			out += GetLogLevelName(ll);
			out += ": ";
		}

		out += s;
		out += "\n";

		auto utf16 = core::UTF8ToUTF16(out.Data_c());

		OutputDebugStringW((const wchar_t*)utf16.Data_c());
	}
};

#endif

}

static Impl::HTMLPrinter realHTMLPrinter;
extern Printer* HTMLPrinter = &realHTMLPrinter;
static Impl::FilePrinter realFilePrinter;
extern Printer* FilePrinter = &realFilePrinter;
static Impl::ConsolePrinter realConsolePrinter;
extern Printer* ConsolePrinter = &realConsolePrinter;

#ifdef LUX_HAS_WIN32_DEBUG_PRINTER
static Impl::Win32DebugPrinter realWin32DebugPrinter;
extern Printer* Win32DebugPrinter = &realWin32DebugPrinter;
#endif

}
}