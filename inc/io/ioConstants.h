#ifndef INCLUDED_LUX_IOCONSTANTS_H
#define INCLUDED_LUX_IOCONSTANTS_H
#include "core/EnumClassFlags.h"

namespace lux
{
namespace io
{

//! The mode in which files are opend
enum class EFileModeFlag
{
	Read = 1, //!< Read from the file
	Write = 2, //!< Write to the file
	ReadWrite = Read | Write, //!< Read and write from the file
};

//! Seek origin, in seek operations
enum class ESeekOrigin
{
	Start = 0, //!< from start of the file
	Cursor, //!< from the current cursor
};

enum class EArchiveCapFlag
{
	Read = 0x1,
	Delete = 0x2,
	Add = 0x4,
	Change = 0x8
};

enum class EVirtualCreateFlag
{
	None = 0,
	Copy = 1,
	ReadOnly = 2,
	DeleteOnDrop = 4,
	Expandable = 8,
};

enum class ELineEnding
{
	Unix,
	Windows,
	Macintosh,

	Unknown,
};

} // namespace io
} // namespace lux

#endif // #ifndef INCLUDED_LUX_IOCONSTANTS_H
