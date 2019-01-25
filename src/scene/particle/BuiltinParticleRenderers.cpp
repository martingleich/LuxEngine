#include "scene/particle/BuiltinParticleRenderers.h"
#include "scene/particle/renderer/QuadRendererMachine.h"
#include "scene/particle/renderer/PointRendererMachine.h"
#include "scene/particle/renderer/LineRendererMachine.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::QuadParticleRenderer, "lux.particlerenderer.Quad");
LX_REFERABLE_MEMBERS_SRC(lux::scene::LineParticleRenderer, "lux.particlerenderer.Line");
LX_REFERABLE_MEMBERS_SRC(lux::scene::PointParticleRenderer, "lux.particlerenderer.Point");

namespace lux
{
namespace scene
{
QuadParticleRenderer::QuadParticleRenderer() :
	LookOrient(ELookOrientation::CameraPlane),
	UpOrient(EUpOrientation::Direction),
	LockedAxis(ELockedAxis::Look),
	Scaling(1.0f, 1.0f),
	ScaleLengthSpeedSq(0.0f),
	EmitLight(false)
{
	m_Machine = QuadRendererMachine::GetShared();
}

LineParticleRenderer::LineParticleRenderer() :
	ScaleSpeed(false),
	Length(1.0f),
	EmitLight(false),
	DefaultDir(math::Vector3F::UNIT_Y)
{
	m_Machine = LineRendererMachine::GetShared();
}
PointParticleRenderer::PointParticleRenderer()
{
	m_Machine = PointRendererMachine::GetShared();
}

}
}