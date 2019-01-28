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
#include "video/Material.h"

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
class RenderTarget;
class AbstractMaterial;
class Pass;
class ShaderParamSetCallback;
class PipelineSettings;
class PipelineOverwrite;
class VertexFormat;
class VideoDriver;
class VertexBuffer;
class IndexBuffer;

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

struct RenderRequest
{
	union
	{
		struct UserDataT
		{
			const void* vertexData;
			const void* indexData;
			const VertexFormat* vertexFormat;
			u32 vertexCount;
			EIndexFormat indexFormat;
		} userData;
		struct BufferDataT
		{
			const VertexBuffer* vb;
			const IndexBuffer* ib;
		} bufferData;
	};
	u32 firstPrimitive;
	u32 primitiveCount;

	EFaceWinding frontFace;
	EPrimitiveType primitiveType;

	bool userPointer;
	bool indexed;

	static RenderRequest IndexedFromMemory(
		EPrimitiveType primitiveType, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		const void* indexData, EIndexFormat indexType,
		EFaceWinding frontFace = EFaceWinding::CCW)
	{
		RenderRequest rq;
		rq.userPointer = true;
		rq.userData.vertexData = vertexData;
		rq.userData.vertexCount = vertexCount;
		rq.userData.vertexFormat = &vertexFormat;
		rq.userData.indexData = indexData;
		rq.userData.indexFormat = indexType;
		rq.indexed = true;
		rq.firstPrimitive = 0;
		rq.primitiveCount = primitiveCount;
		rq.primitiveType = primitiveType;
		rq.frontFace = frontFace;
		return rq;
	}
	static RenderRequest FromMemory(
		EPrimitiveType primitiveType, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		EFaceWinding frontFace = EFaceWinding::CCW)
	{
		RenderRequest rq;
		rq.userPointer = true;
		rq.userData.vertexData = vertexData;
		rq.userData.vertexCount = vertexCount;
		rq.userData.vertexFormat = &vertexFormat;
		rq.indexed = false;
		rq.firstPrimitive = 0;
		rq.primitiveCount = primitiveCount;
		rq.primitiveType = primitiveType;
		rq.frontFace = frontFace;
		return rq;
	}

	LUX_API static RenderRequest FromGeometry(const Geometry* geo);
	LUX_API static RenderRequest FromGeometry(const Geometry* geo, int first, int count);
};

enum class ERenderMode
{
	Mode3D,
	Mode2D,
	None,
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

	//! Get the current major rendertarget
	virtual const RenderTarget& GetRenderTarget() = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Set the active pass
	void SendPassSettings(
		const Pass& pass,
		bool useOverwrite = false,
		ShaderParamSetCallback* paramSetCallback = nullptr,
		void* userParam = nullptr)
	{
		SendPassSettingsEx(
			ERenderMode::Mode3D,
			pass,
			useOverwrite,
			paramSetCallback,
			userParam);
	}

	virtual void SendPassSettingsEx(
		ERenderMode renderMode,
		const Pass& pass,
		bool useOverwrite = false,
		ShaderParamSetCallback* paramSetCallback = nullptr,
		void* userParam = nullptr) = 0;

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

	//! Set a transform matrix
	virtual void SetTransform(ETransform transform, const math::Matrix4& matrix) = 0;
	virtual const math::Matrix4& GetTransform(ETransform transform) const = 0;

	//! Renormalize normals after transformation(default = true)
	virtual void SetNormalizeNormals(bool normalize, NormalizeNormalsToken* token = nullptr) = 0;
	virtual bool GetNormalizeNormals() const = 0;

	///////////////////////////////////////////////////////////////////////////

	virtual core::AttributeList GetBaseParams() const = 0;
	virtual void SetParams(core::AttributeList attributes) = 0;
	virtual core::AttributeList GetParams() const = 0;

	///////////////////////////////////////////////////////////////////////////

	virtual void Draw(const RenderRequest& rq) = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Retrieve the driver owning this renderer
	virtual VideoDriver* GetDriver() const = 0;
};

struct VideoRendererToken : core::Uncopyable
{
	Renderer* renderer = nullptr;
	void Unlock() { renderer = nullptr; }
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