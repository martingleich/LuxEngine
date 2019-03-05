#ifndef INCLUDED_LUX_SCENE_RENDERABLE_H
#define INCLUDED_LUX_SCENE_RENDERABLE_H
#include "math/ViewFrustum.h"
#include "video/RenderTarget.h"
#include "video/AbstractMaterial.h"
#include "core/lxOptional.h"
#include "core/lxEnumSet.h"

namespace lux
{
namespace video
{
class Renderer;
}
namespace scene
{

enum class ERenderPass
{
	Unknown,

	SkyBox,
	Transparent,
	Solid,
	ShadowCaster,
};

using RenderPassSet = core::EnumSet<ERenderPass>;

class SceneRenderCamData
{
public:
	math::Transformation transform;
	math::ViewFrustum frustum;
};

class SceneRenderData
{
public:
	video::Renderer* video;
	ERenderPass pass;

	SceneRenderCamData camData;
};

class SceneRenderPassDefaultData
{
public:
	// Setup settings.
	SceneRenderCamData camData;
	//u32 matchAnyTags = 0xFFFFFFFF;
	//u32 matchAllTags = 0;

	// RenderSettings
	video::RenderTarget renderTarget;
	//core::Optional<bool> clearColor;
	//core::Optional<bool> clearStencil;
	//core::Optional<bool> clearDepth;

	//core::Optional<bool> beginNewScene;
	core::Action<> onPreRender;

	core::Action<> onPostRender;
	//core::Optional<bool> endScene;
};


class SceneRenderPassHelper
{
public:
	virtual ~SceneRenderPassHelper() {}
	virtual video::Renderer* GetRenderer() = 0;

	virtual void DefaultRenderScene(const SceneRenderPassDefaultData& data) = 0;
};

class SceneRenderPassController
{
public:
	virtual int GetPriority() const = 0;
	virtual void Render(SceneRenderPassHelper* helper) = 0;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SCENE_RENDERABLE_H