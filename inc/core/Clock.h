#ifndef INCLUDED_CLOCK_H
#define INCLUDED_CLOCK_H
#include "LuxBase.h"
#include "DateAndTime.h"

namespace lux
{
namespace core
{

//! Engine clock retrieve the current time and date.
namespace Clock
{
	//! Retrieve the current date and time
	LUX_API DateAndTime GetDateAndTime();

	//! Retrieve the current tick count.
	LUX_API u64 GetTicks();

	LUX_API u64 TicksPerSecond();
};

}
}

#endif