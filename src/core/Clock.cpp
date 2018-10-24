#include "core/Clock.h"
#include <chrono>
#include <thread>

namespace lux
{
namespace core
{

static DateAndTime::EWeekDay TimeStructToWeekDay(int tm)
{
	switch(tm) {
	case 0: return DateAndTime::EWeekDay::Sunday;
	case 1: return DateAndTime::EWeekDay::Monday;
	case 2: return DateAndTime::EWeekDay::Tuesday;
	case 3: return DateAndTime::EWeekDay::Wednesday;
	case 4: return DateAndTime::EWeekDay::Thursday;
	case 5: return DateAndTime::EWeekDay::Friday;
	case 6: return DateAndTime::EWeekDay::Saturday;
	default: return DateAndTime::EWeekDay::Sunday;
	}
}

DateAndTime Clock::GetDateAndTime()
{
	auto now = std::chrono::system_clock::now();
	time_t secondsSince1970 = std::chrono::system_clock::to_time_t(now);
	time(&secondsSince1970);
	tm* timeStruct = localtime(&secondsSince1970);
	DateAndTime out;
	out.dayOfMonth = timeStruct->tm_mday;
	out.hours = timeStruct->tm_hour;
	out.isDayLightSaving = (timeStruct->tm_isdst != 0);
	out.minutes = timeStruct->tm_min;
	out.month = timeStruct->tm_mon + 1;
	out.seconds = timeStruct->tm_sec;
	out.milliseconds = std::chrono::duration_cast<std::chrono::duration<u64, std::ratio<1, 1000>>>(now.time_since_epoch()).count() % 1000;
	out.weekDay = TimeStructToWeekDay(timeStruct->tm_wday);
	out.year = timeStruct->tm_year + 1900;

	return out;
}

Duration Clock::GetTicks()
{
	return Duration(std::chrono::high_resolution_clock::now().time_since_epoch());
}

TimerManager::~TimerManager()
{
	while(m_FirstTimer) {
		auto next = m_FirstTimer->next;
		delete m_FirstTimer;
		m_FirstTimer = next;
	}
}

void TimerManager::RunLoop(Duration stepSize)
{
	m_AbortLoop = false;
	Duration passed;
	auto start = Clock::GetTicks();
	Duration waitTime;
	while(true) {
		start = Clock::GetTicks();

		waitTime += (stepSize - passed) / 2;

		// Take a step.
		if(stepSize < passed) {
			passed -= stepSize;
			Tick(stepSize);
		}
		// Abort if there are no more timers.
		if(!m_FirstTimer || m_AbortLoop)
			return;

		// Wait.
		if(waitTime.Count() > 0)
			std::this_thread::sleep_for(waitTime.AsStdDuration());

		Duration end = Clock::GetTicks();
		passed += end - start;
		start = end;
	}
}

Timer TimerManager::CreateTimer(const TimerSettings& settings)
{
	return Timer(CreateInternalTimer(settings));
}

Timer TimerManager::CreateTimer(Duration period)
{
	TimerSettings settings(period);
	return Timer(CreateInternalTimer(settings));
}

InternalTimer* TimerManager::CreateInternalTimer(const TimerSettings& settings)
{
	auto t = new InternalTimer;
	t->repeatCount = settings.count;
	t->period = settings.period;
	t->remain = settings.start;
	t->paused = false;
	t->next = m_FirstTimer;
	t->prev = nullptr;
	m_FirstTimer = t;
	++m_TimerCount;
	return m_FirstTimer;
}

InternalTimer* TimerManager::DestroyInternalTimer(InternalTimer* t)
{
	auto next = t->next;
	if(t->next)
		t->next->prev = t->prev;
	if(t->prev)
		t->prev->next = t->next;
	if(t == m_FirstTimer)
		m_FirstTimer = nullptr;
	--m_TimerCount;
	delete t;
	return next;
}

} // namespace core
} // namespce lux