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
	void Render();
	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;
};

}
}

#endif // #ifndef INCLUDED_EMPTY_SCENE_NODE_H