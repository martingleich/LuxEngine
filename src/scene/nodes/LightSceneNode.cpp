#include "scene/nodes/LightSceneNode.h"
#include "scene/SceneManager.h"
#include "video/Renderer.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::LightSceneNode)

namespace lux
{
namespace scene
{

void LightSceneNode::SetLightData(const video::LightData& light)
{
	m_LightData = light;
}

const video::LightData& LightSceneNode::GetLightData() const
{
	return m_LightData;
}

video::LightData& LightSceneNode::GetLightData()
{
	return m_LightData;
}

void LightSceneNode::SetRange(float range)
{
	m_LightData.range = range;
}

float LightSceneNode::GetRange() const
{
	return m_LightData.range;
}

void LightSceneNode::SetColor(const video::Colorf& color)
{
	m_LightData.color = color;
}

const video::Colorf& LightSceneNode::GetColor() const
{
	return m_LightData.color;
}

void LightSceneNode::SetLightType(video::LightData::EType type)
{
	m_LightData.type = type;
}

video::LightData::EType LightSceneNode::GetLightType() const
{
	return m_LightData.type;
}

const math::vector3f& LightSceneNode::GetDirection()
{
	m_LightData.direction = this->FromRelativeDir(math::vector3f::UNIT_Z);
	return m_LightData.direction;
}

void LightSceneNode::OnRegisterSceneNode()
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

void LightSceneNode::Render()
{
	video::Renderer* renderer = GetSceneManager()->GetRenderer();
	renderer->AddLight(m_LightData);
}

core::Name LightSceneNode::GetReferableSubType() const
{
	return SceneNodeType::Light;
}

StrongRef<Referable> LightSceneNode::Clone() const
{
	return LUX_NEW(LightSceneNode)(*this);
}

}
}