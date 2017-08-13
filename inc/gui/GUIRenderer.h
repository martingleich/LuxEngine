#ifndef INCLUDED_GUI_RENDERER_H
#define INCLUDED_GUI_RENDERER_H
#include "core/ReferenceCounted.h"
#include "math/Rect.h"
#include "video/Color.h"
#include "video/Texture.h"
#include "video/Renderer.h"
#include "gui/Font.h"

namespace lux
{
namespace gui
{

class Renderer : public ReferenceCounted
{
public:
	LUX_API Renderer(video::Renderer* r);
	LUX_API void DrawText(gui::Font* font, const String& text, const math::Vector2F& position, gui::Font::EAlign align, video::Color color, const math::RectF* clip);
	LUX_API void DrawRectangle(const math::RectF& rect, const video::Color& color);
	LUX_API void Flush();

private:
	video::Renderer* m_Renderer;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_RENDERER_H