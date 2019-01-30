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
		Dirty_ViewProj,
		Dirty_Rendertarget,
	};

public:
	RendererNull(VideoDriver* driver, video::MatrixTable& matrixTable);
	virtual ~RendererNull() {}

	///////////////////////////////////////////////////////////////////////////

	void PushPipelineOverwrite(const PipelineOverwrite& over, PipelineOverwriteToken* token);
	void PopPipelineOverwrite(PipelineOverwriteToken* token);

	///////////////////////////////////////////////////////////////////////////

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