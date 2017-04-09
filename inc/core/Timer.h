#ifndef INCLUDED_TIMER_H
#define INCLUDED_TIMER_H
#include "ReferenceCounted.h"
#include "DateAndTime.h"

namespace lux
{
namespace core
{

//! Timer object
/**
Retrieve the current time
or just get a stopwatch
*/
class Timer : public ReferenceCounted
{
public:
	//! Destructor
	virtual ~Timer()
	{
	}

	//! Retrieve the current date and time
	/*
	\return The current date and time
	*/
	virtual DateAndTime GetDateAndTime() const = 0;

	//! The current realtime
	/**
	\return the current realtime in milliseconds
	*/
	virtual u32 GetRealTime() const = 0;

	//! The current virtual time
	/**
	The virtual time goes at another rate than realtime
	\return The current virtual time
	*/
	virtual u32 GetTime() const = 0;


	//! Set the virtual time
	/**
	The virtual time goes at another rate than realtime
	\param time The new virtual time
	*/
	virtual void SetTime(u32 time) = 0;

	//! Stops the virtual time
	virtual void Stop() = 0;

	//! Procced the virtual time
	virtual void start() = 0;

	//! Set the speed of the virtual time
	/**
	\param speed The speed of virtual time in relation to realtime
	*/
	virtual void SetSpeed(float speed) = 0;

	//! Get the speed of virtual time
	/**
	\return The speed of virtual time in relation to realtime
	*/
	virtual float GetSpeed() const = 0;

	//! Is the virtual timer running
	/**
	\return runs the virtual timer
	*/
	virtual bool IsStopped() const = 0;

	//! Update the virtual time
	/**
	Is called automatic for the engine timer
	*/
	virtual void Tick() = 0;
};

}    

}    


#endif