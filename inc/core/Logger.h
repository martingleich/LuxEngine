#ifndef INCLUDED_LUX_LOGGER_H
#define INCLUDED_LUX_LOGGER_H
#include "core/StringConverter.h"
#include "core/lxAlgorithm.h"
#include "core/lxName.h"
#include "core/lxArray.h"
#include "core/HelperTemplates.h"
#include "io/Path.h"

#include <mutex>

namespace lux
{
namespace log
{

//! The type or level of log messages
enum class ELogLevel
{
	Debug, //!< A debug message
	Info, //!< A information message
	Warning, //!< A warning message
	Error, //!< An error message
	None, //!< A message without any type
};

//! A log printer
/**
A printer describes how log messages are shown to the user.
\ref HTMLPrinter
\ref FilePrinter
\ref ConsolePrinter
*/
class Printer : core::Uncopyable
{
public:
	struct Settings
	{
		Settings() :
			ShowTime(true)
		{
		}

		virtual ~Settings() {}

		bool ShowTime;
	};

public:
	virtual ~Printer() {}

	//! Set the configuration data for the logger.
	/**
	Must be done before first use. If used after first use the function might not have any effect.
	*/
	virtual void Configure(const Settings& data) { LUX_UNUSED(data); }

	//! Initialize the logger, happens automatically.
	virtual void Init() = 0;
	//! Exits the logger, happens automatically.
	virtual void Exit() = 0;

	//! Finish a entry and flush it.
	/**
	This function is threadsafe
	*/
	void PrintSync(const core::String& s, ELogLevel ll)
	{
		std::lock_guard<std::mutex> _(m_PrinterLock);
		Print(s, ll);
	}
protected:
	virtual void Print(const core::String& s, ELogLevel ll) = 0;

private:
	std::mutex m_PrinterLock;
};

LUX_API void SetLogLevel(ELogLevel ll);
LUX_API ELogLevel GetLogLevel();
LUX_API void SetPrinter(Printer* p);
LUX_API Printer* GetPrinter();

/*
Loggers are functors instead of pure function, to allow
easier future extensions.
*/

//! The logger class.
/**
Each logger class represents a log level.
All loggers can be used in diffrent threads.
*/
class Logger : core::Uncopyable
{
public:
	explicit Logger(const ELogLevel ll) :
		m_MyLogLevel(ll)
	{
	}

	template <typename... T>
	void Write(const char* format, const T&... data)
	{
		auto printer = GetPrinter();
		auto curLogLevel = GetLogLevel();
		if(!printer)
			return;

		if(curLogLevel <= m_MyLogLevel && curLogLevel != ELogLevel::None) {
			ifconst(sizeof...(data))
			{
				core::String out;
				core::StringSink sink(out);
				format::format(sink, format, data...);

				printer->PrintSync(out, m_MyLogLevel);
			} else {
				printer->PrintSync(format, m_MyLogLevel);
			}
		}
	}

	template <typename... T>
	void operator()(const char* format, const T&... data)
	{
		Write(format, data...);
	}

	template <typename... T>
	void operator()(const core::String& format, const T&... data)
	{
		Write(format.Data(), data...);
	}

private:
	const ELogLevel m_MyLogLevel;
};

struct HTMLPrinterSettings : public Printer::Settings
{
	HTMLPrinterSettings(const io::Path& p) :
		FilePath(p)
	{
	}

	io::Path FilePath;
};

struct FilePrinterSettings : public Printer::Settings
{
	FilePrinterSettings(const io::Path& p) :
		FilePath(p)
	{
	}

	io::Path FilePath;
};

struct ConsolePrinterSettings : public Printer::Settings
{};

extern LUX_API Printer* HTMLPrinter; //!< Prints the log into a html file(check for null before using)
extern LUX_API Printer* FilePrinter; //!< Prints the log into a text file
extern LUX_API Printer* ConsolePrinter; //!< Prints the log to stdout
extern LUX_API Printer* Win32DebugPrinter; //!< Prints the log the the win32 debug server(check for null before using)

extern LUX_API Logger Debug;
extern LUX_API Logger Info;
extern LUX_API Logger Warning;
extern LUX_API Logger Error;
extern LUX_API Logger Log;
}
}

#endif
