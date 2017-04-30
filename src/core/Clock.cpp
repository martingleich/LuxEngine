#include "core/Clock.h"
#include <chrono>

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

u64 Clock::GetTicks()
{
	auto time = std::chrono::high_resolution_clock::now();
	return (time.time_since_epoch() / 1000000).count();
}

}
}