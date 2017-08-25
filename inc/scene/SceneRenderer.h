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

	virtual void PushPipelineOverwrite(ERenderPass pass, const video::PipelineOverwrite& over) = 0;
	virtual void PopPipelineOverwrite(ERenderPass pass) = 0;

	////////////////////////////////////////////////////////////////////////////////////

	virtual core::VariableAccess Attribute(const String& str) = 0;
	virtual const core::Attributes& Attributes() const = 0;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_SCENE_RENDERER_H