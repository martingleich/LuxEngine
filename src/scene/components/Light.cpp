#include "scene/components/Light.h"
#include "scene/Node.h"
#include "video/Renderer.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::PointLight, "lux.comp.PointLight");
LX_REFERABLE_MEMBERS_SRC(lux::scene::DirectionalLight, "lux.comp.DirectionalLight");
LX_REFERABLE_MEMBERS_SRC(lux::scene::SpotLight, "lux.comp.SpotLight");
LX_REFERABLE_MEMBERS_SRC(lux::scene::GlobalAmbientLight, "lux.comp.GlobalAmbientLight");

namespace lux
{
namespace scene
{

ClassicalLight::ClassicalLight(video::ELightType type) :
	m_Power(1.0f),
	m_Color(1,1,1)
{
	m_Desc.type = type;
}

void ClassicalLight::SetRange(float range) { m_Desc.range = range >= 0 ? range : 0; }
float ClassicalLight::GetRange() const { return m_Desc.range; }
void ClassicalLight::SetPower(float power) { m_Power = power; }
float ClassicalLight::GetPower() const { return m_Power; }
void ClassicalLight::SetColor(const video::ColorF& color) { m_Color = color; }
const video::ColorF& ClassicalLight::GetColor() const { return m_Color; }
bool ClassicalLight::IsShadowCasting() const { return m_Desc.isShadowCasting; }
void ClassicalLight::SetShadowCasting(bool b) { m_Desc.isShadowCasting = b; }

static bool IsValidLightType(video::ELightType type)
{
	return type == video::ELightType::Point || type == video::ELightType::Spot || type == video::ELightType::Directional;
}

ClassicalLightDescription* ClassicalLight::GetLightDescription()
{
	auto node = GetParent();

	lxAssert(IsValidLightType(m_Desc.type));

	m_Desc.finalColor = m_Color * m_Power;
	if(m_Desc.type == video::ELightType::Spot ||
		m_Desc.type == video::ELightType::Directional) {
		m_Desc.direction = node->FromRelativeDir(math::Vector3F::UNIT_Z);
	}

	if(m_Desc.type == video::ELightType::Spot ||
		m_Desc.type == video::ELightType::Point) {
		m_Desc.position = node->GetAbsolutePosition();
	}

	return &m_Desc;
}


DirectionalLight::DirectionalLight() :
	ClassicalLight(video::ELightType::Directional)
{
}
PointLight::PointLight() :
	ClassicalLight(video::ELightType::Point)
{
}
SpotLight::SpotLight() :
	ClassicalLight(video::ELightType::Spot)
{
	m_Desc.innerCone = m_Desc.outerCone = math::AngleF::Degree(45.0f).Radian();
	m_Desc.falloff = 0.0f;
}

void SpotLight::SetInnerCone(math::AngleF angle)
{
	m_Desc.innerCone = angle.Radian();
}

math::AngleF SpotLight::GetInnerCone() const
{
	return math::AngleF::Radian(m_Desc.innerCone);
}

void SpotLight::SetOuterCone(math::AngleF angle)
{
	m_Desc.outerCone = angle.Radian();
}

math::AngleF SpotLight::GetOuterCone() const
{
	return math::AngleF::Radian(m_Desc.outerCone);
}

void SpotLight::SetFalloff(float falloff)
{
	m_Desc.falloff = falloff;
}

float SpotLight::GetFalloff() const
{
	return m_Desc.falloff;
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
	m_Desc.color = color;
}

video::ColorF GlobalAmbientLight::GetColor() const
{
	return m_Desc.color;
}

AmbientLightDescription* GlobalAmbientLight::GetLightDescription()
{
	return &m_Desc;
}

}
}