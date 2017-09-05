#ifndef INCLUDED_CLOCK_H
#define INCLUDED_CLOCK_H
#include "LuxBase.h"
#include "DateAndTime.h"

namespace lux
{
namespace core
{

//! Engine clock to retrieve the current time and date.
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