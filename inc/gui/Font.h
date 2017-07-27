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
	String name; //!< The name of the font family, empty if not available

	u32 size; //!< The size of the base font in pixel, 0 if not available
	EFontWeight weight; //!< The weight of the font
	bool italic; //!< Is the font italic, false if not available
	bool antialiased; //!< Is the font antialiased, false if not available

	FontDescription()
	{
	}

	FontDescription(
		const String& _name,
		u32 _size,
		EFontWeight _weight = EFontWeight::Normal,
		bool _italic = false,
		bool _antialiased = true) :
		name(_name),
		size(_size),
		weight(_weight),
		italic(_italic),
		antialiased(_antialiased)
	{}
	FontDescription(
		u32 _size,
		EFontWeight _weight = EFontWeight::Normal,
		bool _italic = false,
		bool _antialiased = true) :
		size(_size),
		weight(_weight),
		italic(_italic),
		antialiased(_antialiased)
	{}
};

//! A font object
/**
Render lines of text with this object, or get information about
textsizes and positioning values
*/
class Font : public core::Resource
{
public:
	//! Enumeration giving the alignment of text
	enum class EAlign
	{
		HLeft = 1,          //!< Align to the left
		HCenter = 2,        //!< Align to the horicontal center
		HRight = 4,         //!< Align to the right

		VBottom = 8,        //!< Align to the bottom
		VCenter = 16,       //!< Align to the vertical center
		VTop = 32,          //!< Align to the top

		Centered = VCenter | HCenter,        //!< Align text in the center
		BottomLeft = VBottom | HLeft,       //!< Align to the bottom left
		TopLeft = VTop | HLeft, //!< Align to the top left
		BottomRight = VBottom | HRight,       //!< Align to the bottom right
		TopRight = VTop | HRight, //!< Align to the top right
	};

public:
	virtual ~Font()
	{
	}

	//! Draw a line of unformated text
	/**
	Characters like newline or tab have no special meaning, i.e.
	will be printed like every other character.
	The text is aligned relative to Position, so if Align is Centered
	Position is in the center of the textbox
	if Algin is BottomLeft Position is in the bottom left corner of the textbox
	\param text The text to draw
	\param position The position where the text is drawn
	\param align How to align the text,
	if no vertical align is given, ETA_VBOTTOM is used
	if no horizontal align is given, ETA_HLEFT ist used
	\param color in which color the text is drawn
	\param clip If non null the text is clipped against this rect
	*/
	virtual void Draw(const String& text,
		const math::Vector2F& position,
		EAlign align = EAlign::BottomLeft,
		video::Color color = video::Color::Black,
		const math::RectF* clip = nullptr) = 0;

	//! Get the width of some text
	/**
	\param text The width of this text is calculated
	\param charCount The count of chars of the text to use, max_size_t for all chars
	\return The length of the text, ignores all boundaries like the end of screen
	*/
	virtual float GetTextWidth(const String& text, size_t charCount = std::numeric_limits<size_t>::max()) = 0;

	//! The text caret from a x position
	/**
	in a text are len(Text)+1 Carrets
	|H|a|l|l|o| |W|o|r|l|d|
	each | is a caret, this method returns the id of the nearest caret in the text
	The first caret is the left edge of the text box, the last carret the right edge
	\param text The used text
	\param xPosition The text position from the begin of the text, 0.0 is the left edge of the text box
	\return The hit caret
	*/
	virtual size_t GetCaretFromOffset(const String& text, float xPosition) = 0;

	//! All text carets in a given text
	/**
	in a text are len(Text)+1 Carrets
	|H|a|l|l|o| |W|o|r|l|d|
	each | is a caret, this method returns the position of all carets in the text
	The first caret is the left edge of the text box, the last carret the right edge
	\param text The used text
	\param [out] carets Here the caret positions are written, it always writes exactly CharCount+1 Charrets
	\param charCount The number of chars of the text to use, or max_size_t for all chars
	*/
	virtual void GetTextCarets(const String& text, core::Array<float>& carets, size_t charCount = std::numeric_limits<size_t>::max()) = 0;

	//! Get the height of the font in pixel.
	/**
	Single chars can be much smaller than the font height.
	*/
	virtual float GetFontHeight() const = 0;

	//! Set the character distance
	/**
	After each printed characters to printing cursor is advanced,
	with fontSize*charDistance pixels.
	A chardistance of 0 means no additional offset, and only uses
	the default font size.
	\param space The new character distance.
	*/
	virtual void SetCharDistance(float space) = 0;

	//! Set the word distance
	/**
	When printing a space character, the width of the space
	character is multiplied by the word distance.
	\param space The new word distance.
	*/
	virtual void SetWordDistance(float space) = 0;

	//! Set the line distance.
	/**
	A new-line advances the y cursor about 
	the font-height multiplied with the line-distance
	\param space The new line distance.
	*/
	virtual void SetLineDistance(float space) = 0;

	//! Set the font scaling
	/**
	All sizes and distances are multiplied with the scale
	before rendering.
	A scale diffrent than 1, can lead to pixellated or 
	unsharp fonts.
	*/
	virtual void SetScaling(float scaling) = 0;

	virtual float GetCharDistance() const = 0;
	virtual float GetWordDistance() const = 0;
	virtual float GetLineDistance() const = 0;
	virtual float GetScaling() const = 0;

	//! Get the base line of the text
	/**
	The base line is the line were the text would rest,
	this line is the line drawn to underline text.
	\return The base line of the text, relative to bottom of the textrect in pixel
	*/
	virtual float GetBaseLine() const = 0;

	//! Set the base line of the text
	/**
	\Param base The new base line

	\ref GetBaseLine
	*/
	virtual void SetBaseLine(float base) = 0;

	//! Retrieve the material used for rendering the font
	/**
	\return The new material used for rendering the font
	*/
	virtual const video::Material* GetMaterial() const = 0;

	//! Get a description of the font
	virtual const FontDescription& GetDescription() const = 0;

	core::Name GetResourceType() const
	{
		return core::ResourceType::Font;
	}
};

} // namespace gui

DECLARE_FLAG_CLASS(lux::gui::Font::EAlign);

} // namespace lux

#endif