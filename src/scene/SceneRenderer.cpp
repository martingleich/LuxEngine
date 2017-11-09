#include "scene/SceneRenderer.h"

namespace lux
{
namespace scene
{

core::VariableAccess SceneRenderer::Attribute(const core::String& str)
{
	return m_Attributes[str];
}

const core::Attributes& SceneRenderer::Attributes() const
{
	return m_Attributes;
}

} // namespace scene
} // namespace lux