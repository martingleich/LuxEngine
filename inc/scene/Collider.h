#ifndef INCLUDED_SCENE_COLLIDER_H
#define INCLUDED_SCENE_COLLIDER_H
#include "core/ReferenceCounted.h"
#include "core/Result.h"
#include "math/aabbox3d.h"

namespace lux
{
namespace scene
{
class Query;
class QueryCallback;
class SceneNode;

class Collider : public ReferenceCounted
{
public:
	virtual EResult ExecuteQuery(SceneNode* owner, Query* query, QueryCallback* result) = 0;
	virtual const math::aabbox3df& GetBoundingBox() const = 0;
};

}
}

#endif // #ifndef INCLUDED_SCENE_COLLIDER_H