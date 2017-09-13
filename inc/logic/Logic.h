#ifndef INCLUDED_LOGIC_H
#define INCLUDED_LOGIC_H
#include "core/LuxBase.h"

namespace lux
{
namespace logic
{

class Timer
{
public:
	Timer(float period = 1.0f) :
		m_Time(0.0f)
	{
		SetPeriod(period);
	}

	bool Tick(float secsPassed)
	{
		m_Time += secsPassed;
		if(m_Time >= m_Period) {
			m_Time -= m_Period;
			return true;
		} else {
			return false;
		}
	}

	void Reset()
	{
		m_Time = 0.0f;
	}

	void SetPeriod(float f)
	{
		lxAssert(f > 0.0f);
		m_Period = f;
	}

	float GetPeriod() const
	{
		return m_Period;
	}

private:
	float m_Time;
	float m_Period;
};

class Switch
{
public:
	Switch(bool start = false) :
		m_State(start)
	{
	}

	bool Flip()
	{
		m_State = !m_State;
		return m_State;
	}

	void Set(bool x)
	{
		m_State = x;
	}

	bool Get() const
	{
		return m_State;
	}

	operator bool() const
	{
		return Get();
	}

private:
	bool m_State;
};

} // namespace logic
} // namespace lux

#endif // #ifndef INCLUDED_LOGIC_H