#ifndef INCLUDED_GUIENVIRONMENT_H
#define INCLUDED_GUIENVIRONMENT_H
#include "io/Path.h"
#include "core/ReferenceCounted.h"

namespace lux
{
namespace gui
{
class Font;
class FontCreator;

//! Represent all parts of the GUI
class GUIEnvironment : public ReferenceCounted
{
public:
	//! Retrieve the font creator.
	virtual StrongRef<FontCreator> GetFontCreator() = 0;
};

} // namespace gui
} // namespace lux

#endif // INCLUDED_GUIENVIRONMENT_H
