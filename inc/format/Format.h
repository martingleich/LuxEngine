#ifndef INCLUDED_FORMAT_FORMAT_H
#define INCLUDED_FORMAT_FORMAT_H

#include "format/FormatLocale.h"
#include "format/StringType.h"
#include "format/Placeholder.h"
#include "format/Slice.h"
#include "format/Context.h"
#include "format/Sink.h"

//! Contains all format functionality
namespace format
{

/** \addtogroup Formatting
@{
*/

//! Convert a string and write it into a given context
/**
Will throw syntax_exception on bad syntax.<br>
Can throw exception on conversion error, depending on FORMAT_ERROR_TEXT.<br>
Usefull for conv_data implementations.<br>
Will perform normal formatting, but will write the result into a Context.<br>

\param ctx The context where the formatted data is written to
\param fmtStringType The format of the string type, for improved compatibilty a ascii string if best
\param str The format string
\param args The placeholder arguments
*/
template <typename... Types>
void vformat(Context& ctx, StringType fmtStringType, const char* str, const Types&... args);

//! Format a string
/**
\param sink The destination sink
\param dstStringType The string type to write into the sink
\param locale The locale to use while writing, pass NULL too use the default locale
\param sinkFlags A combination of ESinkFlag values
\param fmtStringType The format of the format string
\param str The format string
\param args The placeholder arguments
\return The number of characters written, or -1 on error
\throws format::exception When an error occured an FORMAT_EXCEPTIONS is set
*/
template <typename SinkT, typename... Types>
size_t formatEx(SinkT&& sink, StringType dstStringType, const Locale* locale, int sinkFlags, StringType fmtStringType, const char* str, const Types&... args);

//! Format a string
/**
The output and input types default to FORMAT_STRING_TYPE
Per default the sink is not flushed
\param sink The destination sink
\param str The format string
\param args The placeholder arguments
\return The number of characters written, or -1 on error
\throws format::exception When an error occured an FORMAT_EXCEPTIONS is set
*/
template <typename SinkT, typename... Types>
size_t format(SinkT&& sink, const char* str, const Types&... args);

//! Format a string and write a newline character into the sink and flush the sink
/**
The output and input types default to FORMAT_STRING_TYPE
\param sink The destination sink
\param str The format string
\param args The placeholder arguments
\return The number of characters written, or -1 on error
\throws format::exception When an error occured an FORMAT_EXCEPTIONS is set
*/
template <typename SinkT, typename... Types>
size_t formatln(SinkT&& sink, const char* str, const Types&... args);

/** @}*/
}

#include "Format.inl"

#endif // #ifndef INCLUDED_FORMAT_FORMAT_H
