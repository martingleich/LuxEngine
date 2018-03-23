#ifndef INCLUDED_PARTICLE_MODEL_H
#define INCLUDED_PARTICLE_MODEL_H
#include "core/lxRandom.h"

#include "video/Color.h"
#include "video/SpriteBank.h"

#include "scene/particle/ParticleRenderer.h"
#include "scene/Curve.h"

namespace lux
{

namespace scene
{
struct Particle
{
	// Often accessed parameters(many times per frame)
	math::Vector3F position;
	math::Vector3F velocity;
	float age;
	float life;

	// Rarely accessed parameters
	float* params;

	Particle() :
		params(nullptr)
	{
	}

	void Kill()
	{
		age += life;
		life = 0.0f;
	}

	float& Param(int off)
	{
		return params[off];
	}

	const float& Param(int off) const
	{
		return params[off];
	}
};

class ParticleParam
{
public:
	enum EParameter : u8
	{
		Red = 0,
		Green = 1,
		Blue = 2,
		Alpha = 3,

		Size = 4,

		Angle = 5,
		RotSpeed = 6,

		Sprite = 7,

		Custom1 = 8,
		Custom2 = 9,
		Custom3 = 10,

		COUNT = 11
	};

	enum class EState : u8
	{
		Disabled,
		Constant,
		Fixed,
		Random,
		Changing,
		ChangingRandom,
		Interpolated
	};

public:
	ParticleParam() :
		state(EState::Disabled)
	{
	}
	ParticleParam(EState _state) :
		state(_state)
	{
	}

	static ParticleParam Disabled()
	{
		return ParticleParam();
	}
	static ParticleParam Fixed(float value)
	{
		auto param = ParticleParam(EState::Fixed);
		param.values[0] = value;
		return param;
	}
	static ParticleParam Random(float min, float max)
	{
		auto param = ParticleParam(EState::Random);
		param.values[0] = min;
		param.values[1] = max;
		return param;
	}
	static ParticleParam Changing(float from, float to)
	{
		auto param = ParticleParam(EState::Changing);
		param.values[0] = from;
		param.values[1] = to;
		return param;
	}
	static ParticleParam ChangingRandom(float minFrom, float maxFrom, float minTo, float maxTo)
	{
		auto param = ParticleParam(EState::ChangingRandom);
		param.values[0] = minFrom;
		param.values[1] = maxFrom;
		param.values[2] = minTo;
		param.values[3] = maxTo;
		return param;
	}

	bool IsEnabled() const
	{
		return state != EState::Disabled;
	}

public:
	EState state;
	float values[4];

	StrongRef<scene::Curve> curve;
};

class ParticleModel : public ReferenceCounted
{
public:
	LUX_API ParticleModel();
	LUX_API ~ParticleModel();

	LUX_API void InitParticle(Particle& particle) const;
	LUX_API void UpdateParticle(Particle& particle, float secsPassed) const;

	LUX_API void SetLifetime(const core::Distribution& time);
	LUX_API const core::Distribution& GetLifetime() const;

	LUX_API void SetParam(ParticleParam::EParameter param, const ParticleParam& value);
	LUX_API const ParticleParam& GetParam(ParticleParam::EParameter param) const;

	void SetRGB(const video::ColorF& color)
	{
		SetParam(ParticleParam::Red, ParticleParam::Fixed(color.r));
		SetParam(ParticleParam::Green, ParticleParam::Fixed(color.g));
		SetParam(ParticleParam::Blue, ParticleParam::Fixed(color.b));
	}

	int GetParamOffset(ParticleParam::EParameter param) const
	{
		return IsEnabled(param) ? GetInternalParam(param).value_offset : -1;
	}
	LUX_API float ReadValue(const Particle& p, ParticleParam::EParameter param) const;
	bool IsEnabled(ParticleParam::EParameter param) const
	{
		return GetParam(param).IsEnabled();
	}
	u32 GetFloatParticleParams() const
	{
		return m_ParticleDataSize;
	}

	void SetGravity(const math::Vector3F& v)
	{
		m_Gravity = v;
	}
	const math::Vector3F& GetGravity() const
	{
		return m_Gravity;
	}

	LUX_API StrongRef<ParticleRenderer> SetRenderMode(core::Name type);
	StrongRef<ParticleRenderer> SetRenderer(ParticleRenderer* r)
	{
		m_Renderer = r;
		return m_Renderer;
	}
	StrongRef<ParticleRenderer> GetRenderer() const
	{
		return m_Renderer;
	}

	void SetSmoothingModel(int model) { m_SmoothingModel = model; }
	int GetSmoothingModel() const { return m_SmoothingModel; }

	int GetChangeId() const { return m_ChangeId; }
	int GetParamStateChangeId() const { return m_ParamTypeChangeId; }

private:
	struct InternalParam
	{
		ParticleParam param;
		s8 value_offset; // The offset of the current value in the particle data.
		s8 update_offset; // The offset of the update data int particle data.
	};

private:
	InternalParam& GetInternalParam(ParticleParam::EParameter param)
	{
		lxAssert((u32)param < (u32)ParticleParam::COUNT);
		return m_Params[(u32)param];
	}

	const InternalParam& GetInternalParam(ParticleParam::EParameter param) const
	{
		lxAssert((u32)param < (u32)ParticleParam::COUNT);
		return m_Params[(u32)param];
	}

private:
	static const int PARAM_COUNT = ParticleParam::EParameter::COUNT;
	static const float DEFAULT[PARAM_COUNT];

	StrongRef<ParticleRenderer> m_Renderer; //!< The renderer used to display the particles

	InternalParam m_Params[PARAM_COUNT];

	core::Distribution m_Lifetime;
	int m_SmoothingModel;
	math::Vector3F m_Gravity; //!< The gravity used by the group

	int m_ParticleDataSize;

	mutable core::Randomizer m_Randomizer;

	int m_ChangeId;
	int m_ParamTypeChangeId;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_PARTICLE_MODEL_H
