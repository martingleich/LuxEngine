#ifndef INCLUDED_GUIENVIRONMENT_IMPL_H
#define INCLUDED_GUIENVIRONMENT_IMPL_H
#include "gui/GUIEnvironment.h"

namespace lux
{
namespace gui
{
class Font;

class GUIEnvironmentImpl : public GUIEnvironment
{
public:
	GUIEnvironmentImpl();
	~GUIEnvironmentImpl();

	StrongRef<FontCreator> GetFontCreator();

private:
	StrongRef<FontCreator> m_FontCreator;
};

} 
} 

#endif