#ifndef INCLUDED_PARTICLE_MODEL_H
#define INCLUDED_PARTICLE_MODEL_H
#include "video/Color.h"
#include "video/SpriteBank.h"

#include "core/lxRandom.h"

#include "Particle.h"
#include "ParticleRenderer.h"

namespace lux
{
namespace scene
{
class ParticleInterpolator;

enum class EParticleParamState
{
	Disabled,
	Constant,
	Fixed,
	Random,
	Changing,
	ChangingRandom,
	Interpolated
};

class ParticleParamStates
{
public:
	ParticleParamStates()
	{
		Clear();
	}

	void Clear()
	{
		for(size_t i = 0; i < (size_t)Particle::EParameter::COUNT; ++i)
			states[i] = EParticleParamState::Disabled;
	}

	ParticleParamStates& Disabled(Particle::EParameter p) { states[(size_t)p] = EParticleParamState::Disabled; return *this; }
	ParticleParamStates& Fixed(Particle::EParameter p) { states[(size_t)p] = EParticleParamState::Constant; return *this; }
	ParticleParamStates& Constant(Particle::EParameter p) { states[(size_t)p] = EParticleParamState::Fixed; return *this; }
	ParticleParamStates& Random(Particle::EParameter p) { states[(size_t)p] = EParticleParamState::Random; return *this; }
	ParticleParamStates& Changing(Particle::EParameter p) { states[(size_t)p] = EParticleParamState::Changing; return *this; }
	ParticleParamStates& ChangingRandom(Particle::EParameter p) { states[(size_t)p] = EParticleParamState::ChangingRandom; return *this; }
	ParticleParamStates& Interpolated(Particle::EParameter p) { states[(size_t)p] = EParticleParamState::Interpolated; return *this; }

	EParticleParamState states[(size_t)Particle::EParameter::COUNT];
};

class ParticleModel : public ReferenceCounted
{
public:
	LUX_API ParticleModel();

	LUX_API void SetParamStates(const ParticleParamStates& states);

	LUX_API void ResetToDefault();

	LUX_API void SetLifeTime(float min, float max);
	LUX_API void GetLifeTime(float& min, float& max);

	LUX_API void SetImmortal(bool immortal);
	LUX_API bool IsImmortal() const;

	bool IsEnabled(Particle::EParameter param) const
	{
		return ((int)GetParamState(param) > (int)EParticleParamState::Constant);
	}

	LUX_API EParticleParamState GetParamState(Particle::EParameter param) const;

	float ReadValue(const Particle& p, Particle::EParameter param)
	{
		int off = GetOffset(param);
		if(off != -1)
			return p.Param(off);
		else
			return m_Params[(u32)param].values[0];
	}

	/*
	Enabled -> 1 Wert				Fixed
	Mutable -> 2 Werte				Begin End
	Random -> 2 Werte				Min	Max
	Mutable & Random -> 4 Werte		BeginMin EndMin BeginMax EndMax
	*/
	void SetValues(Particle::EParameter param, float a, float b = 0.0f, float c = 0.0f, float d = 0.0f)
	{
		const float f[4] = {a, b, c, d};
		SetValues(param, f);
	}

	void SetValues(Particle::EParameter param, int a, int b = 0, int c = 0, int d = 0)
	{
		const float f[4] = {float(a), float(b), float(c), float(d)};
		SetValues(param, f);
	}

	void SetValues(Particle::EParameter param, video::SpriteBank::Sprite a, video::SpriteBank::Sprite b = 0, video::SpriteBank::Sprite c = 0, video::SpriteBank::Sprite d = 0)
	{
		SetValues(param, a.id, b.id, c.id, d.id);
	}

	void SetColor(
		const video::Colorf& a,
		const video::Colorf& b = video::Colorf(),
		const video::Colorf& c = video::Colorf(),
		const video::Colorf& d = video::Colorf())
	{
		SetValues(Particle::EParameter::Red, a.r, b.r, c.r, d.r);
		SetValues(Particle::EParameter::Blue, a.b, b.b, c.b, d.b);
		SetValues(Particle::EParameter::Green, a.g, b.g, c.g, d.g);
	}

	LUX_API void SetValues(Particle::EParameter param, const float* values);
	LUX_API void GetValues(Particle::EParameter param, float* values) const;

	LUX_API void SetInterpolator(Particle::EParameter param, ParticleInterpolator* interpolator);
	LUX_API StrongRef<ParticleInterpolator> GetInterpolator(Particle::EParameter param) const;

	LUX_API int GetOffset(Particle::EParameter param);
	LUX_API float GetDefaultValue(Particle::EParameter param) const;
	LUX_API u32 GetBytesParticleParams() const;
	LUX_API u32 GetFloatParticleParams() const;

	LUX_API void InitParticle(Particle& particle) const;
	LUX_API void UpdateParticle(Particle& particle, float secsPassed) const;

	LUX_API void SetGravity(const math::Vector3F& v);
	LUX_API const math::Vector3F& GetGravity() const;

	LUX_API StrongRef<ParticleRenderer> SetRenderer(ParticleRenderer* r);
	LUX_API StrongRef<ParticleRenderer> GetRenderer();

	LUX_API StrongRef<ParticleRenderer> SetRenderMode(core::Name type);

	//! Set the maximal number of particles in this models
	/**
	Use 0 to enable automatic calculation of the particle count, based
	on the emitters and the average lifetime.
	*/
	void SetCapacity(u32 capacity)
	{
		m_Capacity = capacity;
	}

	u32 GetCapacity() const
	{
		return m_Capacity;
	}

	float GetAvgLifeTime() const
	{
		return 0.5f*(m_LifeTimeMax + m_LifeTimeMin);
	}

private:
	struct Param
	{
		Particle::EParameter param;
		EParticleParamState state;
		float values[4];
		StrongRef<ParticleInterpolator> interpolator;
		s8 offset;
	};

private:
	Param& GetParam(Particle::EParameter param)
	{
		lxAssert((u32)param < (u32)Particle::EParameter::COUNT);
		return m_Params[(u32)param];
	}

	const Param& GetParam(Particle::EParameter param) const
	{
		lxAssert((u32)param < (u32)Particle::EParameter::COUNT);
		return m_Params[(u32)param];
	}

private:
	static const u32 PARAM_COUNT = (u32)Particle::EParameter::COUNT;
	static const float DEFAULT[PARAM_COUNT];

	float m_LifeTimeMin;
	float m_LifeTimeMax;

	bool  m_IsImmortal;

	Param m_Params[PARAM_COUNT];
	u32 m_ParamCount;
	u32 m_StaticCount;
	u32 m_ChangingCount;
	u32 m_InterpolatedCount;

	// Contains the base offset for the fixed value of a parameter.
	// At first for the first, second, third ... changing parameter, then for the last, before-last, ..., second, first interpolated parameter.
	u8 m_BaseOffset[PARAM_COUNT];

	u32 m_ParticleDataSize;

	mutable core::Randomizer m_Randomizer;

	StrongRef<ParticleRenderer> m_Renderer; //!< The renderer used to display the particles
	math::Vector3F m_Gravity; //!< The gravity used by the group

	u32 m_Capacity; //!< The maximal number of particles of this model or 0 to auto calculate the capacititys
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_PARTICLE_MODEL_H
