#ifndef INCLUDED_FORMAT_CONVERTERS_STD_H
#define INCLUDED_FORMAT_CONVERTERS_STD_H
#include "format/Converters.h"
#include <string>

namespace format
{
void fmtPrint(Context& ctx, const std::string& data, const Placeholder& placeholder);
}

#endif // #ifndef INCLUDED_FORMAT_CONVERTERS_STD_H
