#pragma once
#include "Sink.h"
#include "FormatConfig.h"

namespace format
{

enum class EIcon
{
	Error,
	Warning,
	Information,
	Question,
	None
};

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

class MsgBox_sink : public sink
{
public:
	MsgBox_sink(EIcon icon = EIcon::None, const char* Utf8Caption = nullptr, EButtons button = EButtons::Ok) :
		m_Icon(icon),
		m_Caption(Utf8Caption),
		m_Buttons(button),
		m_PressedButton(EButton::Invalid)
	{
	}
	size_t Write(Context& ctx, const slice* firstSlice, int flags);

	EButton GetPressedButton() const;
private:
	const char* m_Caption;
	EIcon m_Icon;
	EButtons m_Buttons;
	EButton m_PressedButton;
};

inline MsgBox_sink MsgBox(EIcon icon = EIcon::None, const char* Utf8Caption = nullptr, EButtons button = EButtons::Ok)
{
	return MsgBox_sink(icon, Utf8Caption, button);
}

}
