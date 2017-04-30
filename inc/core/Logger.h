#ifndef INCLUDED_LOGGER_H
#define INCLUDED_LOGGER_H
#include "StringConverter.h"
#include "lxAlgorithm.h"
#include "core/lxArray.h"
#include "core/lxName.h"
#include "math/lxMathPrinter.h"
#include "core/lxFormat.h"

#include <mutex>
#include <condition_variable>

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
class Printer
{
public:
	struct Settings
	{
		Settings() :
			ShowTime(true)
		{}

		virtual ~Settings() {}

		bool ShowTime;
	};

public:
	Printer() : m_RefCount(1), m_IsInit(false), m_ContextLevel(ELogLevel::Warning)
	{
	}

	virtual ~Printer() {}

	void SetContextLevel(ELogLevel Level)
	{
		m_ContextLevel = Level;
	}

	ELogLevel GetContextLevel() const
	{
		return m_ContextLevel;
	}

	virtual void SetSettings(const Settings& data) { LUX_UNUSED(data); }
	virtual bool Init()
	{
		m_IsInit = true;
		return true;
	}

	virtual void Exit()
	{
		m_IsInit = false;
	}

	bool IsInit()
	{
		return m_IsInit;
	}

	void Drop()
	{
		--m_RefCount;
		if(m_RefCount == 0)
			Exit();
	}
	void Grab()
	{
		++m_RefCount;
	}

	virtual void Print(const string& s, ELogLevel ll) = 0;

	void FinishEntry(const string& s, ELogLevel ll)
	{
		m_PrinterLock.lock();
		Print(s, ll);
		m_PrinterLock.unlock();
	}

private:
	std::mutex m_PrinterLock;
	int m_RefCount;
	bool m_IsInit;
	ELogLevel m_ContextLevel;
};

class Logger;

class LogSystem
{
private:
	core::array<Logger*> m_Loggers;
	ELogLevel m_LogLevel;

public:
	LogSystem(ELogLevel ll = ELogLevel::Info) : m_LogLevel(ll)
	{
	}
	void AddLogger(Logger* l)
	{
		m_Loggers.Push_Back(l);
	}

	void RemoveLogger(Logger* l)
	{
		core::Remove(m_Loggers.First(), m_Loggers.End(), l);
	}

	void SetLogLevel(ELogLevel ll)
	{
		m_LogLevel = ll;
	}

	ELogLevel GetLogLevel() const
	{
		return m_LogLevel;
	}

	LUX_API void Exit();
	LUX_API bool HasUnsetLogs() const;
	LUX_API void SetNewPrinter(Printer* p, bool OnlyIfNULL = false);
};

class Logger
{
private:
	Printer* m_Printer;

	const ELogLevel m_MyLogLevel;
	LogSystem& m_LogSystem;

public:
	Logger(LogSystem& LogSystem, ELogLevel ll, Printer* printer = nullptr) :
		m_MyLogLevel(ll),
		m_LogSystem(LogSystem),
		m_Printer(printer)
	{
		m_LogSystem.AddLogger(this);
	}

	Logger(const Logger& l) = delete;
	Logger(Logger&& l) = delete;

	~Logger()
	{
		m_LogSystem.RemoveLogger(this);
		if(m_Printer)
			m_Printer->Drop();
		m_Printer = nullptr;
	}

	void SetNewPrinter(Printer* p)
	{
		if(m_Printer)
			m_Printer->Drop();
		m_Printer = p;
		if(m_Printer)
			m_Printer->Grab();
	}

	Printer* GetCurrentPrinter()
	{
		return m_Printer;
	}

	template <typename... T>
	void Write(const char* format, T... data)
	{
		if(!m_Printer)
			return;

		if(m_LogSystem.GetLogLevel() <= m_MyLogLevel) {
			ifconst(sizeof...(data)) {
				string out;
				core::string_sink sink(out);
				format::format(sink, format, data...);

				m_Printer->Print(out, m_MyLogLevel);
			} else {
				m_Printer->Print(format, m_MyLogLevel);
			}
		}
	}

	template <typename... T>
	void operator()(const char* format, T... data)
	{
		Write(format, data...);
	}

	template <typename... T>
	void operator()(const string& format, T... data)
	{
		Write(format.Data(), data...);
	}
};

struct HTMLPrinterSettings : public Printer::Settings
{
	HTMLPrinterSettings(const io::path& p) :
		FilePath(p)
	{
	}

	io::path FilePath;
};

struct FilePrinterSettings : public Printer::Settings
{
	FilePrinterSettings(const io::path& p) :
		FilePath(p)
	{
	}

	io::path FilePath;
};

struct ConsolePrinterSettings : public Printer::Settings
{
};

extern LUX_API Printer* HTMLPrinter; //!< Prints the whole log into a html file
extern LUX_API Printer* FilePrinter; //!< Prints the whole log into a simple text file
extern LUX_API Printer* ConsolePrinter; //!< Prints the whole log to stdout

#ifdef LUX_WINDOWS
extern LUX_API Printer* Win32DebugPrinter;
#endif

extern LUX_API LogSystem EngineLog;
extern LUX_API Logger Debug;
extern LUX_API Logger Info;
extern LUX_API Logger Warning;
extern LUX_API Logger Error;
extern LUX_API Logger Log;
}
}

#endif
