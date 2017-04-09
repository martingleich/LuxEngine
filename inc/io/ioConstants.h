#ifndef INCLUDED_IOCONSTANTS_H
#define INCLUDED_IOCONSTANTS_H
#include "core/EnumClassFlags.h"

namespace lux
{
namespace io
{

//! The mode in which files are opend
enum class EFileMode
{
	Read = 1, //!< Read from the file
	Write = 2, //!< Write to the file
	ReadWrite = Read | Write, //!< Read and write from the file
};

//! Seek origin, in seek operations
enum class ESeekOrigin
{
	Start = 0, //!< from start of the file
	End, //!< from the end of the file(offset negative)
	Cursor, //!< from the current cursor
};

enum class EArchiveCapabilities
{
	Read = 0x1,
	Delete = 0x2,
	Add = 0x4,
	Change = 0x8
};

}

DECLARE_FLAG_CLASS(io::EArchiveCapabilities);
}

#endif // #ifndef INCLUDED_IOCONSTANTS_H
