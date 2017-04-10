#pragma once
#include "Context.h"
#include "Placeholder.h"
#include "Exception.h"
#include "StringBasics.h"

namespace format
{

/*
The conv_data function is called to convert a placeholder-argument to a string.
Each conv_data function, must convert it's argument to a string and
add it to the Context.
There can be additional memory allocation.
If a conversion is not possible or invalid, a exception should be thrown.
The passed FormatOptions can be changed to change the behavior of the align operation
following the conv_data call.
*/

void conv_data(Context& ctx, const char* data, Placeholder& placeholder);
void conv_data(Context& ctx, const std::string& data, Placeholder& placeholder);
void conv_data(Context& ctx, intmax_t data, Placeholder& placeholder);
void conv_data(Context& ctx, uintmax_t data, Placeholder& placeholder);
void conv_data(Context& ctx, const void* data, Placeholder& placeholder);
void conv_data(Context& ctx, double data, Placeholder& placeholder);
void conv_data(Context& ctx, bool data, Placeholder& placeholder);
void conv_data(Context& ctx, Cursor* ptr, Placeholder& placeholder);

}