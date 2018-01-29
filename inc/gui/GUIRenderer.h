#ifndef INCLUDED_GUI_RENDERER_H
#define INCLUDED_GUI_RENDERER_H
#include "core/ReferenceCounted.h"
#include "math/Rect.h"
#include "math/Triangle3.h"
#include "video/Color.h"
#include "video/Texture.h"
#include "video/Renderer.h"
#include "video/Pass.h"
#include "gui/Font.h"

namespace lux
{
namespace gui
{

class Renderer : public ReferenceCounted
{
public:
	LUX_API Renderer(video::Renderer* r);
	LUX_API void Begin();
	LUX_API void DrawText(gui::Font* font, const FontRenderSettings& settings, core::Range<core::String::ConstIterator> text, const math::Vector2F& position, const math::RectF* clip);

	LUX_API void DrawRectangle(const math::RectF& rect, const video::Color& color, const math::RectF* clip = nullptr);
	LUX_API void DrawRectangle(const math::RectF& rect, video::Texture* texture, const math::RectF& tCoord=math::RectF(0,0,1,1), const video::Color& color=video::Color::White, const math::RectF* clip = nullptr);
	LUX_API void DrawTriangle(const math::Vector2F& a, const math::Vector2F& b, const math::Vector2F& c, const video::Color& color, const math::RectF* clip = nullptr);

	LUX_API void Flush();
	LUX_API video::Renderer* GetRenderer() const;

private:
	video::Renderer* m_Renderer;
	video::Pass m_TexturePass;
	video::Pass m_DiffusePass;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_RENDERER_H