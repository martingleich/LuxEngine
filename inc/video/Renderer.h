#ifndef INCLUDED_LUX_RENDERER_H
#define INCLUDED_LUX_RENDERER_H
#include "core/ReferenceCounted.h"

#include "video/VideoEnums.h"
#include "video/Color.h"

#include "math/AABBox.h"
#include "math/Rect.h"
#include "math/Matrix4.h"

#include "core/Attributes.h"

#include "video/RenderStatistics.h"

namespace lux
{
namespace core
{
class ParamPackage;
class VariableAccess;
}

namespace video
{
class Geometry;
class LightData;
class FogData;
class RenderTarget;
class AbstractMaterial;
class Pass;
class ShaderParamSetCallback;
class PipelineSettings;
class PipelineOverwrite;
class VertexFormat;
class VideoDriver;

//! The diffrent transforms which can be set in the renderer
enum class ETransform
{
	World, //!< The current world transformation
	View, //!< The current view transformation
	Projection, //!< The current projection transformation.
};

struct PipelineOverwriteToken;
struct ScissorRectToken;
struct NormalizeNormalsToken;

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
	Draw commands can only be used, with a started scene
	Each begun scene must be finished with EndScene.
	*/
	virtual void BeginScene() = 0;

	//! Clear all currently set rendertargets
	/*
	\param clearColor Shall the color of the rendertarget be cleared
	\param clearZBuffer Shall the z buffer be cleared
	\param clearStencil Shall the stencil buffer be cleared
	\param color The color to set the rendertarget to
	\param z The value to which the zBuffer is set
	\param stencil The value to which the stencil buffer is cleared
	*/
	virtual void Clear(
		bool clearColor, bool clearZBuffer, bool clearStencil,
		video::Color color = video::Color::Black, float z = 1.0f, u32 stencil = 0) = 0;

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
	virtual void SetRenderTarget(const core::Array<RenderTarget>& targets) = 0;
	virtual const math::Dimension2I& GetRenderTargetSize() = 0;

	//! Get the current major rendertarget
	virtual const RenderTarget& GetRenderTarget() = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Set the active pass
	virtual void SetPass(const Pass& pass, bool useOverwrite = false, ShaderParamSetCallback* paramSetCallback = nullptr, void* userParam = nullptr) = 0;

	//! Set the active material
	virtual void SetMaterial(AbstractMaterial* material) = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Apply a pipeline overwrite
	/**
	See \ref PipelineOverwrite about the usage of pipeline overwrites.
	\param over The overwrite to apply
	\param token A token to restore the previous state of the overwrites, see \ref PipelineOverwriteToken for more
	*/
	virtual void PushPipelineOverwrite(const PipelineOverwrite& over, PipelineOverwriteToken* token = nullptr) = 0;

	//! Remove the previous added pipeline overwrite
	/*
	\param token A token to restore the previous state of the overwrites, see \ref PipelineOverwriteToken for more
	*/
	virtual void PopPipelineOverwrite(PipelineOverwriteToken* token = nullptr) = 0;

	virtual void SetScissorRect(const math::RectI& rect, ScissorRectToken* token = nullptr) = 0;
	virtual const math::RectI& GetScissorRect() const = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Add a new active light
	virtual void AddLight(const LightData& light) = 0;

	//! Remove all lights from the scene
	virtual void ClearLights() = 0;

	//! The maximal number of lights which can be active at once
	virtual int GetMaxLightCount() const = 0;

	//! Set and enable fog
	virtual void SetFog(const FogData& fog) = 0;

	//! Disable the fog
	virtual void ClearFog() = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Set a transform matrix
	virtual void SetTransform(ETransform transform, const math::Matrix4& matrix) = 0;

	//! Get a active transform matrix
	virtual const math::Matrix4& GetTransform(ETransform transform) const = 0;

	//! Renormalize normals after transformation(default = true)
	virtual void SetNormalizeNormals(bool normalize, NormalizeNormalsToken* token = nullptr) = 0;
	virtual bool GetNormalizeNormals() const = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Add a new scene parameter
	/**
	Scene parameters can be accessed from shaders of material renderers.
	\param name The name of the new parameter
	\param type The type of the new parameter
	\throws InvalidArgumentException If the name is already in use
	*/
	virtual void AddParam(const core::String& name, core::Type type, const void* value = nullptr) = 0;

	template <typename T>
	void AddParam(const core::String& name, const T& value)
	{
		AddParam(name, core::TemplType<T>::Get(), &value);
	}

	virtual core::AttributePtr GetParam(const core::String& name) const = 0;
	virtual const core::Attributes& GetParams() const = 0;

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
	\param is3D Is the 2D or 3D pipeline used
	\param frontFace Clockwise or counter clockwise
	*/
	virtual void DrawIndexedPrimitiveList(
		EPrimitiveType primitiveType, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		const void* indexData, EIndexFormat indexType,
		bool is3D,
		EFaceWinding frontFace = EFaceWinding::CCW) = 0;

	void DrawIndexed3DPrimitiveList(
		EPrimitiveType primitiveType, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		const void* indexData, EIndexFormat indexType,
		EFaceWinding frontFace = EFaceWinding::CCW)
	{
		DrawIndexedPrimitiveList(primitiveType, primitiveCount,
			vertexData, vertexCount, vertexFormat,
			indexData, indexType,
			true,
			frontFace);
	}

	void DrawIndexed2DPrimitiveList(
		EPrimitiveType primitiveType, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		const void* indexData, EIndexFormat indexType,
		EFaceWinding frontFace = EFaceWinding::CCW)
	{
		DrawIndexedPrimitiveList(primitiveType, primitiveCount,
			vertexData, vertexCount, vertexFormat,
			indexData, indexType,
			false,
			frontFace);
	}

	//! Draw a primitive list from memory
	/**
	\param primitiveType The primitive type to draw
	\param primitiveCount The number of primitives to draw
	\param vertexData The vertices to draw
	\param vertexCount The nuumber of vertices to draw
	\param vertexFormat The format of the vertices
	\param is3D Is the 3d or the 2d pipeline used
	\param frontFace Clockwise or counter clockwise
	*/
	virtual void DrawPrimitiveList(
		EPrimitiveType primitiveType, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		bool is3D,
		EFaceWinding frontFace = EFaceWinding::CCW) = 0;

	void Draw2DPrimitiveList(
		EPrimitiveType primitiveType, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		EFaceWinding frontFace = EFaceWinding::CCW)
	{
		DrawPrimitiveList(
			primitiveType, primitiveCount,
			vertexData, vertexCount, vertexFormat,
			false, frontFace);
	}

	void Draw3DPrimitiveList(
		EPrimitiveType primitiveType, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		EFaceWinding frontFace = EFaceWinding::CCW)
	{
		DrawPrimitiveList(
			primitiveType, primitiveCount,
			vertexData, vertexCount, vertexFormat,
			true, frontFace);
	}

	//! Draw some geometry
	/**
	\param geo The geometry to draw
	\param primitiveCount The number of primitives to draw
	\param is3D Is the 3d or the 2d pipeline used
	*/
	virtual void DrawGeometry(const Geometry* geo, u32 firstPrimitive, u32 primitiveCount, bool is3D = true) = 0;

	//! Draw some geometry
	/**
	\param geo The geometry to draw
	\param is3D Is the 3d or the 2d pipeline used
	*/
	virtual void DrawGeometry(const Geometry* geo, bool is3D = true) = 0;

	///////////////////////////////////////////////////////////////////////////


	//! Retrieve the driver owning this renderer
	virtual VideoDriver* GetDriver() const = 0;
};

struct VideoRendererToken
{
	Renderer* renderer = nullptr;
	void Unlock()
	{
		renderer = nullptr;
	}
};

//! A token to restore the previous state of the overwrite system.
/**
Pass a token to PushPipelineOverwrite so that when the token is destructed the
corresponding overwrite is automatically removed from the pipeline.<br>
If you pop the overwrite by yourself, pass the token too so that is doesn't pop
the same entry twice.<br>
The same token can be used with multiple pushs and pops.<br>
An example:
\code{.cpp}
PipelineOverwriteToken token;
PipelineOverwrite over;
// Set values for overwrite...
renderer->PushPipelineOverwrite(over, &token);
// Render things...
// The PopPipelineOverwrite is automatically called by the token.
\endcode
It's recommended to used the token instead of restoring the pipeline by yourself
since it works reliably in the presence of exceptions and can't be forgotten.
*/
struct PipelineOverwriteToken : VideoRendererToken
{
	~PipelineOverwriteToken()
	{
		if(renderer) {
			while(count)
				renderer->PopPipelineOverwrite(this);
		}
	}

	u32 count = 0;
};

struct ScissorRectToken : VideoRendererToken
{
	~ScissorRectToken()
	{
		if(renderer)
			renderer->SetScissorRect(prevRect);
	}

	math::RectI prevRect;
};

struct NormalizeNormalsToken : public VideoRendererToken
{
	~NormalizeNormalsToken()
	{
		if(renderer)
			renderer->SetNormalizeNormals(prev);
	}

	bool prev;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_RENDERER_H