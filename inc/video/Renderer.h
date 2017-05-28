#ifndef INCLUDED_RENDERER_H
#define INCLUDED_RENDERER_H
#include "core/ReferenceCounted.h"

#include "video/EPrimitiveType.h"
#include "video/HardwareBufferConstants.h"
#include "video/Color.h"

#include "math/aabbox3d.h"
#include "math/rect.h"
#include "math/matrix4.h"

namespace lux
{
namespace core
{
class ParamPackage;
class PackageParam;
}

namespace video
{
class SubMesh;
class LightData;
class FogData;
class RenderTarget;
class Material;
class PipelineSettings;
class PipelineOverwrite;
class VertexFormat;
class RenderStatistics;
class VideoDriver;
class AlphaBlendSettings;

enum class ETransform
{
	World,
	View,
	Projection
};

/**
Rendering 2d or 3d data:

The renderer class can be used to render 2d or 3d data:

When rendering 2d data, lighting and fogging is unavailable.
Polygonoffset aren't available as well.
Positioning in 2d mode is done in pixels starting from the top left corner of
the screen, with growing x coordinates to the right and growing y coordinated down.
The z coordinate and the z buffer is still available when rendering in 2d mode, but
it's usage is dicourages most time it's better to disable the z-writing and reading
and draw all object on a fixed z plane.

Rendering in 3d mode uses the default directX coordinate system with x left, y up, and z into the screen
**/

//! The renderer class of the engine
/**
This class is used to render things.
The procedure for rendering a single frame is alwyas the same.
First a new scene is started with \ref Renderer::BeginScene
Then the render parameters(i.e. Materials, transforms, etc.) are set
Then a number of rendercommands are issued which use these parameters.
After any number of repetitions of these two steps the scene is finished with \ref Renderer:EndScene
If data was rendered to the backbuffer \ref Renderer::Present can be used to display it
*/
class Renderer : public ReferenceCounted
{
public:
	virtual ~Renderer() {}

	///////////////////////////////////////////////////////////////////////////

	//! Begin a new scene
	/**
	Clears the current rendertarget and starts an new scene.
	Draw commands can only be used, with a started scene
	Each begun scene must be finished with EndScene.
	\param clearColor Shall the color of the rendertarget be cleared
	\param clearZBuffer Shall the z buffer be cleared
	\param color The color to set the rendertarget to
	\param z The value to which the zBuffer is set
	*/
	virtual void BeginScene(
		bool clearColor, bool clearZBuffer,
		video::Color color = video::Color::Black, float z = 1.0f) = 0;

	//! Finishes a scene
	/**
	After this call the rendertarget is filled with data and can be read again
	*/
	virtual void EndScene() = 0;

	//! Displays the backbuffer
	/**
	The backbuffer is shown in the windows with which the video driver was created
	\return True if the present was successfull, false otherwise. Presenting may fail if the window
	is minimized or the enery saving mode is active
	*/
	virtual bool Present() = 0;

	//! Set the rendertarget
	/**
	Must be called before calling BeginScene.
	*/
	virtual void SetRenderTarget(const RenderTarget& target) = 0;

	//! Get the current rendertarget
	virtual const RenderTarget& GetRenderTarget() = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Set the active material
	virtual void SetMaterial(const Material* material) = 0;

	//! Get the active matrial
	virtual const Material* GetMaterial() const = 0;

	//! Set the invalid material
	/**
	The invalid material is used to render invalid materials, for example materials with
	missing renderers.
	The invalid is automatically set by the engine.
	*/
	virtual void SetInvalidMaterial(const Material* material) = 0;

	//! Get the invalid material
	virtual const Material* GetInvalidMaterial() = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Apply a pipeline overwrite
	/**
	See \ref PipelineOverwrite about the usage of pipeline overwrites
	*/
	virtual void PushPipelineOverwrite(const PipelineOverwrite& over) = 0;

	//! Remove the previous added pipeline overwrite
	virtual void PopPipelineOverwrite() = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Add a new active light
	virtual void AddLight(const LightData& light) = 0;

	//! Remove all lights from the scene
	virtual void ClearLights() = 0;

	//! The maximal number of lights which can be active at once
	virtual size_t GetMaxLightCount() const = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Set the active fog
	virtual void SetFog(const FogData& fog) = 0;

	//! Get the active fog
	virtual const FogData& GetFog() const = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Set a transform matrix
	virtual void SetTransform(ETransform transform, const math::matrix4& matrix) = 0;

	//! Get a active transform matrix
	virtual const math::matrix4& GetTransform(ETransform transform) const = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Add a new scene parameter
	/**
	Scene parameters can be accessed from shaders of material renderers.
	\param name The name of the new parameter
	\param type The type of the new parameter
	\return The id which was assign to this scene parameter
	\throws InvalidArgumentException If the name is already in use
	*/
	virtual u32 AddParam(const string_type& name, core::Type type) = 0;

	//! Retrieve a scene parameter by it's id
	virtual core::PackageParam GetParam(u32 id) = 0;

	//! Retrieve a scene parameter by it's name
	/**
	\throws ObjectNotFoundException If the parameter can not be found
	*/
	virtual core::PackageParam GetParam(const string_type& string) = 0;

	//! Get the total number of parameters
	/**
	The number from 0 to count can be used as parameter id's.
	*/
	virtual u32 GetParamCount() const = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Draw a indexed primitive list from memory
	/**
	\param primitiveType The primitive type to draw
	\param primitiveCount The number of primitives to draw
	\param vertexData The vertices to draw
	\param vertexCount The nuumber of vertices to draw
	\param vertexFormat The format of the vertices
	\param indexData The indices to draw
	\param indexType The format of the indices
	\param is3D Is the 3d or the 2d pipeline used
	*/
	virtual void DrawIndexedPrimitiveList(
		EPrimitiveType primitiveType, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		const void* indexData, EIndexFormat indexType,
		bool is3D) = 0;

	//! Draw a primitive list from memory
	/**
	\param primitiveType The primitive type to draw
	\param primitiveCount The number of primitives to draw
	\param vertexData The vertices to draw
	\param vertexCount The nuumber of vertices to draw
	\param vertexFormat The format of the vertices
	\param is3D Is the 3d or the 2d pipeline used
	*/
	virtual void DrawPrimitiveList(
		EPrimitiveType primitiveType, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		bool is3D) = 0;

	//! Draw a sub mesh
	/**
	\param subMesh The submesh to draw
	\param primitiveCount The number of primitives to draw
	\param is3D Is the 3d or the 2d pipeline used
	*/
	virtual void DrawSubMesh(const SubMesh* subMesh, u32 primitiveCount, bool is3D = true) = 0;

	//! Draw a sub mesh
	/**
	\param subMesh The submesh to draw
	\param is3D Is the 3d or the 2d pipeline used
	*/
	virtual void DrawSubMesh(const SubMesh* subMesh, bool is3D = true) = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Access information about rendering statistics
	virtual StrongRef<RenderStatistics> GetRenderStatistics() const = 0;

	//! Retrieve the driver owning this renderer
	virtual VideoDriver* GetDriver() const = 0;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_RENDERER_H