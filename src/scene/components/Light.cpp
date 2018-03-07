#include "scene/components/Light.h"
#include "scene/Node.h"
#include "video/Renderer.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::Light, "lux.comp.Light");
LX_REFERABLE_MEMBERS_SRC(lux::scene::GlobalAmbientLight, "lux.comp.GlobalAmbientLight");

namespace lux
{
namespace scene
{

Light::Light() :
	m_Power(1.0f),
	m_Range(FLT_MAX),
	m_IsShadowCasting(true)
{
}

Light::~Light()
{
}

video::LightData Light::GetLightData() const
{
	auto node = GetParent();

	auto data = m_LightData;
	data.color *= m_Power;
	if(data.type == video::ELightType::Spot ||
		data.type == video::ELightType::Directional) {
		data.direction = node->FromRelativeDir(math::Vector3F::UNIT_Z);
	}

	if(data.type == video::ELightType::Spot ||
		data.type == video::ELightType::Point) {
		data.position = node->GetAbsolutePosition();
	}

	return data;;
}

void Light::SetRange(float range)
{
	m_Range = range;
}

float Light::GetRange() const
{
	return m_Range;
}

void Light::SetInnerCone(math::AngleF angle)
{
	m_LightData.innerCone = angle.Radian();
}

math::AngleF Light::GetInnerCone() const
{
	return math::AngleF::Radian(m_LightData.innerCone);
}

void Light::SetOuterCone(math::AngleF angle)
{
	m_LightData.outerCone = angle.Radian();
}

math::AngleF Light::GetOuterCone() const
{
	return math::AngleF::Radian(m_LightData.outerCone);
}

void Light::SetSpotFalloff(float falloff)
{
	m_LightData.falloff = falloff;
}

float Light::GetSpotFalloff() const
{
	return m_LightData.falloff;
}

void Light::SetColor(const video::ColorF& color)
{
	m_LightData.color = color;
}

const video::ColorF& Light::GetColor() const
{
	return m_LightData.color;
}

void Light::SetPower(float power)
{
	m_Power = power;
}

float Light::GetPower() const
{
	return m_Power;
}

void Light::SetLightType(video::ELightType type)
{
	m_LightData.type = type;
}

video::ELightType Light::GetLightType() const
{
	return m_LightData.type;
}

bool Light::IsShadowCasting() const
{
	return m_IsShadowCasting;
}

void Light::SetShadowCasting(bool b)
{
	m_IsShadowCasting = b;
}

/////////////////////////////////////////////////////////////////////

GlobalAmbientLight::GlobalAmbientLight()
{
}

GlobalAmbientLight::~GlobalAmbientLight()
{
}

void GlobalAmbientLight::SetColor(const video::ColorF& color)
{
	m_Color = color;
}

video::ColorF GlobalAmbientLight::GetColor() const
{
	return m_Color;
}

}
}