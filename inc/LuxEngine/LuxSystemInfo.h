#ifndef INCLUDED_LUX_SYSTEM_INFO_H
#define INCLUDED_LUX_SYSTEM_INFO_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "math/Dimension2.h"

namespace lux
{

//! Class to query information about the system.
class LuxSystemInfo : public ReferenceCounted
{
public:
	virtual ~LuxSystemInfo() {}

	virtual bool GetProcessorName(core::String& displayName) = 0;
	virtual bool GetProcessorSpeed(u32& speedInMhz) = 0;

	virtual bool GetTotalRAM(u32& ramInMB) = 0;
	virtual bool GetAvailableRAM(u32& ramInMB) = 0;

	virtual bool GetPrimaryScreenResolution(math::Dimension2U& dimension) = 0;
};

} // namespace lux

#endif // #ifndef INCLUDED_LUX_SYSTEM_INFO_H