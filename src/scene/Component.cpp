#include "scene/Component.h"
#include "scene/Node.h"
#include "scene/Scene.h"

namespace lux
{
namespace scene
{
namespace SceneComponentType
{
const core::Name Rotation("lux.comp.Rotation");
const core::Name FirstPersonCameraControl("lux.comp.FirstPersonCameraControl");
const core::Name TurntableCameraControl("lux.comp.TurntableCameraControl");
const core::Name LinearMove("lux.comp.LinearMove");

const core::Name Camera("lux.comp.Camera");
const core::Name Mesh("lux.comp.Mesh");
const core::Name SpotLight("lux.comp.SpotLight");
const core::Name PointLight("lux.comp.PointLight");
const core::Name DirLight("lux.comp.DirectionalLight");
const core::Name LinearFog("lux.comp.LinearFog");
const core::Name ExponentialFog("lux.comp.ExpFog");
const core::Name SkyBox("lux.comp.SkyBox");
}

void Component::SetAnimated(bool animated)
{
	if(animated == m_IsAnimated)
		return;

	if(auto s = GetScene())
		s->RegisterForTick(this, animated);
	m_IsAnimated = animated;
}

Scene* Component::GetScene()
{
	return m_Node ? m_Node->GetScene() : nullptr;
}

void Component::Register(bool doRegister)
{
	if(auto scene = GetScene()) {
		if(m_IsAnimated)
			scene->RegisterForTick(this, doRegister);
	}
}
}
}
