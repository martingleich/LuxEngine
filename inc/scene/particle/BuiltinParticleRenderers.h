#ifndef INCLUDED_LUX_BUILTIN_PARTICLE_RENDERERS_H
#define INCLUDED_LUX_BUILTIN_PARTICLE_RENDERERS_H
#include "scene/particle/ParticleRenderer.h"
#include "core/ReferableFactory.h"

namespace lux
{
namespace scene
{

enum class ELookOrientation
{
	CameraPlane,
	Axis,
	CameraPoint,
	Point,
};

enum class EUpOrientation
{
	Camera,
	Direction,
	Axis,
	Point
};

enum class ELockedAxis
{
	Look,
	Up
};

class QuadRenderer : public ParticleRenderer
{
	LX_REFERABLE_MEMBERS_API(QuadRenderer, LUX_API);
public:
	QuadRenderer() :
		LookOrient(ELookOrientation::CameraPlane),
		UpOrient(EUpOrientation::Direction),
		LockedAxis(ELockedAxis::Look),
		Scaling(1.0f, 1.0f),
		ScaleLengthSpeedSq(0.0f),
		EmitLight(false)
	{
		m_Machine = core::ReferableFactory::Instance()->CreateShared(core::Name("lux.particlerenderermachine.Quad")).StaticCastStrong<RendererMachine>();
	}

	StrongRef<RendererMachine> GetMachine() const
	{
		return m_Machine;
	}

public:
	ELookOrientation LookOrient;
	EUpOrientation UpOrient;
	ELockedAxis LockedAxis;

	math::Vector3F LookValue;
	math::Vector3F UpValue;

	math::Vector2F Scaling;
	StrongRef<video::SpriteBank> SpriteBank;
	video::SpriteBank::Sprite DefaultSprite;

	float ScaleLengthSpeedSq;

	bool EmitLight;

private:
	StrongRef<RendererMachine> m_Machine;
};

class LineRenderer : public ParticleRenderer
{
	LX_REFERABLE_MEMBERS_API(LineRenderer, LUX_API);
public:
	LineRenderer() :
		ScaleSpeed(false),
		Length(1.0f),
		EmitLight(false),
		DefaultDir(math::Vector3F::UNIT_Y)
	{
		m_Machine = core::ReferableFactory::Instance()->CreateShared(core::Name("lux.particlerenderermachine.Line")).StaticCastStrong<RendererMachine>();
	}

	StrongRef<RendererMachine> GetMachine() const
	{
		return m_Machine;
	}

public:
	bool ScaleSpeed;
	float Length;
	bool EmitLight;

	math::Vector3F DefaultDir;

private:
	StrongRef<RendererMachine> m_Machine;
};

class PointRenderer : public ParticleRenderer
{
	LX_REFERABLE_MEMBERS_API(PointRenderer, LUX_API);
public:
	PointRenderer()
	{
		m_Machine = core::ReferableFactory::Instance()->CreateShared(core::Name("lux.particlerenderermachine.Point")).StaticCastStrong<RendererMachine>();
	}

	StrongRef<RendererMachine> GetMachine() const
	{
		return m_Machine;
	}
public:
	bool EmitLight = false;
private:
	StrongRef<RendererMachine> m_Machine;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_BUILTIN_PARTICLE_RENDERERS_H