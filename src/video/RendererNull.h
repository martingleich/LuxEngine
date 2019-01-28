#ifndef INCLUDED_LUX_RENDERER_NULL
#define INCLUDED_LUX_RENDERER_NULL
#include "video/Renderer.h"

#include "core/ParamPackage.h"

#include "video/AbstractMaterial.h"
#include "video/RenderStatistics.h"
#include "video/MatrixTable.h"
#include "video/Pass.h"

namespace lux
{
namespace video
{

class RendererNull : public Renderer
{
protected:
	enum EDirtyFlags
	{
		Dirty_World,
		Dirty_ViewProj,
		Dirty_Rendertarget,
	};

public:
	RendererNull(VideoDriver* driver);
	virtual ~RendererNull() {}

	///////////////////////////////////////////////////////////////////////////

	void PushPipelineOverwrite(const PipelineOverwrite& over, PipelineOverwriteToken* token);
	void PopPipelineOverwrite(PipelineOverwriteToken* token);

	///////////////////////////////////////////////////////////////////////////

	void SetTransform(ETransform transform, const math::Matrix4& matrix);
	const math::Matrix4& GetTransform(ETransform transform) const;
	void SetNormalizeNormals(bool normalize, NormalizeNormalsToken* token);
	bool GetNormalizeNormals() const;

	///////////////////////////////////////////////////////////////////////////

	core::AttributeList GetBaseParams() const;
	void SetParams(core::AttributeList attributes);
	core::AttributeList GetParams() const;

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

private:
	void UpdatePipelineOverwrite();

protected:
	struct ParamIdCollection
	{
		core::AttributePtr lighting;
		core::AttributePtr fogEnabled;

		core::AttributePtr ambient;
		core::AttributePtr time;
	};

protected:
	ERenderMode m_RenderMode; //!< Active rendermode

	core::Array<PipelineOverwrite> m_PipelineOverwrites; //!< User set pipeline overwrites
	PipelineOverwrite m_FinalOverwrite;

	bool m_NormalizeNormals;
	math::Matrix4 m_TransformWorld;
	math::Matrix4 m_TransformView;
	math::Matrix4 m_TransformProj;

	MatrixTable m_MatrixTable; //!< The currently set matrices, these are used as arguments for shaders and other rendercomponents
	core::AttributeList m_BaseParams;
	core::AttributeList m_Params;
	ParamIdCollection m_ParamIds;

	u32 m_DirtyFlags; //!< The flag list of changed user parameters, see \ref EDirtyFlags

	VideoDriver* m_Driver; //!< The driver owning this renderer
	RenderStatistics* m_RenderStatistics;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_RENDERER_NULL