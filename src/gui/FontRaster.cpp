#include "FontRaster.h"

#include "core/ReferableRegister.h"

#include "video/VideoDriver.h"
#include "video/Renderer.h"
#include "video/Texture.h"
#include "video/VertexTypes.h"
#include "video/MaterialLibrary.h"
#include "video/images/ImageSystem.h"

LUX_REGISTER_RESOURCE_CLASS("lux.resource.Font", lux::gui::FontRaster);

namespace lux
{
namespace gui
{

FontRaster::FontRaster(const core::ResourceOrigin& origin) :
	Font(origin)
{
}

FontRaster::~FontRaster()
{
}

void FontRaster::Init(const FontCreationData& data)
{
	m_Desc = data.desc;

	m_CharHeight = data.charHeight;
	m_CharMap = data.charMap;
	m_BaseLine = data.baseLine;

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
		m_Texture->SetFiltering(video::BaseTexture::Filter::Point);

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
		pass.requirements = video::EMaterialRequirement::Transparent;
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

void FontRaster::Draw(
	const FontRenderSettings& settings,
	core::Range<String::ConstIterator> text,
	const math::Vector2F& position,
	const math::RectF* userClip)
{
	if(text.begin() == text.end())
		return;

	const float italic = m_CharHeight * settings.italic * settings.scale;
	const float charHeight = m_CharHeight * settings.scale;
	const float charSpace = m_CharHeight * settings.charDistance * settings.scale;

	auto renderer = video::VideoDriver::Instance()->GetRenderer();

	math::RectU clipRect = renderer->GetScissorRect();
	if(userClip) {
		clipRect.FitInto(math::RectU(
			(u32)math::Max(0.0f, userClip->left),
			(u32)math::Max(0.0f, userClip->top),
			(u32)math::Max(0.0f, std::ceil(userClip->right)),
			(u32)math::Max(0.0f, std::ceil(userClip->bottom))));
	}

	if(position.y + charHeight + 1 < (float)clipRect.top)
		return;
	if(position.y - 1 > (float)clipRect.bottom)
		return;

	video::ScissorRectToken tok;
	if(userClip)
		renderer->SetScissorRect(clipRect, &tok);

	renderer->SetMaterial(m_Material);
	renderer->SetTransform(video::ETransform::World, math::Matrix4::IDENTITY);

	math::Vector2F cursor = position;
	video::Vertex2D vertices[600];
	u32 vertexCursor = 0;
	for(u32 character : text) {
		const CharInfo& info = GetCharInfo(character);

		if(character == ' ') {
			cursor.x += (info.A + info.B + info.C) * settings.scale * settings.wordDistance;
			cursor.x += charSpace;
			continue;
		}

		const float charWidth = info.B * settings.scale;

		cursor.x += info.A * settings.scale;

		// Top-Left
		vertices[vertexCursor].position.x = std::floor(cursor.x + italic);
		vertices[vertexCursor].position.y = std::floor(cursor.y);
		vertices[vertexCursor].color = settings.color;
		vertices[vertexCursor].texture.x = info.left;
		vertices[vertexCursor].texture.y = info.top;

		// Top-Right
		vertices[vertexCursor + 1].position.x = std::floor(cursor.x + charWidth + italic);
		vertices[vertexCursor + 1].position.y = std::floor(cursor.y);
		vertices[vertexCursor + 1].color = settings.color;
		vertices[vertexCursor + 1].texture.x = info.right;
		vertices[vertexCursor + 1].texture.y = info.top;

		// Lower-Right
		vertices[vertexCursor + 2].position.x = std::floor(cursor.x + charWidth);
		vertices[vertexCursor + 2].position.y = std::floor(cursor.y + charHeight);
		vertices[vertexCursor + 2].color = settings.color;
		vertices[vertexCursor + 2].texture.x = info.right;
		vertices[vertexCursor + 2].texture.y = info.bottom;

		vertices[vertexCursor + 3] = vertices[vertexCursor];
		vertices[vertexCursor + 4] = vertices[vertexCursor + 2];

		// Lower-Left
		vertices[vertexCursor + 5].position.x = std::floor(cursor.x);
		vertices[vertexCursor + 5].position.y = std::floor(cursor.y + charHeight);
		vertices[vertexCursor + 5].color = settings.color;
		vertices[vertexCursor + 5].texture.x = info.left;
		vertices[vertexCursor + 5].texture.y = info.bottom;

		cursor.x += charWidth + info.C * settings.scale + charSpace;

		float maxX = math::Max(
			vertices[vertexCursor + 1].position.x,
			vertices[vertexCursor + 2].position.x) + 1;
		float minX = math::Min(
			vertices[vertexCursor].position.x,
			vertices[vertexCursor + 5].position.x) - 1;
		if(minX < 0 || (u32)minX > clipRect.right)
			break; // Abort rendering loop

		if(maxX < 0 || (u32)maxX < clipRect.left)
			continue; // Abort this character

		// Add this character to the chunk
		vertexCursor += 6;

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

float FontRaster::GetTextWidth(const FontRenderSettings& settings, core::Range<String::ConstIterator> text)
{
	if(text.begin() == text.end())
		return 0.0f;
	const float charSpace = m_CharHeight * settings.charDistance * settings.scale;

	float width = -charSpace;
	for(u32 c : text) {
		const CharInfo& info = GetCharInfo(c);
		if(c == ' ')
			width += (info.A + info.B + info.C) * settings.wordDistance * settings.scale;
		else
			width += (info.A + info.B + info.C) * settings.scale;

		width += charSpace;
	}

	return width;
}

size_t FontRaster::GetCaretFromOffset(const FontRenderSettings& settings, core::Range<String::ConstIterator> text, float XPosition)
{
	if(text.begin() == text.end())
		return 0;
	if(XPosition < 0.0f)
		return 0;

	const float charSpace = m_CharHeight * settings.charDistance * settings.scale;

	float caret = 0.0f;
	float lastCaret;
	size_t counter = 0;
	for(auto c : text) {
		lastCaret = caret;
		const CharInfo& info = GetCharInfo(c);
		if(c == ' ')
			caret += (info.A + info.B + info.C)*settings.scale*settings.wordDistance;
		else
			caret += (info.B + info.A + info.C)*settings.scale;

		if(XPosition >= lastCaret && XPosition <= caret) {
			const float d1 = XPosition - lastCaret;
			const float d2 = caret - XPosition;
			if(d1 < d2)
				return counter;
			else
				return counter + 1;
		}

		caret += charSpace;
		++counter;
	}

	// Its the last caret
	return counter;
}

void FontRaster::GetTextCarets(const FontRenderSettings& settings, core::Range<String::ConstIterator> text, core::Array<float>& carets)
{
	const float charSpace = m_CharHeight * settings.charDistance;

	if(text.begin() == text.end())
		return;

	float width = 0.0f;
	for(u32 c : text) {
		carets.PushBack(width*settings.scale);
		const CharInfo& info = GetCharInfo(c);
		if(c == ' ')
			width += (info.A + info.B + info.C)*settings.wordDistance * settings.scale;
		else
			width += (info.A + info.B + info.C)*settings.scale;

		width += charSpace*settings.scale;
	}

	carets.PushBack(width);
}

const CharInfo& FontRaster::GetCharInfo(u32 c)
{
	auto it = m_CharMap.Find(c);
	return (it != m_CharMap.End()) ? *it : m_ErrorChar;
}

core::Name FontRaster::GetReferableType() const
{
	return core::ResourceType::Font;
}

StrongRef<Referable> FontRaster::Clone() const
{
	return LUX_NEW(FontRaster)(*this);
}

const FontDescription& FontRaster::GetDescription() const
{
	return m_Desc;
}

const core::HashMap<u32, CharInfo>& FontRaster::GetCharMap() const
{
	return m_CharMap;
}
video::Image* FontRaster::GetImage() const
{
	return m_Image;
}

const video::Material* FontRaster::GetMaterial() const
{
	return m_Material;
}

float FontRaster::GetBaseLine() const
{
	return m_BaseLine;
}

float FontRaster::GetFontHeight() const
{
	return m_CharHeight;
}

} // namespace gui
} // namespace lux

