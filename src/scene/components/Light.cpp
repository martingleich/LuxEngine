#include "scene/components/Light.h"
#include "scene/Node.h"
#include "video/Renderer.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS("lux.comp.Light", lux::scene::Light);

namespace lux
{
namespace scene
{

Light::Light() :
	m_IsShadowCasting(true)
{
}

Light::~Light()
{
}

void Light::SetLightData(const video::LightData& light)
{
	m_LightData = light;
}

const video::LightData& Light::GetLightData() const
{
	return m_LightData;
}

void Light::SetRange(float range)
{
	m_LightData.range = range;
}

float Light::GetRange() const
{
	return m_LightData.range;
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

void Light::SetColor(const video::Colorf& color)
{
	m_LightData.color = color;
}

const video::Colorf& Light::GetColor() const
{
	return m_LightData.color;
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

core::Name Light::GetReferableType() const
{
	return SceneComponentType::Light;
}

StrongRef<Referable> Light::Clone() const
{
	return LUX_NEW(Light)(*this);
}

}
}