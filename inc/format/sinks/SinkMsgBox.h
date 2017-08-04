#ifndef INCLUDED_FORMAT_SINK_MSGBOX_H
#define INCLUDED_FORMAT_SINK_MSGBOX_H
#include "../Sink.h"
#ifdef LUX_WINDOWS

namespace format
{

//! The icon used in the message box
enum class EIcon
{
	Error,
	Warning,
	Information,
	Question,
	None
};

//! Buttons shown in the message box
enum class EButtons
{
	AbortRetryIgnore,
	CancleTryContinue,
	Ok,
	OkCancel,
	RetryCancel,
	YesNo,
	YesNoCancel,
};

//! Button pressed in the message box
enum class EButton
{
	Abort,
	Cancel,
	Continue,
	Ignore,
	No,
	Ok,
	Retry,
	TryAgain,
	Yes,

	Invalid
};

/** \addtogroup Sinks
@{
*/

//! Allows to write to a message box
/**
Currently only available for Windows
*/
class MsgBox_sink : public Sink
{
public:
	MsgBox_sink(EIcon icon = EIcon::None, const char* Utf8Caption = nullptr, EButtons button = EButtons::Ok) :
		m_Icon(icon),
		m_Caption(Utf8Caption),
		m_Buttons(button),
		m_PressedButton(EButton::Invalid)
	{
	}
	LUX_API size_t Write(Context& ctx, const Slice* firstSlice, int flags);

	//! Get the pressed button of the message, only available after showing it to the user
	EButton GetPressedButton() const;
	
private:
	EIcon m_Icon;
	const char* m_Caption;
	EButtons m_Buttons;
	EButton m_PressedButton;
};

//! Helper function to create a message box
inline MsgBox_sink MsgBox(EIcon icon = EIcon::None, const char* Utf8Caption = nullptr, EButtons button = EButtons::Ok)
{
	return MsgBox_sink(icon, Utf8Caption, button);
}

/** @}*/
}

#endif // #ifndef LUX_WINDOWS
#endif // #ifndef INCLUDED_FORMAT_SINK_MSGBOX_H