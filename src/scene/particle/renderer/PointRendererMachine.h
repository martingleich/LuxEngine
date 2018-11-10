#ifndef INCLUDED_LUX_PARTICLERENDERER_POINT_MACHINE_H
#define INCLUDED_LUX_PARTICLERENDERER_POINT_MACHINE_H
#include "video/VertexTypes.h"
#include "video/VertexFormat.h"
#include "video/Renderer.h"
#include "video/Pass.h"

#include "scene/particle/BuiltinParticleRenderers.h"
#include "scene/particle/ParticleModel.h"

namespace lux
{
namespace scene
{
class ParticleGroupData;

class PointRendererMachine : public PointRenderer::Machine
{
private:
	struct PointVertex
	{
		math::Vector3F position;
		video::Color color;
	};

public:
	PointRendererMachine();
	static StrongRef<PointRendererMachine> GetShared();
	void Render(video::Renderer* videoRenderer, ParticleGroupData* group, PointRenderer* renderer);

private:
	void CreateBuffer(ParticleGroupData* group);

private:
	core::Array<PointVertex> m_Vertices;

	video::Pass m_DefaultPass;
	video::Pass m_EmitPass;

	video::VertexFormat m_VertexFormat;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_LUX_CPARTICLELINERENDERER
