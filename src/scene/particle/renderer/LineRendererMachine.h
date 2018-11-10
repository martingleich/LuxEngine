#ifndef INCLUDED_LUX_PARTICLERENDERER_LINE_IMPL_H
#define INCLUDED_LUX_PARTICLERENDERER_LINE_IMPL_H
#include "video/VertexTypes.h"
#include "video/VertexFormat.h"
#include "video/Renderer.h"
#include "video/Pass.h"

#include "scene/particle/ParticleRenderer.h"
#include "scene/particle/ParticleModel.h"
#include "scene/particle/BuiltinParticleRenderers.h"

namespace lux
{
namespace scene
{
class ParticleGroupData;

class LineRendererMachine : public LineRenderer::Machine 
{
private:
	struct LineVertex
	{
		math::Vector3F position;
		video::Color color;
	};

public:
	LineRendererMachine();
	static StrongRef<LineRendererMachine> GetShared();
	void Render(video::Renderer* videoRenderer, ParticleGroupData* group, LineRenderer* renderer);

private:
	void CreateBuffer(ParticleGroupData* group);

private:
	StrongRef<LineRenderer> m_Data;
	core::Array<LineVertex> m_Vertices;

	video::Pass m_DefaultPass;
	video::Pass m_EmitPass;

	video::VertexFormat m_VertexFormat;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_LUX_CPARTICLELINERENDERER
