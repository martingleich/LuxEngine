#ifndef INCLUDED_SCENE_LIGHT_H
#define INCLUDED_SCENE_LIGHT_H
#include "scene/Component.h"
#include "video/LightData.h"

namespace lux
{
namespace video
{
class Renderer;
}
namespace scene
{

//! Represent a light in the scenegraph
class Light : public Component
{
public:
	LUX_API Light();
	LUX_API virtual ~Light();

	//! Get the current light parameters
	/**
	\return The current light data
	*/
	LUX_API virtual video::LightData GetLightData() const;

	//! Set the range of the light
	/**
	Point farther away than don't have to be illuminated
	\param range The range of the light
	*/
	LUX_API virtual void SetRange(float range);

	//! Get the range of the light
	/**
	Point farther away than don't have to be illuminated
	\return The range of the light
	*/
	LUX_API virtual float GetRange() const;

	//! Set the power of the light
	LUX_API virtual void SetPower(float power);
	
	//! Get the power of the light
	LUX_API virtual float GetPower() const;

	//! Set the inner cone of a spotlight
	LUX_API virtual void SetInnerCone(math::AngleF angle);

	//! Get the inner cone of a spotlight
	LUX_API virtual math::AngleF GetInnerCone() const;

	//! Set the outer cone of a spotlight
	LUX_API virtual void SetOuterCone(math::AngleF angle);

	//! Get the outer cone of a spotlight
	LUX_API virtual math::AngleF GetOuterCone() const;

	//! Set the spotlight falloff
	LUX_API virtual void SetSpotFalloff(float falloff);

	//! Get the outer cone of a spotlight
	LUX_API virtual float GetSpotFalloff() const;

	//! Set the diffuse color of the light
	/**
	See \ref video::LightData::diffuse
	\param color The new diffuse color of the light
	*/
	LUX_API virtual void SetColor(const video::Colorf& color);

	//! Get the diffuse color of the light
	/**
	See \ref video::LightData::diffuse
	\return The diffuse color of the light
	*/
	LUX_API virtual const video::Colorf& GetColor() const;

	//! Set the type of the light
	/**
	See \ref video::LightData::type
	\param type The type of the light
	*/
	LUX_API virtual void SetLightType(video::ELightType type);

	//! Get the type of the light
	/**
	See \ref video::LightData::type
	\return The type of the light
	*/
	LUX_API virtual video::ELightType GetLightType() const;

	LUX_API virtual bool IsShadowCasting() const;
	LUX_API virtual void SetShadowCasting(bool b);

	LUX_API virtual core::Name GetReferableType() const;
	LUX_API virtual StrongRef<Referable> Clone() const;

protected:
	video::LightData m_LightData;
	float m_Power;
	float m_Range;
	bool m_IsShadowCasting;
};

} // namespace scene
} // namespace lux

#endif