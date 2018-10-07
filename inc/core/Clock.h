#ifndef INCLUDED_LUX_CLOCK_H
#define INCLUDED_LUX_CLOCK_H
#include "LuxBase.h"
#include "core/DateAndTime.h"

namespace lux
{
namespace core
{

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
	LUX_API u64 GetTicks();

	//! Get the number of ticks per second.
	LUX_API u64 TicksPerSecond();
} //! namespace Clock

} // namespace core
} // namespace lux

#endif