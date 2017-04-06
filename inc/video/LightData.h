#ifndef INCLUDED_LIGHTDATA_H
#define INCLUDED_LIGHTDATA_H
#include "math/vector3.h"
#include "video/Color.h"

namespace lux
{
namespace video
{

//! Contains all data needed to represent a light
class LightData
{
public:
	//! The diffrent lighttypes
	enum class EType
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

	LightData() : type(EType::Point), ambient(0.0f, 0.0f, 0.0f), diffuse(1.0f, 1.0f, 1.0f),
		outerCone(45.0f), innerCone(10.0f), falloff(2.0f),
		position(0.0f, 0.0f, 0.0f), direction(0.0f, 0.0f, 1.0f),
		range(100.0f), att0(1.0f), att1(0.0f), att2(0.0f)
	{
	}

	//! The light type
	EType type;

	//! The ambient color of the light
	/**
	Default: Black
	*/
	Colorf ambient;

	//! The diffuse color of the light
	/**
	Default: White
	*/
	Colorf diffuse;

	//! The attentuation the light 
	/**
	relative light illumination = 1 / (Att0 + d*Att1 + d²*Att2)
	d is the distance to the light
	Normal values are Att0 = Att2 = 0.0f und Att1 = 1 / LightRange
	*/
	float att0;
	float att1;
	float att2;

	//! The lightposition in world coordinates
	math::vector3f position;

	//! The light range
	/**
	Points after this range are not illuminated.
	Only for point and spotlights.
	*/
	float range;

	//! The lightdirection in world coordinates
	math::vector3f direction;


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

}    // namespace video
}    // namespace lux

#endif