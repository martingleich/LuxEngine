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
	SolidAndTransparent,
};

class Light;
class SceneData
{
public:
	ERenderPass pass;

	struct LightEntry
	{
		LightEntry(Light* l, Node* n) :
			light(l),
			node(n)
		{}
		Light* light;
		Node* node;
	};
	const core::Array<LightEntry>& illuminatingLights;
	const core::Array<LightEntry>& shadowCastingLights;

	SceneData(const core::Array<LightEntry>& ill, const core::Array<LightEntry>& shadowing) :
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

	virtual void Visit(Renderable* r) = 0;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_SCENE_RENDERABLE_H