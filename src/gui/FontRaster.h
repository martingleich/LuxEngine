#ifndef INCLUDED_LUX_FONT_RASTER_H
#define INCLUDED_LUX_FONT_RASTER_H
#include "gui/Font.h"
#include "core/lxHashMap.h"
#include "video/Material.h"
#include "video/images/Image.h"
#include "video/Texture.h"

namespace lux
{
namespace video
{
class VideoDriver;
}
namespace gui
{

struct CharInfo
{
	float left;
	float top;
	float right;
	float bottom;

	float A;
	float B;
	float C;
};

struct FontCreationData
{
	FontDescription desc;

	u32 channelCount;
	u8* image;
	math::Dimension2I imageSize;

	core::HashMap<u32, CharInfo> charMap;
	float charHeight;
	float baseLine;

	FontRenderSettings baseSettings;

	FontCreationData() :
		channelCount(1),
		image(nullptr),
		charHeight(0.0f),
		baseLine(0.0f)
	{
	}
};

class FontRaster : public Font
{
public:
	FontRaster(const core::ResourceOrigin& origin);
	~FontRaster();

	void Init(const FontCreationData& data);

	void Draw(const FontRenderSettings& settings, const core::StringView& text, const math::Vector2F& Position, const math::RectF* clip);

	float GetTextWidth(const FontRenderSettings& settings, const core::StringView& text);
	int GetCaretFromOffset(const FontRenderSettings& settings, const core::StringView& text, float XPosition);
	void GetTextCarets(const FontRenderSettings& settings, const core::StringView& text, core::Array<FontCaret>& carets);
	const core::HashMap<u32, CharInfo>& GetCharMap() const;
	const CharInfo& GetCharInfo(u32 c);

	void SetBaseFontSettings(const FontRenderSettings& settings);
	const FontRenderSettings& GetBaseFontSettings();

	const FontDescription& GetDescription() const;
	float GetBaseLine() const;
	float GetFontHeight() const;

	void GetRawData(const void*& ptr, int& width, int& height, int& channels);

	core::Name GetReferableType() const;
	StrongRef<Referable> Clone() const;

private:
	FontRenderSettings GetFinalFontSettings(const FontRenderSettings& settings);

	void InitPass();
	void LoadImageData(const u8* imageData,
		math::Dimension2I imageSize, int channelCount);

	template <typename FuncT>
	void IterateCarets(
		const FontRenderSettings& _settings,
		const core::StringView& text,
		FuncT callback)
	{
		auto settings = GetFinalFontSettings(_settings);
		const float charSpace = settings.charDistance;
		if(text.IsEmpty())
			return;

		float width = 0;
		auto codepoints = text.CodePoints();
		for(auto it = codepoints.First(); it != codepoints.End(); ++it) {
			auto c = *it;
			auto offset = int(it.Pointer() - text.Data());
			if(callback(width * settings.scale, offset) == false)
				return;
			const CharInfo& info = GetCharInfo(c);
			if(c == ' ')
				width += (info.A + info.B + info.C)*settings.wordDistance * settings.scale;
			else
				width += (info.A + info.B + info.C)*settings.scale;

			width += charSpace*settings.scale;
		}

		callback(width, text.Size());
	}

private:
	FontDescription m_Desc;
	FontRenderSettings m_BaseSettings;
	core::HashMap<u32, CharInfo> m_CharMap;
	float m_CharHeight;
	float m_BaseLine;

	// Backup image
	core::Array<u8> m_Image;
	math::Dimension2I m_ImageSize;
	u32 m_ChannelCount;

	StrongRef<video::Texture> m_Texture;
	video::Pass m_Pass;

	// Character printed on error
	CharInfo m_ErrorChar;
};

} // namespace gui
} // namespace lux

#endif
