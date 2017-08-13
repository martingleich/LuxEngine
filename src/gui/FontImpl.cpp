#include "FontImpl.h"

#include "core/ReferableRegister.h"

#include "video/VideoDriver.h"
#include "video/Renderer.h"
#include "video/Texture.h"
#include "video/VertexTypes.h"
#include "video/MaterialLibrary.h"
#include "video/images/ImageSystem.h"

LUX_REGISTER_RESOURCE_CLASS("lux.resource.Font", lux::gui::FontImpl);

namespace lux
{
namespace gui
{

FontImpl::FontImpl(const core::ResourceOrigin& origin) :
	Font(origin)
{
}

FontImpl::~FontImpl()
{
}

void FontImpl::Init(const FontCreationData& data)
{
	m_Desc = data.desc;

	m_CharDistance = data.charDistance;
	m_WordDistance = data.wordDistance;
	m_LineDistance = data.lineDistance;
	m_Scale = data.scale;
	m_CharHeight = data.charHeight;
	m_BaseLine = data.baseLine;
	m_CharMap = data.charMap;

	String errorChars = "? ";

	for(auto it = errorChars.First(); it != errorChars.End(); ++it) {
		auto jt = m_CharMap.Find(*it);
		if(jt != m_CharMap.End()) {
			m_ErrorChar = *jt;
			break;
		}
	}

	if(data.image) {
		m_Image = video::ImageSystem::Instance()->CreateImage(data.imageSize, video::ColorFormat::X8, data.image, true, true);
		m_Texture = video::VideoDriver::Instance()->CreateTexture(data.imageSize, video::ColorFormat::A8R8G8B8, 1, false);

		video::TextureLock lock(m_Texture, video::BaseTexture::ELockMode::Overwrite);
		video::ImageLock imgLock(m_Image);

		const FontPixel* srcRow = (const FontPixel*)imgLock.data;
		u8* dstRow = lock.data;
		for(u32 y = 0; y < data.imageSize.height; ++y) {
			const FontPixel* srcPixel = srcRow;
			u8* dstPixel = dstRow;
			for(u32 x = 0; x < data.imageSize.width; ++x) {
				u8 value = srcPixel->intensity;
				srcPixel++;

				*dstPixel++ = value;        // Red
				*dstPixel++ = value;        // Green
				*dstPixel++ = value;        // Blue
				*dstPixel++ = value;        // Alpha
			}

			dstRow += lock.pitch;
			srcRow += imgLock.pitch;
		}
	} else {
		m_Image = nullptr;
		m_Texture = nullptr;
	}

	video::MaterialRenderer* renderer;
	if(!video::MaterialLibrary::Instance()->ExistsMaterialRenderer("font", &renderer)) {
		renderer = video::MaterialLibrary::Instance()->CloneMaterialRenderer("font", "transparent");
		video::Pass& pass = renderer->GetPass(0);
		pass.zWriteEnabled = false;
		pass.zBufferFunc = video::EComparisonFunc::Always;
		pass.backfaceCulling = false;
		pass.lighting = video::ELighting::Disabled;
		pass.fogEnabled = false;
		pass.useVertexColor = true;
		pass.isTransparent = true;
		pass.alphaSrcBlend = video::EBlendFactor::SrcAlpha;
		pass.alphaDstBlend = video::EBlendFactor::OneMinusSrcAlpha;
		pass.alphaOperator = video::EBlendOperator::Add;
		renderer->AddParam("texture", 0, video::EOptionId::Layer0);
		video::TextureStageSettings tss;
		tss.alphaArg1 = video::ETextureArgument::Texture;
		tss.alphaArg2 = video::ETextureArgument::Diffuse;
		tss.alphaOperator = video::ETextureOperator::Modulate;
		tss.colorArg1 = video::ETextureArgument::Texture;
		tss.colorArg2 = video::ETextureArgument::Diffuse;
		tss.colorOperator = video::ETextureOperator::Modulate;
		pass.layerSettings.PushBack(tss);
	}

	m_Material = renderer->CreateMaterial();

	if(m_Texture)
		m_Material->Layer(0) = m_Texture;
}

const video::Material* FontImpl::GetMaterial() const
{
	return m_Material;
}

float FontImpl::GetBaseLine() const
{
	return m_BaseLine;
}

void FontImpl::SetBaseLine(float base)
{
	m_BaseLine = base;
}

void FontImpl::Draw(const String& text, const math::Vector2F& position, EAlign align, video::Color color, const math::RectF* clip)
{
	LUX_UNUSED(clip);
	if(text.IsEmpty())
		return;

	auto renderer = video::VideoDriver::Instance()->GetRenderer();
	renderer->SetMaterial(m_Material);
	renderer->SetTransform(video::ETransform::World, math::Matrix4::IDENTITY);

	const float italic = 0.0f * m_Scale;
	const float charHeight = m_CharHeight * m_Scale;
	const float charSpace = m_CharHeight * m_CharDistance * m_Scale;

	math::Vector2F cursor;
	if(TestFlag(align, Font::EAlign::HCenter))
		cursor.x = position.x - 0.5f * (GetTextWidth(text) + italic);
	else if(TestFlag(align, Font::EAlign::HRight))
		cursor.x = position.x - (GetTextWidth(text) + italic);
	else
		cursor.x = position.x;

	if(TestFlag(align, Font::EAlign::VCenter))
		cursor.y = position.y - 0.5f * charHeight;
	else if(TestFlag(align, Font::EAlign::VBottom))
		cursor.y = position.y - charHeight;
	else
		cursor.y = position.y;

	video::Vertex2D vertices[600];
	u32 vertexCursor = 0;

	for(auto it = text.First(); it != text.End(); ++it) {
		const u32 character = *it;
		const bool isLast = (it == text.Last());
		const CharInfo& info = GetCharInfo(character);

		if(character == ' ') {
			cursor.x += (info.A + info.B + info.C) * m_Scale * m_WordDistance;
			if(!isLast)
				cursor.x += charSpace;
			continue;
		}

		const float CharWidth = info.B * m_Scale;

		cursor.x += info.A * m_Scale;

		// Top-Left
		vertices[vertexCursor].position.x = floorf(cursor.x + italic);
		vertices[vertexCursor].position.y = floorf(cursor.y);
		vertices[vertexCursor].color = color;
		vertices[vertexCursor].texture.x = info.left;
		vertices[vertexCursor].texture.y = info.top;

		// Top-Right
		vertices[vertexCursor + 1].position.x = floorf(cursor.x + CharWidth + italic);
		vertices[vertexCursor + 1].position.y = floorf(cursor.y);
		vertices[vertexCursor + 1].color = color;
		vertices[vertexCursor + 1].texture.x = info.right;
		vertices[vertexCursor + 1].texture.y = info.top;

		// Lower-Right
		vertices[vertexCursor + 2].position.x = floorf(cursor.x + CharWidth);
		vertices[vertexCursor + 2].position.y = floorf(cursor.y + charHeight);
		vertices[vertexCursor + 2].color = color;
		vertices[vertexCursor + 2].texture.x = info.right;
		vertices[vertexCursor + 2].texture.y = info.bottom;

		vertices[vertexCursor + 3] = vertices[vertexCursor];
		vertices[vertexCursor + 4] = vertices[vertexCursor + 2];

		// Lower-Left
		vertices[vertexCursor + 5].position.x = floorf(cursor.x);
		vertices[vertexCursor + 5].position.y = floorf(cursor.y + charHeight);
		vertices[vertexCursor + 5].color = color;
		vertices[vertexCursor + 5].texture.x = info.left;
		vertices[vertexCursor + 5].texture.y = info.bottom;

		vertexCursor += 6;

		cursor.x += CharWidth + info.C * m_Scale;

		if(!isLast)
			cursor.x += charSpace;

		if(vertexCursor >= 600) {
			renderer->DrawPrimitiveList(video::EPrimitiveType::Triangles,
				vertexCursor / 3, vertices,
				vertexCursor, video::VertexFormat::STANDARD_2D,
				false);

			vertexCursor = 0;
		}
	}

	if(vertexCursor > 0) {
		renderer->DrawPrimitiveList(video::EPrimitiveType::Triangles,
			vertexCursor / 3, vertices,
			vertexCursor, video::VertexFormat::STANDARD_2D,
			false);
	}
}

float FontImpl::GetTextWidth(const String& text, size_t charCount)
{
	const float charSpace = m_CharHeight * m_CharDistance * m_Scale;

	size_t counter = 0;
	float width = 0.0f;
	auto it = text.First();
	while(counter < charCount && it != text.End()) {
		++counter;
		const u32 c = *it;
		const CharInfo& info = GetCharInfo(c);
		if(c == ' ') {
			width += (info.A + info.B + info.C) * m_WordDistance * m_Scale;
		} else {
			width += (info.A + info.B + info.C) * m_Scale;
		}

		if(counter != charCount - 1 && it != text.Last())
			width += charSpace;

		++it;
	}

	return width;
}

size_t FontImpl::GetCaretFromOffset(const String& text, float XPosition)
{
	if(XPosition < 0.0f)
		return 0;

	const float charSpace = m_CharHeight * m_CharDistance * m_Scale;

	float caret = 0.0f;
	float lastCaret;
	size_t counter = 0;
	for(auto it = text.First(); it != text.End(); ++it) {
		lastCaret = caret;
		const u32 c = *it;
		const CharInfo& info = GetCharInfo(c);
		if(c == ' ')
			caret += (info.A + info.B + info.C)*m_Scale*m_WordDistance;
		else
			caret += (info.B + info.A + info.C)*m_Scale;

		if(XPosition >= lastCaret && XPosition <= caret) {
			const float d1 = XPosition - lastCaret;
			const float d2 = caret - XPosition;
			if(d1 < d2)
				return counter;
			else
				return counter + 1;
		}

		if(it != text.Last())
			caret += charSpace;
		++counter;
	}

	// Its the last caret
	return counter;
}

void FontImpl::GetTextCarets(const String& text, core::Array<float>& carets, size_t charCount)
{
	const float charSpace = m_CharHeight * m_CharDistance;

	if(text.IsEmpty())
		return;

	float width = 0.0f;
	size_t counter = 0;
	auto it = text.First();
	while(counter < charCount && it != text.Last()) {
		++counter;
		carets.PushBack(width*m_Scale);
		const u32 c = *it;
		const CharInfo& info = GetCharInfo(c);
		if(c == ' ')
			width += (info.A + info.B + info.C)*m_WordDistance * m_Scale;
		else
			width += (info.A + info.B + info.C)*m_Scale;

		if(counter != charCount - 1 && it != text.Last())
			width += charSpace*m_Scale;

		++counter;
	}

	carets.PushBack(width);
}

float FontImpl::GetCharDistance() const
{
	return m_CharDistance;
}

float FontImpl::GetFontHeight() const
{
	return m_CharHeight;
}

float FontImpl::GetWordDistance() const
{
	return m_WordDistance;
}

float FontImpl::GetLineDistance() const
{
	return m_LineDistance;
}

void FontImpl::SetLineDistance(float Space)
{
	m_LineDistance = Space;
}

float FontImpl::GetScaling() const
{
	return m_Scale;
}

void FontImpl::SetCharDistance(float width)
{
	m_CharDistance = width;
}

void FontImpl::SetWordDistance(float Space)
{
	m_WordDistance = Space;
}

void FontImpl::SetScaling(float Scale)
{
	m_Scale = Scale;
}

const CharInfo& FontImpl::GetCharInfo(u32 c)
{
	auto it = m_CharMap.Find(c);
	return (it != m_CharMap.End()) ? *it : m_ErrorChar;
}

core::Name FontImpl::GetReferableType() const
{
	return core::ResourceType::Font;
}

StrongRef<Referable> FontImpl::Clone() const
{
	return LUX_NEW(FontImpl)(*this);
}

const FontDescription& FontImpl::GetDescription() const
{
	return m_Desc;
}

} // namespace gui
} // namespace lux

