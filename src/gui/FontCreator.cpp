#include "gui/FontCreator.h"

const lux::core::array<lux::u32> lux::gui::CHARS_GERMAN;

struct FillArrayWithString
{
	FillArrayWithString(const lux::core::array<lux::u32>& dst, const lux::string& src)
	{
		lux::core::array<lux::u32>& d = const_cast<lux::core::array<lux::u32>&>(dst);
		for(auto it = src.First(); it != src.End(); ++it)
			d.Push_Back(*it);
	}
};

static FillArrayWithString CHAR_GERMAN_FILLER(lux::gui::CHARS_GERMAN, " AA«»íéáóúôîûâê1234567890AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZzÖöÜüÄäß²³{}[]()<>+-*,;.:!?&%§/\\'#~^°\"_´`$€@µ|=");
