#ifndef INCLUDED_RENDER_SETTINGS_H
#define INCLUDED_RENDER_SETTINGS_H
#include "core/LuxBase.h"

namespace lux
{
namespace video
{
class Material;

class RenderSettings
{
public:
	RenderSettings(
		const Material& m) :
		material(m)
	{
	}

	const Material& material;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_RENDER_SETTINGS_H