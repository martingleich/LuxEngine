#ifndef INCLUDED_CFONT_H
#define INCLUDED_CFONT_H
#include "gui/Font.h"
#include "video/Material.h"
#include "core/lxHashMap.h"
#include "video/images/Image.h"

namespace lux
{
namespace gui
{

struct CharInfo
{
	float Left;
	float Top;
	float Right;
	float Bottom;

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
	video::Material material;
	FontPixel* image;
	u32 imageWidth;
	u32 imageHeight;

	core::HashMap<u32, CharInfo> charMap;
	float charHeight;

	float charDistance;
	float wordDistance;
	float lineDistance;
	float scale;
	float baseLine;

	FontCreationData() :
		image(nullptr),
		imageWidth(0),
		imageHeight(0),
		scale(1.0f),
		charDistance(0.0f),
		wordDistance(1.0f),
		lineDistance(1.0f),
		baseLine(0.0f),
		charHeight(0.0f)
	{
		
	}
};

class FontImpl : public Font
{
public:
	FontImpl();
	~FontImpl();
	void Init(video::VideoDriver* driver, const FontCreationData& data);

	void SetMaterial(const video::Material& material);
	const video::Material& GetMaterial() const;
	float GetBaseLine() const;
	void SetBaseLine(float base);
	void Draw(const string& Text, const math::vector2f& Position, EAlign Align, video::Color color, const math::rectf* clip);
	float GetTextWidth(const string& Text, size_t charCount);
	size_t GetCaretFromOffset(const string& Text, float XPosition);
	void GetTextCarets(const string& Text, core::array<float>& carets, size_t charCount);

	float GetFontHeight() const;

	float GetCharDistance() const;
	float GetWordDistance() const;
	float GetLineDistance() const;
	float GetScaling() const;

	void SetLineDistance(float Space);
	void SetCharDistance(float width);
	void SetWordDistance(float Space);
	void SetScaling(float Scale);

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	const CharInfo& GetCharInfo(u32 c);

private:
	video::VideoDriver* m_Driver;

	core::HashMap<u32, CharInfo> m_CharMap;
	float m_CharHeight;

	float m_CharDistance;
	float m_WordDistance;
	float m_LineDistance;
	float m_Scale;
	float m_BaseLine;

	video::Material m_Material;
	FontPixel* m_Image;
	u32 m_ImageWidth;
	u32 m_ImageHeight;

	StrongRef<video::Texture> m_Texture;

	CharInfo m_ErrorChar;
};

}    

}    


#endif
