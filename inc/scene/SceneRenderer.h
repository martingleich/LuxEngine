#ifndef INCLUDED_SCENE_RENDERER_H
#define INCLUDED_SCENE_RENDERER_H
#include "core/ReferenceCounted.h"
#include "core/Attributes.h"
#include "core/ModuleFactory.h"

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

//! Data passed to module factory to create a scene renderer.
struct SceneRendererInitData : public core::ModuleInitData
{};

//! Interface to render scenes.
class SceneRenderer : public ReferenceCounted
{
public:
	virtual ~SceneRenderer() {}

	////////////////////////////////////////////////////////////////////////////////////

	//! Draws a scene.
	/**
	\param scene The scene to draw.
	\param beginScene Should the renderer call video::Renderer::BeginScene, if not the scene must be already started with the correct rendertarget in place.
	\param endScene Should the renderer call video::Renderer::EndScene
	*/
	virtual void DrawScene(Scene* scene, bool beginScene = true, bool endScene = true) = 0;

	////////////////////////////////////////////////////////////////////////////////////

	//! Access an attribute of the scene renderer.
	/**
	Attributes are options to change the behaviour of the renderer, see the renderer documentaions for available options.
	*/
	LUX_API core::VariableAccess Attribute(const String& str);

	//! Get the list of attributes of the scene renderer.
	/**
	Attributes are options to change the behaviour of the renderer, see the renderer documentaions for available options.
	*/
	LUX_API const core::Attributes& Attributes() const;

protected:
	core::Attributes m_Attributes;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_SCENE_RENDERER_H