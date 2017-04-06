#include "Locale.h"
#include <atomic>

namespace format
{
namespace locale
{
FacetType NumericalFormat;
FacetType BooleanFormat;

Facet_NumericalFormat NumericalFormat_English("\'", ".", "+", "-", "nan", "inf");
Facet_NumericalFormat NumericalFormat_German(".", ",", "+", "-", "nan", "inf");

Facet_BooleanFormat BooleanFormat_English(Facet_BooleanFormat::EnglishGetter);
Facet_BooleanFormat BooleanFormat_German(Facet_BooleanFormat::GermanGetter);
Facet_BooleanFormat BooleanFormat_Digit(Facet_BooleanFormat::DigitGetter);

Locale English(NumericalFormat_English, BooleanFormat_English);
Locale German(NumericalFormat_German, BooleanFormat_German);

locale::Locale* g_Locale = &format::locale::English;

void SetLocale(locale::Locale* l)
{
	g_Locale = l;
}

locale::Locale* GetLocale()
{
	return g_Locale;
}

}
}