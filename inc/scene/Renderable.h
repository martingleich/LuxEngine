#ifndef INCLUDED_SCENE_RENDERABLE_H
#define INCLUDED_SCENE_RENDERABLE_H
#include "video/Material.h"
#include "math/aabbox3d.h"

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

	COUNT,
};

class Renderable
{
public:
	virtual size_t GetMaterialCount() const
	{
		return 0;
	}
	virtual video::Material* GetMaterial(size_t)
	{
		throw core::OutOfRangeException();
	}
	virtual const video::Material* GetMaterial(size_t) const
	{
		throw core::OutOfRangeException();
	}
	virtual void SetMaterial(size_t, video::Material*)
	{
		throw core::OutOfRangeException();
	}

	virtual void Render(Node* node, video::Renderer* renderer, ERenderPass pass) = 0;
	virtual ERenderPass GetRenderPass() const = 0;
	
	virtual const math::aabbox3df& GetBoundingBox() const = 0;
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