#ifndef INCLUDED_FONT_RASTER_H
#define INCLUDED_FONT_RASTER_H
#include "gui/Font.h"
#include "video/Material.h"
#include "core/lxHashMap.h"
#include "video/images/Image.h"

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
	math::Dimension2U imageSize;

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

	void Draw(const FontRenderSettings& settings, core::Range<core::String::ConstIterator> text, const math::Vector2F& Position, const math::RectF* clip);
	float GetTextWidth(const FontRenderSettings& settings, core::Range<core::String::ConstIterator> text);
	size_t GetCaretFromOffset(const FontRenderSettings& settings, core::Range<core::String::ConstIterator> text, float XPosition);
	void GetTextCarets(const FontRenderSettings& settings, core::Range<core::String::ConstIterator> text, core::Array<float>& carets);
	const core::HashMap<u32, CharInfo>& GetCharMap() const;
	const CharInfo& GetCharInfo(u32 c);

	const video::Material* GetMaterial() const;

	void SetBaseFontSettings(const FontRenderSettings& settings);
	const FontRenderSettings& GetBaseFontSettings();

	const FontDescription& GetDescription() const;
	float GetBaseLine() const;
	float GetFontHeight() const;

	void GetRawData(const void*& ptr, u32& width, u32& height, u32& channels);

	core::Name GetReferableType() const;
	StrongRef<Referable> Clone() const;

private:
	FontRenderSettings GetFinalFontSettings(const FontRenderSettings& settings);

	video::MaterialRenderer* EnsureMaterialRenderer();
	void LoadImageData(const u8* imageData,
		math::Dimension2U imageSize, u32 channelCount);

private:
	FontDescription m_Desc;
	FontRenderSettings m_BaseSettings;
	core::HashMap<u32, CharInfo> m_CharMap;
	float m_CharHeight;
	float m_BaseLine;

	// Backup image
	core::Array<u8> m_Image;
	math::Dimension2U m_ImageSize;
	u32 m_ChannelCount;

	StrongRef<video::Texture> m_Texture;
	StrongRef<video::Material> m_Material;

	// Character printed on error
	CharInfo m_ErrorChar;
};

} // namespace gui
} // namespace lux

#endif
