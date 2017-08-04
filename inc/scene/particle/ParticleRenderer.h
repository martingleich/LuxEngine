#ifndef INCLUDED_PARTICLERENDERER_H
#define INCLUDED_PARTICLERENDERER_H
#include "video/SpriteBank.h"
#include "math/vector3.h"
#include "core/lxName.h"

namespace lux
{
namespace video
{
class Renderer;
}
namespace scene
{

class RendererMachine;
class ParticleRenderer : public ReferenceCounted
{
public:
	ParticleRenderer(RendererMachine* machine) :
		mMachine(machine)
	{
	}

	virtual ~ParticleRenderer() {}
	inline core::Name GetType() const;
	StrongRef<RendererMachine> GetMachine() const
	{
		return mMachine;
	}

private:
	StrongRef<RendererMachine> mMachine;
};

class ParticleGroupData;
class RendererMachine : public ReferenceCounted
{
public:
	virtual ~RendererMachine() {}

	virtual void Render(video::Renderer* videoRenderer, ParticleGroupData* group, ParticleRenderer* renderer = nullptr) = 0;
	virtual StrongRef<ParticleRenderer> CreateRenderer() = 0;
	virtual core::Name GetType() const = 0;
};

inline core::Name ParticleRenderer::GetType() const
{
	return mMachine->GetType();
}

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
public:
	QuadRenderer(RendererMachine* m) :
		ParticleRenderer(m),
		LookOrient(ELookOrientation::CameraPlane),
		UpOrient(EUpOrientation::Direction),
		LockedAxis(ELockedAxis::Look),
		Scaling(1.0f, 1.0f),
		ScaleLengthSpeedSq(0.0f),
		EmitLight(false)
	{
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
};

class LineRenderer : public ParticleRenderer
{
public:
	LineRenderer(RendererMachine* m) :
		ParticleRenderer(m),
		ScaleSpeed(false),
		Length(1.0f),
		EmitLight(false),
		DefaultDir(math::Vector3F::UNIT_Y)
	{
	}

public:
	bool ScaleSpeed;
	float Length;
	bool EmitLight;

	math::Vector3F DefaultDir;
};

class PointRenderer : public ParticleRenderer
{
public:
	PointRenderer(RendererMachine* m) :
		ParticleRenderer(m)
	{
	}

	bool EmitLight = false;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_IPARTICLERENDERER