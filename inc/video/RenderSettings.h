#ifndef INCLUDED_RENDER_SETTINGS_H
#define INCLUDED_RENDER_SETTINGS_H
#include "core/LuxBase.h"

namespace lux
{
namespace core
{
class VariableAccess;
}
namespace video
{
class Material;
class MaterialRenderer;
class PipelineOverwrite;
class RenderTarget;

class RenderSettings
{
public:
	class ParamListAccess
	{
	public:
		virtual core::VariableAccess operator[](u32 id) const = 0;
	};

	RenderSettings(
		const PipelineOverwrite& o,
		const Material& m,
		const ParamListAccess& pa) :
		overwrite(o),
		material(m),
		params(pa)
	{
	}

	const PipelineOverwrite& overwrite;
	const Material& material;
	const ParamListAccess& params;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_RENDER_SETTINGS_H