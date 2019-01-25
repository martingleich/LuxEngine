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

class QuadParticleRenderer : public ParticleRenderer
{
	LX_REFERABLE_MEMBERS_API(QuadParticleRenderer, LUX_API);
public:
	class Machine : public ReferenceCounted
	{
	public:
		virtual void Render(video::Renderer* videoRenderer, ParticleGroupData* group, QuadParticleRenderer* renderer) = 0;
	};
	QuadParticleRenderer();
	void Render(video::Renderer* videoRenderer, ParticleGroupData* group) override
	{
		m_Machine->Render(videoRenderer, group, this);
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
	StrongRef<Machine> m_Machine;
};

class LineParticleRenderer : public ParticleRenderer
{
	LX_REFERABLE_MEMBERS_API(LineParticleRenderer, LUX_API);
public:
	class Machine : public ReferenceCounted
	{
	public:
		virtual void Render(video::Renderer* videoRenderer, ParticleGroupData* group, LineParticleRenderer* renderer) = 0;
	};
	LineParticleRenderer();
	void Render(video::Renderer* videoRenderer, ParticleGroupData* group) override
	{
		m_Machine->Render(videoRenderer, group, this);
	}

public:
	bool ScaleSpeed;
	float Length;
	bool EmitLight;

	math::Vector3F DefaultDir;

private:
	StrongRef<Machine> m_Machine;
};

class PointParticleRenderer : public ParticleRenderer
{
	LX_REFERABLE_MEMBERS_API(PointParticleRenderer, LUX_API);
public:
	class Machine : public ReferenceCounted
	{
	public:
		virtual void Render(video::Renderer* videoRenderer, ParticleGroupData* group, PointParticleRenderer* renderer) = 0;
	};
	PointParticleRenderer();
	void Render(video::Renderer* videoRenderer, ParticleGroupData* group) override
	{
		m_Machine->Render(videoRenderer, group, this);
	}

public:
	bool EmitLight = false;
private:
	StrongRef<Machine> m_Machine;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_BUILTIN_PARTICLE_RENDERERS_H