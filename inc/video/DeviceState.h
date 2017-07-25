#ifndef INCLUDED_DEVICE_STATE_H
#define INCLUDED_DEVICE_STATE_H
#include "core/LuxBase.h"

namespace lux
{
namespace video
{
class Pass;

class DeviceState
{
public:
	virtual ~DeviceState() {}

	virtual void EnablePass(const Pass& p) = 0;
	virtual void* GetLowLevelDevice() = 0;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_DEVICE_STATE_H