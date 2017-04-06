#include "FontImpl.h"
#include "video/VideoDriver.h"
#include "video/Texture.h"

namespace lux
{
namespace gui
{


// TODO: Make font a shader material
// Use color with shaderParam
// Implement clipping
// Add text borders
FontImpl::FontImpl() :
	m_Driver(nullptr),
	m_Image(nullptr),
	m_ImageWidth(0),
	m_ImageHeight(0)
{
}


FontImpl::~FontImpl()
{
	delete[] m_Image;
}

bool FontImpl::Init(video::VideoDriver* driver, const FontCreationData& data)
{
	m_Driver = driver;

	m_CharDistance = data.charDistance;
	m_WordDistance = data.wordDistance;
	m_LineDistance = data.lineDistance;
	m_Scale = data.scale;
	m_CharHeight = data.charHeight;
	m_BaseLine = data.baseLine;
	m_CharMap = data.charMap;

	const wchar_t errorChars[] = L"? ";

	for(const wchar_t* c = errorChars; *c; ++c) {
		auto it = m_CharMap.Find(*c);
		if(it != m_CharMap.End()) {
			m_ErrorChar = *it;
			break;
		}
	}

	delete[] m_Image;
	if(data.image) {
		m_Image = new FontPixel[data.imageWidth * data.imageHeight];
		m_ImageWidth = data.imageWidth;
		m_ImageHeight = data.imageHeight;

		memcpy(m_Image, data.image, m_ImageWidth*m_ImageHeight * sizeof(FontPixel));

		m_Texture = m_Driver->CreateTexture(math::dimension2du(m_ImageWidth, m_ImageHeight), video::ColorFormat::A8R8G8B8, 0, true, false);
		if(!m_Texture) {
			return false;
		}

		video::BaseTexture::SLockedRect rect;
		m_Texture->Lock(video::BaseTexture::ETLM_OVERWRITE, &rect);
		if(!rect.bits) {
			return false;
		}

		const FontPixel* srcRow = m_Image;
		u8* dstRow = (u8*)rect.bits;
		for(u32 y = 0; y < m_ImageHeight; ++y) {
			const FontPixel* srcPixel = srcRow;
			u8* dstPixel = dstRow;
			for(u32 x = 0; x < m_ImageWidth; ++x) {
				u8 value = srcPixel->intensity;
				srcPixel++;

				*dstPixel++ = value;        // Alpha
				*dstPixel++ = value;        // Red
				*dstPixel++ = value;        // Green
				*dstPixel++ = value;        // Blue
			}

			dstRow += m_ImageWidth * 4;
			srcRow += m_ImageWidth * sizeof(FontPixel);
		}
		m_Texture->Unlock();
	} else {
		m_Image = nullptr;
		m_Texture = nullptr;
		m_ImageWidth = 0;
		m_ImageHeight = 0;
	}

	if(!SetMaterial(data.material)) {
		return false;
	}

	return true;
}

bool FontImpl::SetMaterial(const video::Material& material)
{
	if(material.Layer(0).IsValid() == false)
		return false;

	m_Material = material;

	if(m_Texture)
		m_Material.Layer(0) = m_Texture;

	return true;
}

const video::Material& FontImpl::GetMaterial() const
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

void FontImpl::Draw(const string& text, const math::vector2f& position, EAlign align, video::Color color, const math::rectf* clip)
{
	// TODO: Implement clipping
	LUX_UNUSED(clip);

	m_Driver->Set2DMaterial(m_Material);

	const float italic = 0.0f * m_Scale;
	const float charHeight = m_CharHeight * m_Scale;
	const float charSpace = m_CharHeight * m_CharDistance * m_Scale;

	math::vector2f cursor;
	if(TestFlag(align, Font::EAlign::HCenter))
		cursor.x = position.x - 0.5f * (GetTextWidth(text, 0) + italic);
	else if(TestFlag(align, Font::EAlign::HRight))
		cursor.x = position.x - (GetTextWidth(text, 0) + italic);
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
		vertices[vertexCursor].texture.x = info.Left;
		vertices[vertexCursor].texture.y = info.Top;

		// Top-Right
		vertices[vertexCursor + 1].position.x = floorf(cursor.x + CharWidth + italic);
		vertices[vertexCursor + 1].position.y = floorf(cursor.y);
		vertices[vertexCursor + 1].color = color;
		vertices[vertexCursor + 1].texture.x = info.Right;
		vertices[vertexCursor + 1].texture.y = info.Top;

		// Lower-Right
		vertices[vertexCursor + 2].position.x = floorf(cursor.x + CharWidth);
		vertices[vertexCursor + 2].position.y = floorf(cursor.y + charHeight);
		vertices[vertexCursor + 2].color = color;
		vertices[vertexCursor + 2].texture.x = info.Right;
		vertices[vertexCursor + 2].texture.y = info.Bottom;

		vertices[vertexCursor + 3] = vertices[vertexCursor];
		vertices[vertexCursor + 4] = vertices[vertexCursor + 2];

		// Lower-Left
		vertices[vertexCursor + 5].position.x = floorf(cursor.x);
		vertices[vertexCursor + 5].position.y = floorf(cursor.y + charHeight);
		vertices[vertexCursor + 5].color = color;
		vertices[vertexCursor + 5].texture.x = info.Left;
		vertices[vertexCursor + 5].texture.y = info.Bottom;

		vertexCursor += 6;

		cursor.x += CharWidth + info.C * m_Scale;

		if(!isLast)
			cursor.x += charSpace;

		if(vertexCursor >= 600) {
			m_Driver->Draw2DPrimitiveList(video::EPT_TRIANGLES,
				vertexCursor / 3, vertices,
				vertexCursor, video::VertexFormat::STANDARD_2D,
				nullptr, video::EIndexFormat::Bit16);

			vertexCursor = 0;
		}
	}

	if(vertexCursor > 0) {
		m_Driver->Draw2DPrimitiveList(video::EPT_TRIANGLES,
			vertexCursor / 3, vertices,
			vertexCursor, video::VertexFormat::STANDARD_2D,
			nullptr, video::EIndexFormat::Bit16);
	}
}

float FontImpl::GetTextWidth(const string& text, size_t charCount)
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

size_t FontImpl::GetCaretFromOffset(const string& text, float XPosition)
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

void FontImpl::GetTextCarets(const string& text, core::array<float>& carets, size_t charCount)
{
	const float charSpace = m_CharHeight * m_CharDistance;

	if(text.IsEmpty())
		return;

	float width = 0.0f;
	size_t counter = 0;
	auto it = text.First();
	while(counter < charCount && it != text.Last()) {
		++counter;
		carets.Push_Back(width*m_Scale);
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

	carets.Push_Back(width);
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

core::Name FontImpl::GetReferableSubType() const
{
	return core::ResourceType::Font;
}

StrongRef<Referable> FontImpl::Clone() const
{
	return LUX_NEW(FontImpl)(*this);
}

} // namespace gui
} // namespace lux