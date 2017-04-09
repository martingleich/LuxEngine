#include "TimerWin32.h"
#include "StrippedWindows.h"
#include <time.h>

namespace lux
{
namespace core
{

TimerWin32::TimerWin32() :
	m_VirtualTimerSpeed(1.0f),
	m_VirtualTimerStopCounter(0),
	m_LastVirtualTime(0)
{
	LONGLONG frequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)(&frequency));
	m_Frequency = 1000.0 / (double)(frequency);

	m_StartRealTime = m_StaticTime = GetRealTime();
}

DateAndTime TimerWin32::GetDateAndTime() const
{
	time_t secondsSince1970;
	time(&secondsSince1970);
	tm timeStruct;
	localtime_s(&timeStruct, &secondsSince1970);
	DateAndTime out;
	out.dayOfMonth = timeStruct.tm_mday;
	out.hours = timeStruct.tm_hour;
	out.isDayLightSaving = (timeStruct.tm_isdst != 0);
	out.minutes = timeStruct.tm_min;
	out.month = timeStruct.tm_mon + 1;
	out.seconds = timeStruct.tm_sec;
	out.weekDay = TimeStructToWeekDay(timeStruct.tm_wday);
	out.year = timeStruct.tm_year + 1900;

	return out;
}

inline u32 TimerWin32::GetRealTime() const
{
	LONGLONG time;
	QueryPerformanceCounter((LARGE_INTEGER*)(&time));
	return (u32)((double)(time)* m_Frequency);
}

inline u32 TimerWin32::GetTime() const
{
	if(IsStopped())
		return m_LastVirtualTime;

	return m_LastVirtualTime + (u32)((m_StaticTime - m_StartRealTime) * m_VirtualTimerSpeed);
}

inline void TimerWin32::SetTime(u32 time)
{
	m_StaticTime = GetRealTime();
	m_LastVirtualTime = time;
	m_StartRealTime = m_StaticTime;
}

inline void TimerWin32::Stop()
{
	if(!IsStopped())
		m_LastVirtualTime = GetTime();

	--m_VirtualTimerStopCounter;
}

inline void TimerWin32::start()
{
	++m_VirtualTimerStopCounter;

	if(!IsStopped())
		SetTime(m_LastVirtualTime);
}

inline void TimerWin32::SetSpeed(float speed)
{
	SetTime(GetTime());

	m_VirtualTimerSpeed = speed;

	if(m_VirtualTimerSpeed < 0.0f)
		m_VirtualTimerSpeed = 0.0f;
}

inline float TimerWin32::GetSpeed() const
{
	return m_VirtualTimerSpeed;
}

inline bool TimerWin32::IsStopped() const
{
	return (m_VirtualTimerStopCounter < 0);
}

inline void TimerWin32::Tick()
{
	m_StaticTime = GetRealTime();
}

DateAndTime::EWeekDay TimerWin32::TimeStructToWeekDay(int tm) const
{
	switch(tm) {
	case 0:
		return DateAndTime::EWeekDay::Sunday;
	case 1:
		return DateAndTime::EWeekDay::Monday;
	case 2:
		return DateAndTime::EWeekDay::Tuesday;
	case 3:
		return DateAndTime::EWeekDay::Wednesday;
	case 4:
		return DateAndTime::EWeekDay::Thursday;
	case 5:
		return DateAndTime::EWeekDay::Friday;
	case 6:
		return DateAndTime::EWeekDay::Saturday;
	default:
		return DateAndTime::EWeekDay::Sunday;
	}
}

}    

}    

