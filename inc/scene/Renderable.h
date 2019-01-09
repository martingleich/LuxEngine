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
	Any,
};

enum class ERenderableTags
{
	None = 0,
	EditView = 1,
	Invisible = 2,
};

class Light;
class AbstractCamera;
class SceneData
{
public:
	Node* activeCameraNode;
	AbstractCamera* activeCamera;

	ERenderPass pass;

	SceneData() {}
};

class Renderable
{
public:
	virtual ~Renderable() {}
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
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SCENE_RENDERABLE_H