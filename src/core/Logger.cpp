#include "core/Logger.h"
#include "core/lxSTDIO.h"
#include "core/lxUnicodeConversion.h"
#include "core/Clock.h"
#include "core/StringConverter.h"
#include "io/ioExceptions.h"
#ifdef LUX_WINDOWS
#include "platform/StrippedWindows.h"
#endif

#ifdef LUX_WINDOWS
#define LUX_HAS_WIN32_DEBUG_PRINTER
#endif

namespace lux
{
namespace log
{

LogSystem EngineLog(ELogLevel::Info);
Logger Debug(EngineLog, ELogLevel::Debug);
Logger Info(EngineLog, ELogLevel::Info);
Logger Warning(EngineLog, ELogLevel::Warning);
Logger Error(EngineLog, ELogLevel::Error);
Logger Log(EngineLog, ELogLevel::None);

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
		logger->SetPrinter(nullptr);
	}
	m_Loggers.Clear();
}

void LogSystem::SetPrinter(Printer* p, bool OnlyIfNULL)
{
	if(OnlyIfNULL) {
		for(auto it = m_Loggers.First(); it != m_Loggers.End(); ++it) {
			Logger* logger = *it;
			Printer* currentPrinter = logger->GetCurrentPrinter();
			if(!currentPrinter)
				logger->SetPrinter(p);
		}
	} else {
		for(auto it = m_Loggers.First(); it != m_Loggers.End(); ++it) {
			Logger* logger = *it;
			logger->SetPrinter(p);
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

#if 0
class HTMLPrinter : public Printer
{
public:
	HTMLPrinter() :
		m_Settings("Log.html")
	{
	}
	virtual void Configure(const Printer::Settings& settings)
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

	void Print(const core::String& s, ELogLevel ll)
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

	const core::String& EscapeString(const core::String& str)
	{
		lxAssert(false);
		return core::String::EMPTY;
	}
	
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
#endif // Disable HTML Printer

class FilePrinter : public Printer
{
public:
	FilePrinter() :
		m_Settings("Log.txt")
	{
	}
	virtual void Configure(const Printer::Settings& settings)
	{
		const FilePrinterSettings* data = dynamic_cast<const FilePrinterSettings*>(&settings);
		if(data)
			m_Settings = *data;
	}

	virtual void Init()
	{
		Exit();

		m_File = core::FOpenUTF8(m_Settings.FilePath.Data(), "wb");
		if(!m_File)
			throw io::FileNotFoundException(m_Settings.FilePath.Data_c());

		Printer::Init();
	}

	void Print(const core::String& s, ELogLevel ll)
	{
		if(ll != ELogLevel::None) {
			fputs(GetLogLevelName(ll), m_File);
			if(m_Settings.ShowTime) {
				auto time = core::Clock::GetDateAndTime();
				fprintf(m_File, "(%.2d.%.2d.%.4d %.2d:%.2d:%.2d'%.3d)", time.dayOfMonth, time.month, time.year, time.hours, time.minutes, time.seconds, time.milliseconds);
			}
			fputs(": ", m_File);
		}

		fputs(s.Data(), m_File);
		fputs("\n", m_File);

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
	void Init()
	{
		Exit();
		Printer::Init();
	}

	virtual void Configure(const Printer::Settings& settings)
	{
		const ConsolePrinterSettings* data = dynamic_cast<const ConsolePrinterSettings*>(&settings);
		if(data)
			m_Settings = *data;
	}

	void Print(const core::String& s, ELogLevel ll)
	{
		if(ll != ELogLevel::None) {
			fputs(GetLogLevelName(ll), stdout);
			if(m_Settings.ShowTime) {
				auto time = core::Clock::GetDateAndTime();
				fprintf(stdout, "(%.2d.%.2d.%.4d %.2d:%.2d:%.2d'%.3d)", time.dayOfMonth, time.month, time.year, time.hours, time.minutes, time.seconds, time.milliseconds);
			}

			fputs(": ", stdout);
		}

		puts(s.Data());
	}

private:
	ConsolePrinterSettings m_Settings;
};

#ifdef LUX_WINDOWS

class Win32DebugPrinter : public Printer
{
public:
	void Init()
	{
		Exit();
#if WINVER >= 0x0A00
		// Used to enable unicode support of OutputDebugStringW
		DEBUG_EVENT event;
		WaitForDebugEventEx(&event, 0);
#else
		// No unicode support available
#endif

		Printer::Init();
	}

	void Print(const core::String& s, ELogLevel ll)
	{
		static char BUFFER[100];

		core::String out;
		if(ll != ELogLevel::None) {
			out += GetLogLevelName(ll);

			auto time = core::Clock::GetDateAndTime();
			sprintf(BUFFER, "(%.2d.%.2d.%.4d %.2d:%.2d:%.2d'%.3d)", time.dayOfMonth, time.month, time.year, time.hours, time.minutes, time.seconds, time.milliseconds);
			out += BUFFER;

			out += ": ";
		}

		out += s;
		out += "\n";
		auto utf16 = core::UTF8ToUTF16(out.Data_c());

		OutputDebugStringW((const wchar_t*)utf16.Data_c());
	}
};

#endif // LUX_WINDOWS

}

Printer* HTMLPrinter = nullptr;
static Impl::FilePrinter realFilePrinter;
Printer* FilePrinter = &realFilePrinter;
static Impl::ConsolePrinter realConsolePrinter;
Printer* ConsolePrinter = &realConsolePrinter;

#ifdef LUX_HAS_WIN32_DEBUG_PRINTER
static Impl::Win32DebugPrinter realWin32DebugPrinter;
Printer* Win32DebugPrinter = &realWin32DebugPrinter;
#endif

}
}
