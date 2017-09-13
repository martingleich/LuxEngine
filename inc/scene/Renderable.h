#ifndef INCLUDED_SCENE_RENDERABLE_H
#define INCLUDED_SCENE_RENDERABLE_H
#include "video/Material.h"
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

class Light;
class Camera;
class SceneData
{
public:
	Node* activeCameraNode;
	Camera* activeCamera;

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
	
	virtual const math::AABBoxF& GetBoundingBox() const = 0;
};

class RenderableVisitor
{
public:
	virtual ~RenderableVisitor() {}

	virtual void Visit(Node* node, Renderable* r) = 0;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_SCENE_RENDERABLE_H