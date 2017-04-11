#include "LightSceneNodeImpl.h"
#include "scene/SceneManager.h"
#include "video/VideoDriver.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::LightSceneNodeImpl)

namespace lux
{
namespace scene
{

LightSceneNodeImpl::LightSceneNodeImpl()
{
}

void LightSceneNodeImpl::SetLightData(const video::LightData& light)
{
	m_LightData = light;
}

const video::LightData& LightSceneNodeImpl::GetLightData() const
{
	return m_LightData;
}

video::LightData& LightSceneNodeImpl::GetLightData()
{
	return m_LightData;
}

void LightSceneNodeImpl::SetRange(float range)
{
	m_LightData.range = range;
	m_LightData.att0 = 0.0f;
	m_LightData.att1 = 1.0f / range;
	m_LightData.att2 = 0.0f;
}

float LightSceneNodeImpl::GetRange() const
{
	return m_LightData.range;
}

void LightSceneNodeImpl::SetColor(const video::Colorf& color)
{
	m_LightData.diffuse = color;
}

const video::Colorf& LightSceneNodeImpl::GetColor() const
{
	return m_LightData.diffuse;
}

void LightSceneNodeImpl::SetLightType(video::LightData::EType type)
{
	m_LightData.type = type;
}

video::LightData::EType LightSceneNodeImpl::GetLightType() const
{
	return m_LightData.type;
}

const math::vector3f& LightSceneNodeImpl::GetDirection()
{
	m_LightData.direction = this->FromRelativeDir(math::vector3f::UNIT_Z);
	return m_LightData.direction;
}

void LightSceneNodeImpl::OnRegisterSceneNode()
{
	if(m_LightData.type == video::LightData::EType::Spot ||
		m_LightData.type == video::LightData::EType::Directional) {
		m_LightData.direction = this->FromRelativeDir(math::vector3f::UNIT_Z);
	}

	if(m_LightData.type == video::LightData::EType::Spot ||
		m_LightData.type == video::LightData::EType::Point) {
		m_LightData.position = this->GetAbsolutePosition();
	}

	GetSceneManager()->RegisterNodeForRendering(this, ESNRP_LIGHT);

	SceneNode::OnRegisterSceneNode();
}

void LightSceneNodeImpl::Render()
{
	video::VideoDriver* driver = GetSceneManager()->GetDriver();

	size_t index = driver->AddLight(m_LightData);
	driver->EnableLight(index, true);
}

core::Name LightSceneNodeImpl::GetReferableSubType() const
{
	return SceneNodeType::Light;
}

StrongRef<Referable> LightSceneNodeImpl::Clone() const
{
	return LUX_NEW(LightSceneNodeImpl)(*this);
}

}
}