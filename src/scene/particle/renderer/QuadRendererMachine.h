#ifndef INCLUDED_PARTICLE_QUAD_RENDERER_MACHINE_H
#define INCLUDED_PARTICLE_QUAD_RENDERER_MACHINE_H
#include "video/mesh/Geometry.h"
#include "video/VertexTypes.h"
#include "video/Renderer.h"
#include "video/Pass.h"

#include "scene/particle/ParticleRenderer.h"
#include "scene/particle/Particle.h"

#include "math/Matrix4.h"

namespace lux
{
namespace scene
{
class ParticleModel;
class ParticleGroupData;

class QuadRendererMachine : public RendererMachine
{
public:
	QuadRendererMachine();
	~QuadRendererMachine();

	void Render(video::Renderer* videoRenderer, ParticleGroupData* group, ParticleRenderer* renderer);
	StrongRef<ParticleRenderer> CreateRenderer();
	core::Name GetReferableType() const;

private:
	bool PrecomputeOrientation(const math::Matrix4& invModelView);
	void ComputeGlobalOrientation();
	void ComputeLocalOrientation(const Particle& particle);
	void SetIndexBuffer(video::IndexBuffer* indexBuffer, int from, int to);
	void CreateBuffers(ParticleGroupData* group);

	void RenderQuad_Scaled(video::Vertex3D* vertices, const Particle& particle);
	void RenderQuad_ScaledRotated(video::Vertex3D* vertices, const Particle& particle);

private:
	video::VideoDriver* m_Driver;
	StrongRef<QuadRenderer> m_Data;
	StrongRef<ParticleModel> m_Model;

	StrongRef<video::Geometry> m_Buffer;

	math::Vector3F m_HelpLook;
	math::Vector3F m_HelpUp;

	math::Vector3F m_Up;
	math::Vector3F m_Look;
	math::Vector3F m_Side;
	bool m_IsRotationEnabled;

	video::Pass m_DefaultPass;
	video::Pass m_EmitPass;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_CPARTICLERENDERER