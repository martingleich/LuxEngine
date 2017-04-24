#ifdef LUX_LINUX
#include "LuxDeviceLinux.h"
#include "core/Logger.h"

namespace lux
{
LUX_API StrongRef<LuxDevice> CreateDevice()
{
	log::Error("Linux LuxDevice is not available");
	return nullptr;
}
}

#endif // LUX_LINUX
