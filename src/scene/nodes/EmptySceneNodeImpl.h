#ifndef INCLUDED_EMPTY_SCENE_NODE_H
#define INCLUDED_EMPTY_SCENE_NODE_H
#include "scene/SceneNode.h"

namespace lux
{
namespace scene
{

class EmptySceneNode : public SceneNode
{
public:
	EmptySceneNode() = default;
	EmptySceneNode(const EmptySceneNode& other) = default;

	void Render() {}
	core::Name GetReferableSubType() const
	{
		return SceneNodeType::Empty;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(EmptySceneNode)(*this);
	}
};

}
}

#endif // #ifndef INCLUDED_EMPTY_SCENE_NODE_H