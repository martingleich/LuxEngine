#ifndef INCLUDED_LUX_SCENE_RENDERABLE_H
#define INCLUDED_LUX_SCENE_RENDERABLE_H

namespace lux
{
namespace video
{
class Renderer;
}
namespace scene
{
class Node;
class AbstractCamera;

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

class SceneRenderData
{
public:
	video::Renderer* video;
	ERenderPass pass;

	AbstractCamera* activeCamera;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SCENE_RENDERABLE_H