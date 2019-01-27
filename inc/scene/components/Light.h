#ifndef INCLUDED_LUX_SCENE_LIGHT_H
#define INCLUDED_LUX_SCENE_LIGHT_H
#include "scene/Component.h"

namespace lux
{
namespace scene
{

class LightDescription
{
public:
	virtual ~LightDescription() {}
};

class Light : public Component
{
public:
	virtual LightDescription* GetLightDescription() = 0;
	LUX_API void Register(bool doRegister) override;
};

/////////////////////////////////////////////////////////////////////

enum class ELightType
{
	Point,
	Directional,
	Spot,
};

class ClassicalLightDescription : public LightDescription
{
public:
	virtual ELightType GetType() = 0;
	virtual const video::ColorF& GetColor() = 0;
	virtual const math::Vector3F& GetPosition() = 0;
	virtual const math::Vector3F& GetDirection() = 0;
	virtual float GetInnerCone() = 0;
	virtual float GetOuterCone() = 0;
	virtual float GetFalloff() = 0;
	virtual float GetRange() = 0;
	virtual bool IsShadowCasting() = 0;
};

class ClassicalLight : public Light
{
protected:
	class LightDescriptionImpl : public ClassicalLightDescription
	{
	public:
		ELightType GetType() { return type; }
		const video::ColorF& GetColor() { return finalColor; }
		const math::Vector3F& GetPosition() { return position; }
		const math::Vector3F& GetDirection() { return direction; }
		float GetInnerCone() { return innerCone; }
		float GetOuterCone() { return outerCone; }
		float GetFalloff() { return falloff; }
		float GetRange() { return range; }
		bool IsShadowCasting() { return isShadowCasting; }

		scene::ELightType type;
		video::ColorF finalColor;

		math::Vector3F position;
		math::Vector3F direction;

		float innerCone;
		float outerCone;
		float falloff;

		float range = FLT_MAX;
		bool isShadowCasting = true;
	};

	LUX_API ClassicalLight(scene::ELightType type);
public:
	LUX_API void SetRange(float range);
	LUX_API float GetRange() const;
	LUX_API void SetPower(float power);
	LUX_API float GetPower() const;
	LUX_API void SetColor(const video::ColorF& color);
	LUX_API const video::ColorF& GetColor() const;
	LUX_API bool IsShadowCasting() const;
	LUX_API void SetShadowCasting(bool b);

	LUX_API ClassicalLightDescription* GetLightDescription();

protected:
	LightDescriptionImpl m_Desc;
	float m_Power;
	video::ColorF m_Color;
};

class DirectionalLight : public ClassicalLight
{
	LX_REFERABLE_MEMBERS_API(DirectionalLight, LUX_API);
public:
	LUX_API DirectionalLight();
};

class PointLight : public ClassicalLight
{
	LX_REFERABLE_MEMBERS_API(PointLight, LUX_API);
public:
	LUX_API PointLight();
};

class SpotLight : public ClassicalLight
{
	LX_REFERABLE_MEMBERS_API(SpotLight, LUX_API);
public:
	LUX_API SpotLight();

	//! Set the inner cone of a spotlight
	LUX_API void SetInnerCone(math::AngleF angle);

	//! Get the inner cone of a spotlight
	LUX_API math::AngleF GetInnerCone() const;

	//! Set the outer cone of a spotlight
	LUX_API void SetOuterCone(math::AngleF angle);

	//! Get the outer cone of a spotlight
	LUX_API math::AngleF GetOuterCone() const;

	//! Set the spotlight falloff
	LUX_API void SetFalloff(float falloff);

	//! Get the outer cone of a spotlight
	LUX_API float GetFalloff() const;
};

/////////////////////////////////////////////////////////////////////

class AmbientLightDescription : public LightDescription
{
public:
	virtual const video::ColorF& GetColor() = 0;
};

class GlobalAmbientLight : public Light
{
	LX_REFERABLE_MEMBERS_API(GlobalAmbientLight, LUX_API);
	class LightDescriptionImpl : public AmbientLightDescription
	{
	public:
		const video::ColorF& GetColor() { return color; }

		video::ColorF color;
	};

public:
	LUX_API GlobalAmbientLight();
	LUX_API ~GlobalAmbientLight();

	LUX_API void SetColor(const video::ColorF& color);
	LUX_API video::ColorF GetColor() const;

	LUX_API AmbientLightDescription* GetLightDescription();

protected:
	LightDescriptionImpl m_Desc;
};

} // namespace scene
} // namespace lux

#endif