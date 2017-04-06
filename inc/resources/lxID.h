#ifndef INCLUDED_LX_ID_H
#define INCLUDED_LX_ID_H
#include "core\/LuxBase.h"

namespace lux
{
namespace core
{

struct lxID
{
	lxID() :
		value(0)
	{}

	u32 value;
};

}
}

#endif // #ifndef INCLUDED_LX_ID_H