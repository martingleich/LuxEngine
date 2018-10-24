#ifndef INCLUDED_LUX_CLOCK_H
#define INCLUDED_LUX_CLOCK_H
#include "LuxBase.h"
#include "core/DateAndTime.h"
#include "core/lxSignal.h"
#include <chrono>

// Can be put into the global namespace without problem, since all names are reserved identifers.
using namespace std::chrono_literals;

namespace lux
{
namespace core
{

class Duration
{
public:
	using BaseType = long long;
	static constexpr BaseType CountPerMicro  = 1;
	static constexpr BaseType CountPerMilli  = CountPerMicro*1000;
	static constexpr BaseType CountPerSecond = CountPerMilli*1000;
	static constexpr BaseType CountPerMinute = CountPerSecond*60;

public:
	constexpr Duration() :
		m_Count(0)
	{}
	constexpr explicit Duration(BaseType count) :
		m_Count(count)
	{
	}
	template <typename BT, typename R>
	constexpr Duration(std::chrono::duration<BT, R> count) :
		m_Count((BaseType)((count.count()*R::num*CountPerSecond)/R::den))
	{
	}
	constexpr static Duration Micros(BaseType micros) { return Duration(micros*CountPerMicro); }
	constexpr static Duration Micros(float micros) { return Duration(((BaseType)micros)*CountPerMicro); }
	constexpr static Duration Seconds(BaseType seconds) { return Duration(seconds*CountPerSecond); }
	constexpr static Duration Seconds(float seconds) { return Duration((BaseType)(seconds*CountPerSecond)); }
	constexpr static Duration Minutes(BaseType minutes) { return Duration(minutes*CountPerMinute); }
	constexpr static Duration Minutes(float minutes) { return Duration((BaseType)(minutes*CountPerMinute)); }

	Duration& operator+=(Duration other) { m_Count += other.m_Count; return *this; }
	constexpr Duration operator+(Duration other) const { return Duration(m_Count + other.m_Count); }
	Duration& operator-=(Duration other) { m_Count -= other.m_Count; return *this; }
	constexpr Duration operator-(Duration other) const { return Duration(m_Count - other.m_Count); }
	constexpr Duration& operator/=(BaseType other) { m_Count /= other; return *this; }
	constexpr Duration operator/(BaseType other) const { return Duration(m_Count/other); }

	void SubtractOrZero(Duration other)
	{
		m_Count -= other.m_Count;
		if(m_Count < 0)
			m_Count = 0;
	}

	bool operator==(Duration other) const { return m_Count == other.m_Count; }
	bool operator!=(Duration other) const { return m_Count != other.m_Count; }
	bool operator<(Duration other) const { return m_Count < other.m_Count; }
	bool operator>(Duration other) const { return m_Count > other.m_Count; }
	bool operator<=(Duration other) const { return m_Count <= other.m_Count; }
	bool operator>=(Duration other) const { return m_Count >= other.m_Count; }

	float AsSeconds() const { return (float)m_Count/(float)CountPerSecond; }

	BaseType Count() const { return m_Count; }
	std::chrono::duration<BaseType, std::micro> AsStdDuration() const { return std::chrono::duration<BaseType, std::micro>(m_Count); }

private:
	BaseType m_Count;
};

//! System clock to retrieve the current time and date.
/**
This function can be used to access the walltime. They should never
be used inside the scenemanager or to control gameplay, since the
syncronisation problems might occur. Use the (TODO: Write timer classes)
instance of the scenemanger of the game for this.
*/
namespace Clock
{
	//! Retrieve the current date and time.
	LUX_API DateAndTime GetDateAndTime();

	//! Retrieve the current tick count.
	LUX_API Duration GetTicks();
} //! namespace Clock


// TODO: Would a timer based on integers be better.
// All the times are in seconds.
// TODO: Maybe a type for times should be used(eiter std lib or something homebrewn)
// The whole time handling in the engine core should be improved.
struct TimerSettings
{
	Duration period;
	Duration start;
	int count;

	TimerSettings() {}
	explicit TimerSettings(Duration p, int c = 0) :
		period(p),
		start(p),
		count(c)
	{
	}

	TimerSettings(Duration p, Duration s, int c) :
		period(p),
		start(s),
		count(c)
	{
	}
};

/*
I reimplemented the linked list instead of using core::List because
it is nicer to use pointers instead of iterators and the implementation is
really easy.
*/
struct InternalTimer
{
	core::Signal<> event;
	Duration remain;
	Duration period;
	int repeatCount;
	bool paused;

	InternalTimer* next;
	InternalTimer* prev;
};

class Timer : private Uncopyable
{
	// Only the timer manager can create timers.
	friend class TimerManager;
	explicit Timer(InternalTimer* internal) :
		m_Timer(internal)
	{
	}

public:
	Timer() : m_Timer(nullptr) {}
	Timer(Timer&& old) :
		m_Timer(old.m_Timer)
	{
		old.m_Timer = nullptr;
	}
	Timer& operator=(Timer&& old)
	{
		this->~Timer();
		m_Timer = old.m_Timer;
		old.m_Timer = nullptr;
		return *this;
	}
	~Timer()
	{
		if(m_Timer)
			Kill();
	}

	void Kill()
	{
		m_Timer->period = Duration(std::numeric_limits<Duration::BaseType>::max());
		m_Timer = nullptr;
	}

	void Pause(bool b = true) { m_Timer->paused = b; }
	void Resume(bool b = true) { m_Timer->paused = !b; }
	bool IsPaused() const { return m_Timer->paused; }
	bool IsValid() const { return m_Timer != nullptr; }
	Signal<>& Event() { return m_Timer->event; }
	Duration GetRemainingTime() const { return m_Timer->remain; }
	Duration GetElapsedTime() const { return m_Timer->period - m_Timer->remain; }

private:
	InternalTimer* m_Timer;
};

class TimerManager : private Uncopyable
{
public:
	TimerManager() :
		m_FirstTimer(nullptr),
		m_TimerCount(0)
	{
	}
	TimerManager(TimerManager&& old) :
		m_FirstTimer(old.m_FirstTimer),
		m_TimerCount(old.m_TimerCount)
	{
		old.m_FirstTimer = nullptr;
		old.m_TimerCount = 0;
	}
	LUX_API ~TimerManager();

	TimerManager& operator=(TimerManager&& old)
	{
		this->~TimerManager();
		m_FirstTimer = old.m_FirstTimer;
		m_TimerCount = old.m_TimerCount;
		old.m_FirstTimer = nullptr;
		old.m_TimerCount = 0;
		return *this;
	}

	LUX_API void Tick(Duration passed)
	{
		for(InternalTimer* it = m_FirstTimer; it != nullptr;) {
			auto& t = *it;
			bool deleteTimer = (t.period.Count() == std::numeric_limits<Duration::BaseType>::max());
			if(!t.paused) {
				if(t.remain > passed) {
					t.remain -= passed;
				} else {
					t.remain = Duration(0);
				}
				if(t.remain.Count() == 0 && !deleteTimer) {
					t.event.Broadcast();
					// Maybe the timer was deleted in the broadcast
					deleteTimer = (t.period.Count() == std::numeric_limits<Duration::BaseType>::max());
					t.remain = t.period;
					if(t.repeatCount > 0) {
						t.repeatCount--;
						if(t.repeatCount == 0)
							deleteTimer = true;
					}
				}
			}
			if(deleteTimer)
				it = DestroyInternalTimer(it);
			else
				it = it->next;
		}
	}

	LUX_API void RunLoop(Duration stepSize);
	void AbortLoop() { m_AbortLoop = true; }

	LUX_API Timer CreateTimer(const TimerSettings& settings);
	LUX_API Timer CreateTimer(Duration period);

	int GetTimerCount() const { return m_TimerCount; }

private:
	InternalTimer* CreateInternalTimer(const TimerSettings& settings);
	InternalTimer* DestroyInternalTimer(InternalTimer* t);

private:
	InternalTimer* m_FirstTimer;
	int m_TimerCount;
	bool m_AbortLoop;
};

} // namespace core
} // namespace lux

#endif