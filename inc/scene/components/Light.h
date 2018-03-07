#ifndef INCLUDED_SCENE_LIGHT_H
#define INCLUDED_SCENE_LIGHT_H
#include "scene/Component.h"
#include "video/LightData.h"

namespace lux
{
namespace scene
{

class Light : public Component
{
	LX_REFERABLE_MEMBERS_API(Light, LUX_API);
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
	LUX_API virtual void SetColor(const video::ColorF& color);

	//! Get the diffuse color of the light
	/**
	See \ref video::LightData::diffuse
	\return The diffuse color of the light
	*/
	LUX_API virtual const video::ColorF& GetColor() const;

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

protected:
	video::LightData m_LightData;
	float m_Power;
	float m_Range;
	bool m_IsShadowCasting;
};

/////////////////////////////////////////////////////////////////////

class GlobalAmbientLight : public Component
{
	LX_REFERABLE_MEMBERS_API(GlobalAmbientLight, LUX_API);

public:
	LUX_API GlobalAmbientLight();
	LUX_API ~GlobalAmbientLight();

	LUX_API virtual void SetColor(const video::ColorF& color);
	LUX_API virtual video::ColorF GetColor() const;

protected:
	video::ColorF m_Color;
};

} // namespace scene
} // namespace lux

#endif