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
#include "video/PipelineSettings.h"

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
	friend class ParamListAccessNull;
protected:
	enum EDirtyFlags
	{
		Dirty_Material,
		Dirty_MaterialRenderer,
		Dirty_PipelineOverwrites,
		Dirty_Transform,
		Dirty_Lights,
		Dirty_Pipeline,
		Dirty_Rendertarget,
		Dirty_RenderMode,
		Dirty_Fog,
		Dirty_PolygonOffset,
	};

	struct ParamIdCollection
	{
		u32 lighting;
		u32 ambient;
		u32 time;

		u32 fogRange;
		u32 fogColor;
		u32 fogInfo;

		core::array<u32> lights;
	};

public:
	RendererNull(VideoDriver* driver);
	virtual ~RendererNull() {}

	///////////////////////////////////////////////////////////////////////////

	void SetMaterial(const Material* material);
	const Material* GetMaterial() const;

	void SetInvalidMaterial(const Material* material);
	const Material* GetInvalidMaterial();

	///////////////////////////////////////////////////////////////////////////

	void PushPipelineOverwrite(const PipelineOverwrite& over);
	void PopPipelineOverwrite();

	///////////////////////////////////////////////////////////////////////////

	void AddLight(const LightData& light);
	void ClearLights();

	///////////////////////////////////////////////////////////////////////////

	void SetFog(const FogData& fog);
	const FogData& GetFog() const;

	///////////////////////////////////////////////////////////////////////////

	void SetTransform(ETransform transform, const math::matrix4& matrix);
	const math::matrix4& GetTransform(ETransform transform) const;

	///////////////////////////////////////////////////////////////////////////

	u32 AddParam(const string_type& name, core::Type type);
	u32 AddInternalParam(const string_type& name, core::Type type);
	u32 AddParamEx(const string_type& name, core::Type type, bool isInternal);
	core::PackageParam GetParamEx(u32 id, bool internal);
	core::PackageParam GetParam(u32 id);
	core::PackageParam GetParamInternal(u32 id);
	core::PackageParam GetParam(const string_type& string);
	u32 GetParamCount() const;

	///////////////////////////////////////////////////////////////////////////

	StrongRef<RenderStatistics> GetRenderStatistics() const;
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

	math::matrix4 GenerateLightMatrix(const LightData& data, bool active);

private:
	bool GetLightId(const string_type& string, u32& outId);

protected:
	ERenderMode m_RenderMode; //!< Active rendermode

	// The userset parameters are only rememberd they don't have to be activated immediatly
	StrongRef<Material> m_Material; //!< User set material
	StrongRef<Material> m_InvalidMaterial; //!< The material used to render invalid materials

	core::array<PipelineOverwrite> m_PipelineOverwrites; //!< User set pipeline overwrites

	core::array<LightData> m_Lights; //!< User set lights

	FogData m_Fog; //!< User set fog data

	math::matrix4 m_TransformWorld;
	math::matrix4 m_TransformView;
	math::matrix4 m_TransformProj;

	MatrixTable m_MatrixTable; //! The currently set matrices, these are used as arguments for shaders and other rendercomponents

	core::array<bool> m_InternalTable; //! Saves if a parameter is internal
	core::ParamList m_ParamList; //!< User set parameters
	ParamIdCollection m_ParamId; //!< Collection of default param id's

	u32 m_DirtyFlags; //!< The flag list of changed user parameters, see \ref EDirtyFlags

	VideoDriver* m_Driver; //!< The driver owning this renderer

	StrongRef<RenderStatistics> m_RenderStatistics;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_RENDERER_NULL