#pragma once

#if true
#define FORMAT_WINDOWS
#endif

#if false
#define FORMAT_LINUX
#endif

#undef FORMAT_NO_EXCEPTIONS
#define FORMAT_ERROR_TEXT
#define FORMAT_STRING_TYPE Unicode
