#ifndef INCLUDED_LUX_GUI_RENDERER_H
#define INCLUDED_LUX_GUI_RENDERER_H
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

struct LineStyle
{
	float steps[2];
	bool invert;

	LineStyle() :
		invert(false)
	{
		steps[0] = steps[1] = 0;
	}
	static LineStyle Solid()
	{
		LineStyle out;
		out.steps[0] = 1;
		return out;
	}
	static LineStyle Dashed(float step = 5)
	{
		LineStyle out;
		out.steps[0] = step;
		out.steps[1] = step;
		return out;
	}
};

class Renderer : public ReferenceCounted
{
public:
	LUX_API Renderer(video::Renderer* r);
	LUX_API void Begin();
	LUX_API void DrawText(gui::Font* font, const FontRenderSettings& settings, core::Range<core::String::ConstIterator> text, const math::Vector2F& position, const math::RectF* clip);

	LUX_API void DrawRectangle(const math::RectF& rect, video::Color color, const math::RectF* clip = nullptr);
	LUX_API void DrawRectangle(const math::RectF& rect, video::Texture* texture, const math::RectF& tCoord=math::RectF(0,0,1,1), video::Color color=video::Color::White, const math::RectF* clip = nullptr);
	LUX_API void DrawTriangle(const math::Vector2F& a, const math::Vector2F& b, const math::Vector2F& c, video::Color color, const math::RectF* clip = nullptr);

	LUX_API void DrawLine(const math::Vector2F& start, const math::Vector2F& end, video::Color color, float thickness = 1.0f, const LineStyle& style = LineStyle::Solid());
	LUX_API void Flush();
	LUX_API video::Renderer* GetRenderer() const;

private:
	video::Renderer* m_Renderer;
	video::Pass m_TexturePass;
	video::Pass m_DiffusePass;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_LUX_GUI_RENDERER_H