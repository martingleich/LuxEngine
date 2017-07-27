#ifndef INCLUDED_LIGHTDATA_H
#define INCLUDED_LIGHTDATA_H
#include "math/vector3.h"
#include "video/Color.h"

namespace lux
{
namespace video
{

//! The diffrent lighttypes
enum class ELightType
{
	//! Casts light from a single point into all directions
	Point = 0,

	//! Cast infinite long parallel lightrays
	/**
	Comparable to the sun.
	*/
	Directional,

	//! Cast light from a single point into a cone
	/**
	Comparable to a spotlight.
	*/
	Spot,
};

//! Contains all data needed to represent a light
class LightData
{
public:
	LightData() :
		type(ELightType::Point),
		color(1.0f, 1.0f, 1.0f),
		position(0.0f, 0.0f, 0.0f),
		range(100.0f),
		direction(0.0f, 0.0f, 1.0f),
		innerCone(math::DegToRad(10.0f)),
		outerCone(math::DegToRad(45.0f)),
		falloff(2.0f)
	{
	}

	//! The light type
	ELightType type;

	//! The color of the light
	/**
	Default: White
	*/
	Colorf color;

	//! The lightposition in world coordinates
	math::Vector3F position;

	//! The light range
	/**
	Points after this range are not illuminated.
	Only for point and spotlights.
	*/
	float range;

	//! The lightdirection in world coordinates
	math::Vector3F direction;

	//! The inner lightcone of a spotlight
	/**
	Must be between zero and outerCone
	*/
	float innerCone;

	//! The outer lightcone of a spotlight
	/**
	Must be between innerCone and 2 pi
	*/
	float outerCone;

	//! The falloff between inner and outer light cone
	/**
	Only for spotlights.
	=1: linear falloff
	<1: starts falling slow, becoming faster
	>1: start falling fast, becoming slower
	exact: intensity factor = ((cos a - cos (outerCone/2)) / (cos (innerCone/2) - cos(outerCone/2))) ^ falloff.
	a = Angle between vertex and light direction
	*/
	float falloff;
};

}
}

#endif