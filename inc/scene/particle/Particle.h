#ifndef INCLUDED_PARTICLE_H
#define INCLUDED_PARTICLE_H
#include "math/vector3.h"

namespace lux
{
namespace scene
{

struct Particle
{
	enum class EParameter
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

	// Often accessed parameters(many times per frame)
	math::vector3f position;
	math::vector3f velocity;
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

	float& Param(u32 off)
	{
		return params[off];
	}

	const float& Param(u32 off) const
	{
		return params[off];
	}
};

} // namespace video
} // namespace lux

#endif // !INCLUDED_SPARTICLE
