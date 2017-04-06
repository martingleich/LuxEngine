#include "scene/SceneNodeComponent.h"
#include "scene/SceneNode.h"

namespace lux
{
namespace scene
{
namespace SceneNodeComponentType
{

const core::Name Rotation = "rotation";
const core::Name CameraFPS = "camera_fps";

}

void SceneNodeComponent::SetParent(SceneNode* new_parent)
{
	m_Parent = new_parent;
}

}
}