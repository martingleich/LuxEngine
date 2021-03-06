#ifndef INCLUDED_LUX_BUILTIN_AFFECTORS_H
#define INCLUDED_LUX_BUILTIN_AFFECTORS_H
#include "scene/particle/ParticleAffector.h"

namespace lux
{
namespace scene
{

class SwirlAffector : public ParticleAffector
{
	LX_REFERABLE_MEMBERS_API(SwirlAffector, LUX_API);
public:
	SwirlAffector() :
		m_FixedAttractionSpeed(true),
		m_AttractionSpeed(1.0f),
		m_FixedAngularSpeed(true),
		m_AngularSpeed(1.0f),
		m_EyeRadius(1.0f),
		m_KillRadius(-1.0f),
		m_ParticleKilling(true)
	{
	}

	void Begin(const math::Transformation& trans)
	{
		ParticleAffector::Begin(trans);

		m_TransCenter = m_AbsTransform.TransformPoint(math::Vector3F::ZERO);
		m_TransAxis = m_AbsTransform.TransformDir(math::Vector3F::UNIT_Y) / trans.scale;

		if(m_KillRadius < m_EyeRadius || m_KillRadius <= 0)
			m_KillRadius = m_EyeRadius;

		lxAssert(m_KillRadius >= m_EyeRadius);
	}

	void Apply(Particle& particle, float secsPassed)
	{
		const math::Vector3F delta = particle.position - m_TransCenter;
		const float planeDist = m_TransAxis.Dot(delta); // Distance to the main rotation plane.

		const math::Vector3F rotCenter = m_TransCenter + planeDist * m_TransAxis;

		const float dist = rotCenter.GetDistanceTo(particle.position);

		if(dist <= m_KillRadius) {
			if(m_ParticleKilling)
				particle.Kill();
			return;
		}

		const float realSpeed = m_FixedAttractionSpeed ? m_AttractionSpeed : (dist - m_EyeRadius)*m_AttractionSpeed;
		const float newRadius = dist - realSpeed*secsPassed;
		if(newRadius <= m_KillRadius) {
			if(m_ParticleKilling)
				particle.Kill();
			return;
		}

		const float angleSpeed = (m_FixedAngularSpeed ? m_AngularSpeed : m_AngularSpeed / dist);
		const float deltaAngle = secsPassed * angleSpeed;

		const math::Vector3F normal = (particle.position - rotCenter) / dist;
		const math::Vector3F tangent = normal.Cross(m_TransAxis);

		const math::Vector3F newPos = rotCenter + newRadius * (normal*std::cos(deltaAngle) + tangent*std::sin(deltaAngle));

		// The velocity is calculated to transport it directly to the correct position
		math::Vector3F newVelocity = (newPos - particle.position) / secsPassed;
		math::Vector3F orthoVelocity = m_TransAxis * particle.velocity.Dot(m_TransAxis);
		particle.velocity = orthoVelocity + newVelocity;
	}

	void SetFixedAttractionSpeed(bool fixed) { m_FixedAttractionSpeed = fixed; }
	void SetAttractionSpeed(float speed) { m_AttractionSpeed = speed; }
	void SetFixedAngularSpeed(bool fixed) { m_FixedAngularSpeed = fixed; }
	void SetAngularSpeed(float speed) { m_AngularSpeed = speed; }
	void SetEyeRadius(float radius) { m_EyeRadius = radius; }
	void SetKillRadius(float radius) { m_KillRadius = radius; }
	void SetParticleKilling(bool kill) { m_ParticleKilling = kill; }

	bool GetFixedAttractionSpeed() const { return m_FixedAttractionSpeed; }
	float GetAttractionSpeed() const { return m_AttractionSpeed; }
	bool GetFixedAngularSpeed() const { return m_FixedAngularSpeed; }
	float GetAngularSpeed() const { return m_AngularSpeed; }
	float GetEyeRadius() const { return m_EyeRadius; }
	float GetKillRadius() const { return m_KillRadius; }
	bool GetParticleKilling() const { return m_ParticleKilling; }

private:
	bool m_FixedAttractionSpeed; // Is the attraction speed always the same, other the speed is directly proportional to the distance to the eye of the swirl.
	float m_AttractionSpeed; // The speed of attraction to the center, the number of units each particle get closer to the center per second.
	bool m_FixedAngularSpeed; // Is the angular speed always the same, otherwise the speed is indirect proportional to the distance to the center.
	float m_AngularSpeed; // The rotation speed of the swirl.

	float m_EyeRadius; // The radius of the "eye" of the swirl, particles can't get nearer to the center than this.
	float m_KillRadius;
	bool m_ParticleKilling; // Particles which reach the "eye" of the swirl are destroied.

	math::Vector3F m_TransCenter;
	math::Vector3F m_TransAxis; // normalized
};

class LinearForceAffector : public ParticleAffector
{
	LX_REFERABLE_MEMBERS_API(LinearForceAffector, LUX_API);
public:
	LinearForceAffector() :
		m_Direction(math::Vector3F::UNIT_Y)
	{}

	LinearForceAffector(const math::Vector3F& force) :
		m_Direction(force)
	{
	}

	void Begin(const math::Transformation& trans)
	{
		ParticleAffector::Begin(trans);
		m_TransDirection = m_AbsTransform.TransformDir(m_Direction);
	}

	void Apply(Particle& particle, float secsPassed)
	{
		particle.velocity += m_TransDirection*secsPassed;
	}

private:
	math::Vector3F m_Direction;
	math::Vector3F m_TransDirection;
};

}
}

#endif // #ifndef INCLUDED_LUX_BUILTIN_AFFECTORS_H