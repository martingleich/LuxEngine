#ifndef INCLUDED_LUX_BUILTIN_EMITTERS_H
#define INCLUDED_LUX_BUILTIN_EMITTERS_H
#include "scene/particle/ParticleEmitter.h"

namespace lux
{
namespace scene
{

class StraightEmitter : public ParticleEmitter
{
	LX_REFERABLE_MEMBERS_API(StraightEmitter, LUX_API);
public:
	StraightEmitter() :
		m_Direction(math::Vector3F::UNIT_X)
	{
	}

	StraightEmitter(const math::Vector3F& dir)
	{
		SetDirection(dir);
	}

	void SetDirection(const math::Vector3F& dir)
	{
		m_Direction = dir;
		m_Direction.Normalize();
	}

	const math::Vector3F& GetDirection() const
	{
		return m_Direction;
	}

protected:
	void GenerateVelocity(core::Randomizer& rand, Particle& particle, float speed)
	{
		LUX_UNUSED(rand);
		particle.velocity = speed * m_Direction;
	}

private:
	math::Vector3F m_Direction;
};

class NormalEmitter : public ParticleEmitter
{
	LX_REFERABLE_MEMBERS_API(NormalEmitter, LUX_API);
protected:
	void GenerateVelocity(core::Randomizer& rand, Particle& particle, float speed)
	{
		LUX_UNUSED(rand);
		math::Vector3F s = m_Zone->GetNormal(particle.position);
		particle.velocity = speed * s;
	}
};

class LookAtEmitter : public ParticleEmitter
{
	LX_REFERABLE_MEMBERS_API(LookAtEmitter, LUX_API);
public:
	LookAtEmitter()
	{
	}
	LookAtEmitter(const math::Vector3F& point)
	{
		SetPoint(point);
	}

	void SetPoint(const math::Vector3F& point) { m_Point = point; }
	const math::Vector3F& GetPoint() const { return m_Point; }

protected:
	void GenerateVelocity(core::Randomizer& rand, Particle& particle, float speed)
	{
		LUX_UNUSED(rand);
		math::Vector3F dir = (m_Point - particle.position).Normal();
		particle.velocity = speed * dir;
	}

private:
	math::Vector3F m_Point;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_BUILTIN_EMITTERS_H