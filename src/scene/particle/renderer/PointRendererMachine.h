#ifndef INCLUDED_PARTICLERENDERER_POINT_MACHINE_H
#define INCLUDED_PARTICLERENDERER_POINT_MACHINE_H
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

class PointRendererMachine : public RendererMachine
{
private:
	struct PointVertex
	{
		math::Vector3F position;
		video::Color color;
	};

public:
	PointRendererMachine();
	~PointRendererMachine();

	void Render(video::Renderer* videoRenderer, ParticleGroupData* group, ParticleRenderer* renderer);

	core::Name GetType() const;
	StrongRef<ParticleRenderer> CreateRenderer();


private:
	void CreateBuffer(ParticleGroupData* group);

private:
	core::Array<PointVertex> m_Vertices;

	StrongRef<video::Material> m_BaseMaterial;
	StrongRef<video::Material> m_EmitMaterial;

	video::VertexFormat m_VertexFormat;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_CPARTICLELINERENDERER
