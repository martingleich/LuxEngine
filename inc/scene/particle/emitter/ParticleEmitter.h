#ifndef INCLUDED_PARTICLEEMITTER_H
#define INCLUDED_PARTICLEEMITTER_H
#include "scene/particle/Particle.h"

#include "scene/Component.h"
#include "scene/zones/Zone.h"

#include "core/lxRandom.h"
#include "math/Transformation.h"

namespace lux
{
namespace scene
{
class ParticleModel;

class ParticleEmitter : public Component
{
public:
	ParticleEmitter() :
		m_IsGlobal(false),
		m_IsActive(true),
		m_Flow(0.0f),
		m_ForceMin(0.0f),
		m_ForceMax(0.0f),
		m_Count(0.0f)
	{

	}

	ParticleEmitter(const ParticleEmitter& other) :
		m_IsGlobal(other.m_IsGlobal),
		m_IsActive(other.m_IsActive),
		m_Flow(other.m_Flow),
		m_ForceMin(other.m_ForceMin),
		m_ForceMax(other.m_ForceMax),
		m_Count(0),
		m_Model(other.m_Model)
	{
		if(other.m_Zone)
			m_Zone = other.m_Zone->Clone();
		else
			m_Zone = nullptr;
	}

	void SetZone(Zone* pZone)
	{
		m_Zone = pZone;
	}

	StrongRef<Zone> GetZone()
	{
		return m_Zone;
	}

	void SetFlow(float flow)
	{
		m_Flow = flow;
	}

	float GetFlow() const
	{
		return m_Flow;
	}

	void SetForce(float force)
	{
		SetForce(force, force);
	}

	void SetForce(float min, float max)
	{
		m_ForceMin = min;
		m_ForceMax = max;
	}

	float GetMaxForce() const
	{
		return m_ForceMax;
	}

	float GetMinForce() const
	{
		return m_ForceMin;
	}

	void SetActive(bool active)
	{
		m_IsActive = active;
	}

	bool IsActive() const
	{
		return m_IsActive;
	}

	u32 GetEmitCount(float secsPassed) const
	{
		m_Count += m_Flow * secsPassed;
		if(m_Count > 0.0f) {
			u32 out = (u32)m_Count;
			m_Count -= std::floor(m_Count);
			return out;
		} else {
			return 0;
		}
	}

	virtual void Begin(const math::Transformation& trans) const
	{
		m_Transform = trans;
	}

	void Emit(Particle& particle) const
	{
		if(m_Zone)
			particle.position = m_Position + m_Zone->GetPointInside(m_Rand);
		else
			particle.position = m_Position;

		GenerateVelocity(particle, m_Rand.GetFloat(m_ForceMin, m_ForceMax));

		particle.position = m_Transform.TransformPoint(particle.position);
		particle.velocity = particle.velocity;
	}

	virtual void GenerateVelocity(Particle& particle, float speed) const = 0;

	bool IsGlobal() const
	{
		return m_IsGlobal;
	}
	void SetGlobal(bool isGlobal)
	{
		m_IsGlobal = isGlobal;
	}

	void SetPosition(const math::Vector3F& pos)
	{
		m_Position = pos;
	}

	void SetModel(ParticleModel* m)
	{
		m_Model = m;
	}
	ParticleModel* GetModel() const
	{
		return m_Model;
	}

protected:
	core::Randomizer m_Rand;

	math::Vector3F m_Position;
	bool m_IsGlobal;
	mutable math::Transformation m_Transform;

	bool m_IsActive;
	float m_Flow;

	float m_ForceMin;
	float m_ForceMax;

	mutable float m_Count;

	StrongRef<Zone> m_Zone;

	StrongRef<ParticleModel> m_Model;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_IPARTICLEEMITTER
