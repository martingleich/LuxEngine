#ifndef INCLUDED_PARTICLERENDERER_LINE_IMPL_H
#define INCLUDED_PARTICLERENDERER_LINE_IMPL_H
#include "video/VertexTypes.h"
#include "video/VertexFormats.h"
#include "video/Renderer.h"
#include "video/MaterialRenderer.h"

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
		math::vector3f position;
		video::Color color;
	};

public:
	LineRendererMachine();
	~LineRendererMachine();

	void Render(video::Renderer* videoRenderer, ParticleGroupData* group, ParticleRenderer* renderer);
	core::Name GetType() const;
	StrongRef<ParticleRenderer> CreateRenderer();

private:
	void CreateBuffer(ParticleGroupData* group);

private:
	StrongRef<LineRenderer> m_Data;

	core::Array<LineVertex> m_Vertices;

	StrongRef<video::Material> m_BaseMaterial;
	StrongRef<video::Material> m_EmitMaterial;

	video::VertexFormat m_VertexFormat;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_CPARTICLELINERENDERER
