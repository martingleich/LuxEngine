#ifndef INCLUDED_LUX_LX_EVENTS_H
#define INCLUDED_LUX_LX_EVENTS_H
#include "math/Vector2.h"

namespace lux
{
namespace core
{
	struct Button
	{
		bool state;
	};

	struct Axis
	{
		float state;
	};

	struct Area
	{
		math::Vector2F state;
	};

	class Event
	{
	public:
		virtual ~Event() {}
	};

} // namespace event
} // namespace lux

#endif // #ifndef INCLUDED_LUX_LX_EVENTS_H
