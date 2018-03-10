#ifndef INCLUDED_FORMAT_SINK_MSGBOX_H
#define INCLUDED_FORMAT_SINK_MSGBOX_H
#include "../Sink.h"
#include <string>
#ifdef FORMAT_WINDOWS

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
	FORMAT_API MsgBox_sink(EIcon icon = EIcon::None, const std::string& caption = "", EButtons button = EButtons::Ok);
	FORMAT_API size_t Write(Context& ctx, const Context::SlicesT& slices, int flags);

	//! Get the pressed button of the message, only available after showing it to the user
	EButton GetPressedButton() const;
	
private:
	EIcon m_Icon;
	std::vector<uint16_t> m_Caption;
	EButtons m_Buttons;
	EButton m_PressedButton;
};

//! Helper function to create a message box
inline MsgBox_sink MsgBox(EIcon icon = EIcon::None, const std::string& utf8Caption = "", EButtons button = EButtons::Ok)
{
	return MsgBox_sink(icon, utf8Caption, button);
}

/** @}*/
}

#endif // #ifndef FORMAT_WINDOWS
#endif // #ifndef INCLUDED_FORMAT_SINK_MSGBOX_H