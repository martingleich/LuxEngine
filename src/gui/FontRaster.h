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

struct FontPixel
{
	u8 intensity;
};

struct FontCreationData
{
	FontDescription desc;

	FontPixel* image;
	math::Dimension2U imageSize;

	core::HashMap<u32, CharInfo> charMap;
	float charHeight;
	float baseLine;

	FontCreationData() :
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

	const video::Material* GetMaterial() const;

	const FontDescription& GetDescription() const;
	float GetBaseLine() const;
	float GetFontHeight() const;

	core::Name GetReferableType() const;
	StrongRef<Referable> Clone() const;

	const core::HashMap<u32, CharInfo>& GetCharMap() const;
	video::Image* GetImage() const;

private:
	const CharInfo& GetCharInfo(u32 c);

private:
	core::HashMap<u32, CharInfo> m_CharMap;
	float m_CharHeight;
	float m_BaseLine;

	StrongRef<video::Material> m_Material;
	StrongRef<video::Image> m_Image;
	StrongRef<video::Texture> m_Texture;

	CharInfo m_ErrorChar;

	FontDescription m_Desc;
};

} // namespace gui
} // namespace lux

#endif
