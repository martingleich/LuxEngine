#ifndef INCLUDED_RENDER_SETTINGS_H
#define INCLUDED_RENDER_SETTINGS_H
#include "core/LuxBase.h"

namespace lux
{
namespace video
{
class Material;
class PipelineOverwrite;

class RenderSettings
{
public:
	RenderSettings(
		const PipelineOverwrite& o,
		const Material& m) :
		overwrite(o),
		material(m)
	{
	}

	const PipelineOverwrite& overwrite;
	const Material& material;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_RENDER_SETTINGS_H