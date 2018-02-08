#include "gui/GUIRenderer.h"
#include "video/VertexTypes.h"
#include "video/VertexFormat.h"

namespace lux
{
namespace gui
{

Renderer::Renderer(video::Renderer* r)
{
	m_Renderer = r;
	m_TexturePass.useVertexColor = true;
	m_TexturePass.backfaceCulling = false;
	m_TexturePass.zWriteEnabled = false;
	m_TexturePass.zBufferFunc = video::EComparisonFunc::Always;
	m_TexturePass.alphaSrcBlend = video::EBlendFactor::SrcAlpha;
	m_TexturePass.alphaDstBlend = video::EBlendFactor::OneMinusSrcAlpha;
	m_TexturePass.alphaOperator = video::EBlendOperator::Add;
	m_TexturePass.requirements = video::EMaterialRequirement::Transparent;
	m_TexturePass.AddTexture();
	const video::TextureStageSettings MIX_SETTINGS(
		video::ETextureArgument::Diffuse,
		video::ETextureArgument::Texture,
		video::ETextureOperator::Modulate,
		video::ETextureArgument::Diffuse,
		video::ETextureArgument::Texture,
		video::ETextureOperator::Modulate);
	m_TexturePass.layerSettings.PushBack(MIX_SETTINGS);

	m_DiffusePass.useVertexColor = true;
	m_DiffusePass.backfaceCulling = false;
	m_DiffusePass.zWriteEnabled = false;
	m_DiffusePass.zBufferFunc = video::EComparisonFunc::Always;
	m_DiffusePass.alphaSrcBlend = video::EBlendFactor::SrcAlpha;
	m_DiffusePass.alphaDstBlend = video::EBlendFactor::OneMinusSrcAlpha;
	m_DiffusePass.alphaOperator = video::EBlendOperator::Add;
	m_DiffusePass.requirements = video::EMaterialRequirement::Transparent;
}

void Renderer::Begin()
{
	m_Renderer->SetTransform(video::ETransform::World, math::Matrix4::IDENTITY);
}

void Renderer::DrawText(gui::Font* font,
	const FontRenderSettings& settings,
	core::Range<core::String::ConstIterator> text,
	const math::Vector2F& position,
	const math::RectF* clip)
{
	if(font && text.begin() != text.end())
		font->Draw(settings, text, position, clip);
}

void Renderer::DrawRectangle(const math::RectF& rect, video::Color color, const math::RectF* clip)
{
	auto realRect = rect;
	if(clip)
		realRect.FitInto(*clip);
	if(realRect.IsEmpty())
		return;

	video::Vertex2D quad[4] = {
		video::Vertex2D(realRect.left, realRect.bottom, color),
		video::Vertex2D(realRect.right, realRect.bottom, color),
		video::Vertex2D(realRect.left, realRect.top, color),
		video::Vertex2D(realRect.right, realRect.top, color),
	};

	m_Renderer->SetPass(m_DiffusePass);
	m_Renderer->Draw2DPrimitiveList(
		video::EPrimitiveType::TriangleStrip,
		2, quad, 4, video::VertexFormat::STANDARD_2D);
}

void Renderer::DrawRectangle(const math::RectF& rect, video::Texture* texture, const math::RectF& tCoord, video::Color color, const math::RectF* clip)
{
	auto realRect = rect;
	if(clip)
		realRect.FitInto(*clip);
	if(realRect.IsEmpty())
		return;

	video::Vertex2D quad[4] = {
		video::Vertex2D(realRect.left, realRect.bottom, color, tCoord.left, tCoord.bottom),
		video::Vertex2D(realRect.right, realRect.bottom, color, tCoord.right, tCoord.bottom),
		video::Vertex2D(realRect.left, realRect.top, color, tCoord.left, tCoord.top),
		video::Vertex2D(realRect.right, realRect.top, color, tCoord.right, tCoord.top)
	};
	m_TexturePass.layers[0].texture = texture;
	m_Renderer->SetPass(m_TexturePass);
	m_Renderer->Draw2DPrimitiveList(
		video::EPrimitiveType::TriangleStrip,
		2, quad, 4, video::VertexFormat::STANDARD_2D);
}

void Renderer::DrawTriangle(const math::Vector2F& a, const math::Vector2F& b, const math::Vector2F& c, video::Color color, const math::RectF* clip)
{
	video::Vertex2D tri[3] = {
		video::Vertex2D(a.x, a.y, color),
		video::Vertex2D(b.x, b.y, color),
		video::Vertex2D(c.x, c.y, color),
	};

	m_Renderer->SetPass(m_DiffusePass);
	m_Renderer->Draw2DPrimitiveList(
		video::EPrimitiveType::Triangles,
		1, tri, 3, video::VertexFormat::STANDARD_2D);
}

namespace
{
struct LineBuffer
{
	video::Renderer* renderer;
	video::Vertex2D BUFFER[100];
	size_t cursor;
	video::Color color;

	LineBuffer(video::Renderer* r, video::Color c) :
		renderer(r),
		cursor(0),
		color(c)
	{}

	~LineBuffer()
	{
		Flush();
	}

	void DrawLine(math::Vector2F a, math::Vector2F b)
	{
		BUFFER[cursor].position = a;
		BUFFER[cursor].color = color;
		++cursor;
		BUFFER[cursor].position = b;
		BUFFER[cursor].color = color;
		++cursor;
		if(cursor == sizeof(BUFFER) / sizeof(*BUFFER))
			Flush();
	}

	void Flush()
	{
		if(!cursor)
			return;
		renderer->Draw2DPrimitiveList(
			video::EPrimitiveType::Lines,
			cursor / 2,
			&BUFFER, cursor,
			video::VertexFormat::STANDARD_2D);
		cursor = 0;
	}
};
}

void Renderer::DrawLine(const math::Vector2F& start, const math::Vector2F& end, video::Color color, float thickness, const LineStyle& style)
{
	lxAssert(sizeof(style.steps) / sizeof(*style.steps));

	if(thickness <= 0)
		return;
	if(style.steps[style.invert] == 0)
		return;
	m_Renderer->SetPass(m_DiffusePass);
	LineBuffer lineBuffer(m_Renderer, color);

	if(style.steps[1 - style.invert] == 0) {
		lineBuffer.DrawLine(start, end);
	} else {
		auto dir = end - start;
		float length = dir.GetLength();
		dir /= length;

		int stepId = 0;
		auto nextStep = [&]() -> float {
			float out = style.steps[stepId];
			stepId = (stepId + 1) % (sizeof(style.steps) / sizeof(*style.steps));
			return out;
		};

		float pos = 0;
		math::Vector2F base = start;
		bool state = !style.invert;
		while(pos < length) {
			float step = nextStep();
			if(pos + step > length)
				step = length - pos;
			if(state)
				lineBuffer.DrawLine(base, base + step*dir);
			pos += step;
			base += step*dir;
			state = !state;
		}
	}
}

void Renderer::Flush()
{
}

video::Renderer* Renderer::GetRenderer() const
{
	return m_Renderer;
}

} // namespace gui
} // namespace lux
