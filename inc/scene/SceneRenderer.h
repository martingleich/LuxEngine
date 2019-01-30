#ifndef INCLUDED_LUX_SCENE_RENDERER_H
#define INCLUDED_LUX_SCENE_RENDERER_H
#include "core/ReferenceCounted.h"
#include "core/Attributes.h"

#include "scene/Component.h"

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

//! Data passed to module factory to create a scene renderer.
class SceneRendererInitData
{
public:
	StrongRef<Scene> scene;
};

class SceneRendererPassSettings
{
public:
	virtual ~SceneRendererPassSettings() {}
	bool wireframe = false;
	bool disableCulling = false;
};

//! Interface to render scenes.
class SceneRenderer : public ReferenceCounted
{
public:
	virtual ~SceneRenderer() {}

	////////////////////////////////////////////////////////////////////////////////////

	//! Draws a scene.
	/**
	\param beginScene Should the renderer call video::Renderer::BeginScene, if not the scene must be already started with the correct rendertarget in place.
	\param endScene Should the renderer call video::Renderer::EndScene
	*/
	virtual void DrawScene(
		bool beginScene = true,
		bool endScene = true) = 0;

	////////////////////////////////////////////////////////////////////////////////////

	//! Get the list of attributes of the scene renderer.
	/**
	Attributes are options to change the behaviour of the renderer, see the renderer documentaions for available options.
	*/
	virtual core::AttributeList AttributeList() const = 0;

	virtual void SetPassSettings(ERenderPass pass, const SceneRendererPassSettings& settings) = 0;
	virtual const SceneRendererPassSettings& GetPassSettings(ERenderPass pass ) = 0;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SCENE_RENDERER_H