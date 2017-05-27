#ifndef INCLUDED_RENDER_SETTINGS_H
#define INCLUDED_RENDER_SETTINGS_H
#include "core/LuxBase.h"

namespace lux
{
namespace core
{
class PackageParam;
}
namespace video
{
class Material;
class PipelineSettings;
class RenderTarget;

class RenderSettings
{
public:
	class ParamListAccess
	{
	public:
		virtual core::PackageParam operator[](u32 id) const = 0;
	};

	RenderSettings(const Material& m, const PipelineSettings& ps, const ParamListAccess& pa) :
		material(m),
		pipeline(ps),
		params(pa)
	{
	}

	const Material& material;
	const PipelineSettings& pipeline;
	const ParamListAccess& params;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_RENDER_SETTINGS_H