#ifndef INCLUDED_PARTICLERENDERER_LINE_IMPL_H
#define INCLUDED_PARTICLERENDERER_LINE_IMPL_H
#include "video/VertexTypes.h"
#include "video/VertexFormat.h"
#include "video/Renderer.h"
#include "video/Pass.h"

#include "scene/particle/ParticleRenderer.h"
#include "scene/particle/Particle.h"

namespace lux
{
namespace scene
{
class ParticleModel;
class ParticleGroupData;

class LineRendererMachine : public RendererMachine
{
private:
	struct LineVertex
	{
		math::Vector3F position;
		video::Color color;
	};

public:
	LineRendererMachine();
	~LineRendererMachine();

	void Render(video::Renderer* videoRenderer, ParticleGroupData* group, ParticleRenderer* renderer);
	StrongRef<ParticleRenderer> CreateRenderer();

	core::Name GetReferableType() const;

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

#endif // !INCLUDED_CPARTICLELINERENDERER
