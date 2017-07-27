#ifndef INCLUDED_IGUIRENDERER_H
#define INCLUDED_IGUIRENDERER_H
#include "core/ReferenceCounted.h"
#include "math/rect.h"
#include "video/Color.h"
#include "video/SpriteBank.h"
#include "core/lxString.h"
#include "gui/Font.h"

namespace lux
{
namespace gui
{

class GUIElement;

class GUIRenderer : public ReferenceCounted
{
public:
	enum EDefaultColor
	{

	};
public:
	virtual void AddRect(const math::RectI& rect, GUIElement* elem, video::SpriteBank::SSprite sprite, video::Color color = video::Color::White) = 0;
	virtual void AddRect(const math::RectI& rect, GUIElement* elem, video::Color color = video::Color::White) = 0;

	virtual video::Color GetDefaultColor(EDefaultColor color) = 0;

	virtual void Flush();
};

class GUIRendererImpl : public GUIRenderer
{

};

}
}

#endif // !INCLUDED_IGUIRENDERER_H
