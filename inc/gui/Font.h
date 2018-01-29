#ifndef INCLUDED_FONT_H
#define INCLUDED_FONT_H
#include "resources/Resource.h"
#include "core/lxString.h"
#include "core/lxArray.h"
#include "math/Rect.h"
#include "video/Color.h"

namespace lux
{
namespace video
{
class Material;
}
namespace gui
{

//! The weight/thickness of a font
enum class EFontWeight
{
	ExtraThin,
	Thin,
	Normal,
	Bolt,
	ExtraBolt
};

//! Contains information about a font.
struct FontDescription
{
	core::String name; //!< The name of the font family, empty if not available

	u32 size; //!< The size of the base font in pixel, 0 if not available
	u32 borderSize; //!< Size of the border in pixels
	EFontWeight weight; //!< The weight of the font
	bool italic; //!< Is the font italic, false if not available
	bool antialiased; //!< Is the font antialiased, false if not available

	FontDescription() :
		size(0),
		borderSize(0),
		weight(EFontWeight::Normal),
		italic(false),
		antialiased(false)
	{
	}

	FontDescription(
		const core::String& _name,
		u32 _size,
		EFontWeight _weight = EFontWeight::Normal,
		bool _italic = false,
		u32 _borderSize = 0,
		bool _antialiased = true) :
		name(_name),
		size(_size),
		borderSize(_borderSize),
		weight(_weight),
		italic(_italic),
		antialiased(_antialiased)
	{
	}
	FontDescription(
		u32 _size,
		EFontWeight _weight = EFontWeight::Normal,
		bool _italic = false,
		u32 _borderSize = 0,
		bool _antialiased = true) :
		size(_size),
		borderSize(_borderSize),
		weight(_weight),
		italic(_italic),
		antialiased(_antialiased)
	{
	}
};

struct FontRenderSettings
{
	/*
	After each printed characters to printing cursor is advanced,
	with charDistance pixels.
	A chardistance of 0 means no additional offset, and only uses
	the default font size.
	*/
	float charDistance = 0.0f;
	/*
	When printing a space character, the width of the space
	character is multiplied by the word distance.
	*/
	float wordDistance = 1.0f;
	/*
	A new-line advances the y cursor with
	the font-height multiplied with the line-distance
	*/
	float lineDistance = 1.0f;
	/*
	All sizes and distances are multiplied with the scale
	before rendering.
	A scale diffrent than 1, can lead to pixellated or
	unsharp fonts.
	*/
	float scale = 1.0f;

	/*
	Slanting of the font.
	Positive to the right, negative to the left.
	*/
	float slanting = 0.0f;

	//! The color of the font.
	video::Color color = video::Color::Black;

	//! The color of the font border(if border available).
	video::Color borderColor = video::Color::Black;
};

//! A font object
/**
Render lines of text with this object, or get information about
textsizes and positioning values
*/
class Font : public core::Resource
{
public:
	Font() {}
	Font(const core::ResourceOrigin& origin) :
		Resource(origin)
	{
	}

	virtual ~Font()
	{
	}

	//! Draw a line of unformated text
	/**
	Characters like newline or tab have no special meaning, i.e.
	will be printed like every other character.
	\param settings Font render settings
	\param text The text to draw
	\param position The position where the text is drawn
	\param clip If non null the text is clipped against this rect
	*/
	virtual void Draw(
		const FontRenderSettings& settings,
		core::Range<core::String::ConstIterator> text,
		const math::Vector2F& position,
		const math::RectF* clip = nullptr) = 0;

	//! Get the width of some text
	/**
	\param settings Font render settings
	\param text The width of this text is calculated
	\return The length of the text, ignores all boundaries like the end of screen
	*/
	virtual float GetTextWidth(const FontRenderSettings& settings,
		core::Range<core::String::ConstIterator> text) = 0;

	//! The text caret from a x position
	/**
	in a text are len(Text)+1 Carets
	|H|e|l|l|o| |W|o|r|l|d|
	each | is a caret, this method returns the id of the nearest caret in the text
	The first caret is the left edge of the text box, the last carret the right edge
	\param settings Font render settings
	\param text The used text
	\param xPosition The text position from the begin of the text, 0.0 is the left edge of the text box
	\return The hit caret
	*/
	virtual size_t GetCaretFromOffset(const FontRenderSettings& settings,
		core::Range<core::String::ConstIterator> text,
		float xPosition) = 0;

	//! All text carets in a given text
	/**
	in a text are len(Text)+1 Carets
	|H|e|l|l|o| |W|o|r|l|d|
	each | is a caret, this method returns the position of all carets in the text
	The first caret is the left edge of the text box, the last carret the right edge
	\param settings Font render settings
	\param text The used text
	\param [out] carets Here the caret positions are written, it always writes exactly CharCount+1 Charrets
	*/
	virtual void GetTextCarets(const FontRenderSettings& settings,
		core::Range<core::String::ConstIterator> text,
		core::Array<float>& carets) = 0;

	//! Retrieve the material used for rendering the font
	/**
	\return The material used for rendering the font
	*/
	virtual const video::Material* GetMaterial() const = 0;

	//! Get a description of the font
	virtual const FontDescription& GetDescription() const = 0;

	virtual float GetBaseLine() const = 0;

	//! Get the height of the font in pixels.
	virtual float GetFontHeight() const = 0;

	//! Set base values for font render settings.
	/**
	These settings are combined with the passed setting for each rendercall
	*/
	virtual void SetBaseFontSettings(const FontRenderSettings& settings) = 0;

	//! Get base values for font render settings.
	virtual const FontRenderSettings& GetBaseFontSettings() = 0;
};

} // namespace gui

} // namespace lux

#endif