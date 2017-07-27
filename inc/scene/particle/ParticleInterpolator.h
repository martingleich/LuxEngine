#ifndef INCLUDED_PARTICLE_INTERPOLATOR_H
#define INCLUDED_PARTICLE_INTERPOLATOR_H
#include "core/ReferenceCounted.h"

#include "core/lxArray.h"
#include "core/lxAlgorithm.h"
#include "math/Curve.h"

#include "Particle.h"

namespace lux
{
namespace scene
{

class ParticleInterpolator : public ReferenceCounted
{
public:
	enum class EVariable
	{
		Age,
		SqrSpeed,
		Param,
	};

public:
	ParticleInterpolator() :
		m_Variable(EVariable::Age),
		m_TimeScaleVar(0.0f),
		m_UseAbsTime(false)
	{
	}

	math::Curve<float>& GetCurve()
	{
		return m_Curve;
	}

	const math::Curve<float>& GetCurve() const
	{
		return m_Curve;
	}

	void SetVariable(EVariable var)
	{
		m_Variable = var;
	}

	EVariable GetVariable() const
	{
		return m_Variable;
	}

	void SetAbsoluteTime(bool useAbsTime)
	{
		m_UseAbsTime = useAbsTime;
	}

	bool GetAbsoluteTime() const
	{
		return m_UseAbsTime;
	}

	void SetTimeScaleVariation(float variation)
	{
		m_TimeScaleVar = variation;
	}

	float GetTimeScaleVariation() const
	{
		return m_TimeScaleVar;
	}

	float Interpolate(const Particle& particle, float timeScale) const
	{
		const float t = timeScale * CalcVariable(particle);

		// Throw their dirty errors right back at them.
		if(std::isnan(t))
			return t;

		return m_Curve.GetValue(t);
	}

private:
	float CalcVariable(const Particle& p) const
	{
		switch(m_Variable) {
		case EVariable::Age:
			if(m_UseAbsTime)
				return p.age;
			else
				return p.age / (p.age + p.life);
		case EVariable::SqrSpeed:
			return p.velocity.GetLengthSq();
		default:
			// TODO: Support any parameter
			return 0.0f;
		}
	}

private:
	math::Curve<float> m_Curve;
	EVariable m_Variable;
	float m_TimeScaleVar;
	bool m_UseAbsTime;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_IPARTICLEINTERPOLATOR
