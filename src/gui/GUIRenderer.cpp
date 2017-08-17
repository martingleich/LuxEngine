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

void Renderer::Begin()
{
	m_Renderer->SetTransform(video::ETransform::World, math::Matrix4::IDENTITY);
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

void Renderer::DrawRectangle(const math::RectF& rect, const video::Color& color, const math::RectF* clip)
{
	video::Pass p;
	p.useVertexColor = true;
	p.backfaceCulling = false;
	p.zWriteEnabled = false;
	p.zBufferFunc = video::EComparisonFunc::Always;
	p.alphaSrcBlend = video::EBlendFactor::SrcAlpha;
	p.alphaDstBlend = video::EBlendFactor::OneMinusSrcAlpha;
	p.alphaOperator = video::EBlendOperator::Add;
	p.isTransparent = true;
	m_Renderer->SetPass(p);

	auto realRect = rect;
	if(clip)
		realRect.FitInto(*clip);
	video::Vertex2D quad[6] = {
		video::Vertex2D(realRect.left, realRect.bottom, color),
		video::Vertex2D(realRect.right, realRect.bottom, color),
		video::Vertex2D(realRect.left, realRect.top, color),
		video::Vertex2D(realRect.right, realRect.top, color),
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
