#ifndef INCLUDED_CFONT_H
#define INCLUDED_CFONT_H
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
	math::dimension2du imageSize;

	core::HashMap<u32, CharInfo> charMap;
	float charHeight;

	float scale;
	float charDistance;
	float wordDistance;
	float lineDistance;
	float baseLine;

	FontCreationData() :
		image(nullptr),
		charHeight(0.0f),
		scale(1.0f),
		charDistance(0.0f),
		wordDistance(1.0f),
		lineDistance(1.0f),
		baseLine(0.0f)
	{
		
	}
};

class FontImpl : public Font
{
public:
	FontImpl();
	~FontImpl();

	void Init(const FontCreationData& data);

	const video::Material* GetMaterial() const;
	float GetBaseLine() const;
	void SetBaseLine(float base);
	void Draw(const String& Text, const math::vector2f& Position, EAlign Align, video::Color color, const math::rectf* clip);
	float GetTextWidth(const String& Text, size_t charCount = std::numeric_limits<size_t>::max());
	size_t GetCaretFromOffset(const String& Text, float XPosition);
	void GetTextCarets(const String& Text, core::Array<float>& carets, size_t charCount = std::numeric_limits<size_t>::max());

	float GetFontHeight() const;

	float GetCharDistance() const;
	float GetWordDistance() const;
	float GetLineDistance() const;
	float GetScaling() const;

	void SetLineDistance(float Space);
	void SetCharDistance(float width);
	void SetWordDistance(float Space);
	void SetScaling(float Scale);

	const FontDescription& GetDescription() const;

	core::Name GetReferableType() const;
	StrongRef<Referable> Clone() const;

private:
	const CharInfo& GetCharInfo(u32 c);

private:
	core::HashMap<u32, CharInfo> m_CharMap;
	float m_CharHeight;

	float m_CharDistance;
	float m_WordDistance;
	float m_LineDistance;
	float m_Scale;
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
