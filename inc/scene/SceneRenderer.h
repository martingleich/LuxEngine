#ifndef INCLUDED_SCENE_RENDERER_H
#define INCLUDED_SCENE_RENDERER_H
#include "core/ReferenceCounted.h"
#include "core/Attributes.h"

#include "scene/Renderable.h"

namespace lux
{
namespace video
{
class Renderer;
class PipelineSettings;
class PipelineOverwrite;
}

namespace scene
{
class Scene;

class SceneRenderer : public ReferenceCounted
{
public:
	virtual ~SceneRenderer() {}

	////////////////////////////////////////////////////////////////////////////////////

	virtual void DrawScene(Scene* scene, bool beginScene = true, bool endScene = true) = 0;

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API core::VariableAccess Attribute(const String& str);
	LUX_API const core::Attributes& Attributes() const;

protected:
	core::Attributes m_Attributes;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_SCENE_RENDERER_H