#ifndef INCLUDED_LUX_SCENE_RENDERABLE_H
#define INCLUDED_LUX_SCENE_RENDERABLE_H
#include "core/lxArray.h"
#include "math/AABBox.h"

namespace lux
{
namespace video
{
class Renderer;
}
namespace scene
{
class Node;

enum class ERenderPass
{
	None,

	SkyBox,

	Transparent,
	Solid,
	DeferredEffect,
	Any,
};

enum class ERenderableTags
{
	None = 0,
	EditView = 1,
};

class Light;
class AbstractCamera;
class SceneData
{
public:
	Node* activeCameraNode;
	AbstractCamera* activeCamera;

	ERenderPass pass;

	const core::Array<Light*> illuminatingLights;
	const core::Array<Light*> shadowCastingLights;

	SceneData(const core::Array<Light*>& ill, const core::Array<Light*>& shadowing) :
		illuminatingLights(ill),
		shadowCastingLights(shadowing)
	{}
};

class Renderable
{
public:
	virtual void Render(Node* node, video::Renderer* renderer, const SceneData& scene) = 0;
	virtual ERenderPass GetRenderPass() const = 0;
	
	virtual const math::AABBoxF& GetBoundingBox() const { return math::AABBoxF::EMPTY; }
};

class RenderableVisitor
{
public:
	virtual ~RenderableVisitor() {}

	virtual void Visit(Node* node, Renderable* r) = 0;
};

} // namespace scene

DECLARE_FLAG_CLASS(scene::ERenderableTags);

} // namespace lux

#endif // #ifndef INCLUDED_LUX_SCENE_RENDERABLE_H