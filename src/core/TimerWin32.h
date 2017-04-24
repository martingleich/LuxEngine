#ifndef INCLUDED_TIMER_WIN32_H
#define INCLUDED_TIMER_WIN32_H
#include "core/Timer.h"

#ifdef LUX_WINDOWS

namespace lux
{
namespace core
{

class TimerWin32 : public Timer
{
public:
	TimerWin32();
	~TimerWin32()
	{
	}
	DateAndTime GetDateAndTime() const;
	u32 GetRealTime() const;
	u32 GetTime() const;
	void SetTime(u32 time);
	void Stop();
	void start();
	void SetSpeed(float speed);
	float GetSpeed() const;
	bool IsStopped() const;
	void Tick();

private:
	DateAndTime::EWeekDay TimeStructToWeekDay(int tm) const;

private:
	double m_Frequency;
	float m_VirtualTimerSpeed;
	int m_VirtualTimerStopCounter;
	u32 m_LastVirtualTime;
	u32 m_StartRealTime;
	u32 m_StaticTime;

};

}

}

#endif // LUX_WINDOWS


#endif
