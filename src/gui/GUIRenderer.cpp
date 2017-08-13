#include "gui/GUIRenderer.h"
#include "video/Pass.h"
#include "video/VertexTypes.h"
#include "video/VertexFormats.h"

namespace lux
{
namespace gui
{

Renderer::Renderer(video::Renderer* r)
{
	m_Renderer = r;
}

void Renderer::DrawText(gui::Font* font, const String& text, const math::Vector2F& position, gui::Font::EAlign align, video::Color color, const math::RectF* clip)
{
	if(font && !text.IsEmpty()) {
		font->Draw(
			text,
			position,
			align, color,
			clip);
	}
}

void Renderer::DrawRectangle(const math::RectF& rect, const video::Color& color)
{
	video::Pass p;
	p.useVertexColor = true;
	p.backfaceCulling = false;
	p.zBufferFunc = video::EComparisonFunc::Always;
	m_Renderer->SetPass(p);

	video::Vertex2D quad[6] = {
		video::Vertex2D(rect.left, rect.bottom, color),
		video::Vertex2D(rect.right, rect.bottom, color),
		video::Vertex2D(rect.left, rect.top, color),
		video::Vertex2D(rect.right, rect.top, color),
	};
	m_Renderer->DrawPrimitiveList(
		video::EPrimitiveType::TriangleStrip,
		2, quad, 4, video::VertexFormat::STANDARD_2D, false);
}

void Renderer::Flush()
{
}

} // namespace gui
} // namespace lux
