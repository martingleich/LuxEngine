#ifndef INCLUDED_SCENE_COLLIDER_H
#define INCLUDED_SCENE_COLLIDER_H
#include "core/Referable.h"
#include "core/Result.h"
#include "math/aabbox3d.h"
#include "math/triangle3d.h"

namespace lux
{
namespace scene
{
class Query;
class QueryCallback;
class SceneNode;

class Collider : public Referable
{
public:
	virtual EResult ExecuteQuery(SceneNode* owner, Query* query, QueryCallback* result) = 0;

	core::Name GetReferableType() const
	{
		return ReferableType::Collider;
	}
};

class TriangleCollider : public Collider
{
public:
	virtual const math::triangle3df& GetTriangle(u32 id) const = 0;
};

}
}

#endif // #ifndef INCLUDED_SCENE_COLLIDER_H