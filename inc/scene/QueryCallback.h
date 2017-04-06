#ifndef INCLUDED_QUERY_CALLBACK_H
#define INCLUDED_QUERY_CALLBACK_H
#include "core/ReferenceCounted.h"

namespace lux
{
namespace scene
{
class SceneNode;

class QueryCallback : public ReferenceCounted
{
public:
	virtual bool OnObject(SceneNode* node)
	{
		LUX_UNUSED(node);
		return true;
	}
};

}
}

#endif // #ifndef INCLUDED_QUERY_CALLBACK_H
