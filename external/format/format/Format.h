#pragma once
#include "FormatConfig.h"
#include "Locale.h"
#include "StringType.h"
#include "FormatMagicTemplates.h"
#include "Placeholder.h"
#include "Slice.h"
#include "Context.h"

namespace format
{
// Format string into Context
/**
Will throw syntax_exception on bad syntax.
Can throw exception on conversion error, depending on FORMAT_ERROR_TEXT.
Usefull conv_data implementations.
Will perform normal formatting, but will write the result into a Context.
*/
template <typename... Types>
void vformat(Context& ctx, StringType fmtStringType, const char* str, const Types&... args);

template <typename SinkT, typename... Types>
size_t formatEx(SinkT&& sink, StringType dstStringType, const locale::Locale* locale, int sinkFlags, StringType fmtStringType, const char* str, const Types&... args);

template <typename SinkT, typename... Types>
size_t format(SinkT&& sink, const char* str, const Types&... args);

template <typename SinkT, typename... Types>
size_t formatln(SinkT&& sink, const char* str, const Types&... args);

}

#include "Format.inl"
