#pragma once

#define FORMAT_WINDOWS

#undef FORMAT_NO_EXCEPTIONS
#define FORMAT_ERROR_TEXT
#define FORMAT_STRING_TYPE Unicode
// TODO: This should set the default string-type when passing char*, add wrapper types around char to use other string types.
