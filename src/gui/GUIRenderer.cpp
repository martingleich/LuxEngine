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

void Renderer::DrawText(gui::Font* font, const String& text, const math::Vector2F& position, gui::EAlign align, video::Color color, const math::RectF* clip)
{
	if(font && !text.IsEmpty()) {
		font->Draw(
			text,
			position,
			align, color,
			clip);
	}
}
void Renderer::DrawText(gui::Font* font, const String& text, const math::RectF& rect, gui::EAlign align, video::Color color, const math::RectF* clip)
{
	if(!font)
		return;

	float lineHeight = font->GetLineDistance()*font->GetScaling()*font->GetFontHeight();
	math::Vector2F cursor;
	if(TestFlag(align, EAlign::VTop))
		cursor.y = rect.top;
	else if(TestFlag(align, EAlign::VCenter))
		cursor.y = (rect.top + rect.bottom) / 2 - lineHeight / 2;
	else if(TestFlag(align, EAlign::VBottom))
		cursor.y = rect.bottom - lineHeight;

	if(TestFlag(align, EAlign::HLeft))
		cursor.x = rect.left;
	else if(TestFlag(align, EAlign::HCenter))
		cursor.x = (rect.left + rect.right) / 2;
	else if(TestFlag(align, EAlign::HRight))
		cursor.x = rect.right;
	EAlign lineAlign = EAlign::VTop | (align & ~(EAlign::VCenter | EAlign::VBottom | EAlign::VTop));
	DrawText(font, text, cursor, lineAlign, color, clip);
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
	p.requirements = video::EMaterialRequirement::Transparent;
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

void Renderer::DrawTriangle(const math::Vector2F& a, const math::Vector2F& b, const math::Vector2F& c, const video::Color& color, const math::RectF* clip)
{
	video::Pass p;
	p.useVertexColor = true;
	p.backfaceCulling = false;
	p.zWriteEnabled = false;
	p.zBufferFunc = video::EComparisonFunc::Always;
	p.alphaSrcBlend = video::EBlendFactor::SrcAlpha;
	p.alphaDstBlend = video::EBlendFactor::OneMinusSrcAlpha;
	p.alphaOperator = video::EBlendOperator::Add;
	p.requirements = video::EMaterialRequirement::Transparent;
	m_Renderer->SetPass(p);

	video::Vertex2D quad[6] = {
		video::Vertex2D(a.x, a.y, color),
		video::Vertex2D(b.x, b.y, color),
		video::Vertex2D(c.x, c.y, color),
	};
	m_Renderer->DrawPrimitiveList(
		video::EPrimitiveType::Triangles,
		1, quad, 3, video::VertexFormat::STANDARD_2D, false);
}

void Renderer::Flush()
{
}

} // namespace gui
} // namespace lux
