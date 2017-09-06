#ifndef INCLUDED_RENDERER_NULL
#define INCLUDED_RENDERER_NULL
#include "video/Renderer.h"

#include "core/ParamPackage.h"

#include "video/RenderSettings.h"
#include "video/Material.h"
#include "video/MaterialRenderer.h"
#include "video/LightData.h"
#include "video/FogData.h"
#include "video/RenderStatistics.h"
#include "video/MatrixTable.h"
#include "video/Pass.h"

namespace lux
{
namespace video
{

enum class ERenderMode
{
	Mode3D,
	Mode2D,
	None,
};

class RendererNull : public Renderer
{
protected:
	enum EDirtyFlags
	{
		Dirty_Material,
		Dirty_MaterialRenderer,
		Dirty_World,
		Dirty_ViewProj,
		Dirty_Lights,
		Dirty_Rendertarget,
		Dirty_RenderMode,
		Dirty_Fog,
		Dirty_PolygonOffset,
	};

public:
	RendererNull(VideoDriver* driver);
	virtual ~RendererNull() {}

	///////////////////////////////////////////////////////////////////////////

	void SetPass(const Pass& pass, bool useOverwrite = false, ParamSetCallback* paramSetCallback = nullptr);
	void SetMaterial(const Material* material);

	void SetInvalidMaterial(const Material* material);
	const Material* GetInvalidMaterial();

	///////////////////////////////////////////////////////////////////////////

	void PushPipelineOverwrite(const PipelineOverwrite& over, PipelineOverwriteToken* token);
	void PopPipelineOverwrite(PipelineOverwriteToken* token);

private:
	void UpdatePipelineOverwrite();
public:

	///////////////////////////////////////////////////////////////////////////

	void AddLight(const LightData& light);
	void ClearLights();

	void SetFog(const FogData& fog);
	void ClearFog();

	///////////////////////////////////////////////////////////////////////////

	void SetTransform(ETransform transform, const math::Matrix4& matrix);
	const math::Matrix4& GetTransform(ETransform transform) const;
	void SetNormalizeNormals(bool normalize, NormalizeNormalsToken* token);
	bool GetNormalizeNormals() const;

	///////////////////////////////////////////////////////////////////////////

	void AddParam(const String& name, core::Type type, const void* value);

	core::AttributePtr GetParam(const String& name) const;
	const core::Attributes& GetParams() const;

	///////////////////////////////////////////////////////////////////////////

	VideoDriver* GetDriver() const;

protected:
	inline bool IsDirty(u32 flag)
	{
		return (m_DirtyFlags & (1 << flag)) != 0;
	}

	inline void ClearDirty(u32 flag)
	{
		m_DirtyFlags &= ~(1 << flag);
	}

	inline void SetDirty(u32 flag)
	{
		m_DirtyFlags |= (1 << flag);
	}

	inline void ClearAllDirty()
	{
		m_DirtyFlags = 0;
	}

	math::Matrix4 GenerateLightMatrix(const LightData& data, bool active);

protected:
	struct ParamIdCollection
	{
		core::AttributePtr lighting;
		core::AttributePtr ambient;
		core::AttributePtr time;

		core::AttributePtr fog1;
		core::AttributePtr fog2;

		core::Array<core::AttributePtr> lights;
	};

protected:
	ERenderMode m_RenderMode; //!< Active rendermode

	// The userset parameters are only rememberd they don't have to be activated immediatly
	bool m_UseMaterial;
	bool m_UseOverwrite;
	ParamSetCallback* m_ParamSetCallback;
	Pass m_Pass;
	StrongRef<Material> m_Material; //!< User set material
	StrongRef<Material> m_InvalidMaterial; //!< The material used to render invalid materials

	core::Array<PipelineOverwrite> m_PipelineOverwrites; //!< User set pipeline overwrites
	PipelineOverwrite m_FinalOverwrite;

	core::Array<LightData> m_Lights; //!< User set lights

	bool m_IsFogActive;
	FogData m_Fog; //!< User set fog data

	bool m_NormalizeNormals;
	math::Matrix4 m_TransformWorld;
	math::Matrix4 m_TransformView;
	math::Matrix4 m_TransformProj;

	MatrixTable m_MatrixTable; //! The currently set matrices, these are used as arguments for shaders and other rendercomponents
	core::Attributes m_Params;
	ParamIdCollection m_ParamId;

	u32 m_DirtyFlags; //!< The flag list of changed user parameters, see \ref EDirtyFlags

	VideoDriver* m_Driver; //!< The driver owning this renderer
	RenderStatistics* m_RenderStatistics;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_RENDERER_NULL