#ifndef INCLUDED_DATE_AND_TIME_H
#define INCLUDED_DATE_AND_TIME_H
#include "LuxBase.h"
#include "lxFormat.h"

namespace lux
{
namespace core
{

//! Date and Time
struct DateAndTime
{
	//! The days of week
	enum class EWeekDay
	{
		Sunday = 0,
		Monday,
		Tuesday,
		Wednesday,
		Thursday,
		Friday,
		Saturday,
	};

	u32 milliseconds;      //!< Milliseconds - [0, 999]
	u32 seconds;           //!< Seconds - [0,59] 
	u32 minutes;           //!< Minutes - [0,59] 
	u32 hours;             //!< Hours - [0,23] 
	u32 dayOfMonth;        //!< Day of month - [1,31] 
	u32 month;             //!< month - [1,12] 
	u32 year;              //!< Year
	EWeekDay weekDay;      //!< Day of week 
	bool isDayLightSaving; //!< Is daylightsaving

	bool IsValid() const
	{
		return !(
			seconds >= 60 ||
			minutes >= 60 ||
			hours >= 24 ||
			dayOfMonth == 0 || dayOfMonth >= 32 ||
			month == 0 || month >= 13);
	}
};

inline void fmtPrint(format::Context& ctx, const DateAndTime& date, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);

	static const char wday_name[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	static const char mon_name[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	if(!date.IsValid()) {
		ctx.AddTerminatedSlice("[invalid date]");
	} else {
		format::vformat(ctx,
			"~s ~s ~d ~.2d:~.2d:~.2d ~d",
			wday_name[(int)date.weekDay],
			mon_name[date.month - 1],
			date.dayOfMonth, date.hours,
			date.minutes, date.seconds,
			date.year);
	}
}

}

}


#endif // #ifndef INCLUDED_DATE_AND_TIME_H
