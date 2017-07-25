#ifndef INCLUDED_LX_ID_H
#define INCLUDED_LX_ID_H
#include "core/LuxBase.h"

namespace lux
{
namespace core
{

struct lxID
{
	lxID() :
		value(0)
	{}

	bool IsValid() const
	{
		return (value != 0);
	}

	//! The internal value of the id
	/**
	There are no guarantees about the type or content of this variable, except its uniqueness.
	It should never be used directly.
	*/
	u32 value;
};

}
}

#endif // #ifndef INCLUDED_LX_ID_H