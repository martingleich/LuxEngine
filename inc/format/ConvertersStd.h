#ifndef INCLUDED_FORMAT_CONVERTERS_STD_H
#define INCLUDED_FORMAT_CONVERTERS_STD_H
#include "Converters.h"
#include <string>

namespace format
{
void conv_data(Context& ctx, const std::string& data, Placeholder& placeholder);
}

#endif // #ifndef INCLUDED_FORMAT_CONVERTERS_STD_H
