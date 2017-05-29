#include "scene/nodes/EmptySceneNode.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::EmptySceneNode)

namespace lux
{
namespace scene
{
void EmptySceneNode::Render()
{
}

core::Name EmptySceneNode::GetReferableSubType() const
{
	return SceneNodeType::Empty;
}

StrongRef<Referable> EmptySceneNode::Clone() const
{
	return LUX_NEW(EmptySceneNode)(*this);
}

} // namespace scene
} // namespace lux
