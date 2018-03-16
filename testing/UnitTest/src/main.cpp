#include "stdafx.h"
#include <signal.h>

using namespace UnitTesting;

class NameFilter : public UnitTesting::Filter
{
public:
	NameFilter(std::vector<std::string> v, bool including) :
		m_Names(v),
		m_Including(including)
	{
	}

	static NameFilter Include(std::vector<std::string> v)
	{
		return NameFilter(v, true);
	}

	static NameFilter Exclude(std::vector<std::string> v)
	{
		return NameFilter(v, false);
	}

	virtual bool IsOK(const UnitTesting::Suite& suite)
	{
		auto it = std::find(m_Names.begin(), m_Names.end(), suite.GetInfo().GetName());
		bool found = (it != m_Names.end());

		if((m_Including && found) || (!m_Including && !found))
			return true;
		else
			return false;
	}

	virtual bool IsOK(const UnitTesting::Test& test)
	{
		return true;
	}

private:
	std::vector<std::string> m_Names;
	bool m_Including;
};

class ProjectPrinter : public UnitTesting::ConsoleCallback
{
private:
	size_t& m_Succeded;
	size_t& m_Failed;

public:
	ProjectPrinter(size_t& succeeded, size_t& failed) :
		m_Succeded(succeeded),
		m_Failed(failed)
	{
	}
	virtual void OnTestBegin(Test& t)
	{
	}

	virtual bool OnTestEnd(const TestResult& t)
	{
		if(t.GetTotalResult() == Result::Success) {
			++m_Succeded;
		} else if(t.GetTotalResult() == Result::Fail) {
			++m_Failed;
			std::cout << t.GetTest().GetSuite().GetInfo().GetName() << ":" << t.GetTest().GetInfo().GetName() << " --> Failed." << std::endl;
		}

		if(t.GetTotalResult() == Result::Fail) {
			for(size_t i = 0; i < t.GetAssertCount(); ++i) {
				const AssertResult& a = t.GetAssert(i);
				std::cout << "     \"" << a.message << "\" --> ";
				if(a.GetTotalResult() == Result::Success)
					std::cout << "Succeeded.";
				else if(a.GetTotalResult() == Result::Fail)
					std::cout << "Failed.";

				std::cout << std::endl;
			}
		}

		return false;
	}

	virtual void OnSuiteBegin(Suite& s)
	{
	}

	virtual void OnSuiteEnd(const SuiteResult& result)
	{
	}

	virtual void OnBegin(Environment& env)
	{
		m_Succeded = 0;
		m_Failed = 0;
		std::cout << "Start unit tests." << std::endl;
	}

	virtual void OnEnd(const EnvironmentResult& result)
	{
		std::cout << "Finished unit tests." << std::endl;
		std::cout << "====== " << m_Succeded << " succeeded, " << m_Failed << " failed =======" << std::endl;
	}
};

void SignalHandler(int signal)
{
	throw "Access violation";
}

int main(int argc, const char* argv[])
{
	bool isProject = false;
	if(argc > 1) {
		if(strcmp(argv[1], "-project") == 0) {
			isProject = true;
		}
	}

	size_t succeded = 0, failed = 0;
	if(isProject)
		UnitTesting::Environment::Instance().SetControl(new ProjectPrinter(succeded, failed));

#ifdef _DEBUG
	// This is pure evil.
	// We can maybe catch the first exception, after that the stack is most likly fucked up.
	// But one message is better than nothing.
	typedef void(*SignalHandlerPtr)(int);
	SignalHandlerPtr previous = signal(SIGSEGV, SignalHandler);
#endif

	UnitTesting::Environment::Instance().Run();

	std::cin.get();

	return failed ? -1 : 0;
}
