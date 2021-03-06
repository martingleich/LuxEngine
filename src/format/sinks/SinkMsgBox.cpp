#include "format/sinks/SinkMsgBox.h"
#ifdef FORMAT_WINDOWS
#include "format/UnicodeConversion.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace format
{
static std::string GetDefaultTitle(EIcon icon)
{
	switch(icon) {
	case EIcon::Error: return "Error";
	case EIcon::Warning: return "Warning";
	case EIcon::Information: return "Information";
	case EIcon::Question: return "Question";
	case EIcon::None: return "MsgBox";
	default: return "Title";
	}
}

static UINT GetIconFlags(EIcon icon)
{
	switch(icon) {
	case EIcon::Error:
		return MB_ICONERROR;
	case EIcon::Warning:
		return MB_ICONWARNING;
	case EIcon::Information:
		return MB_ICONINFORMATION;
	case EIcon::Question:
		return MB_ICONQUESTION;
	case EIcon::None:
		return 0;
	default:
		return 0;
	}
}

static UINT GetButtonsFlags(EButtons buttons)
{
	switch(buttons) {
	case EButtons::AbortRetryIgnore: return MB_ABORTRETRYIGNORE;
	case EButtons::CancleTryContinue: return MB_CANCELTRYCONTINUE;
	case EButtons::Ok: return MB_OK;
	case EButtons::OkCancel: return MB_OKCANCEL;
	case EButtons::RetryCancel: return MB_RETRYCANCEL;
	case EButtons::YesNo: return MB_YESNO;
	case EButtons::YesNoCancel: return MB_YESNOCANCEL;
	default: return MB_OK;
	}
}

static UINT GetFlags(EButtons buttons, EIcon icon)
{
	return GetIconFlags(icon) | GetButtonsFlags(buttons);
}

static EButton GetButton(int i)
{
	switch(i) {
	case IDABORT: return EButton::Abort;
	case IDCANCEL: return EButton::Cancel;
	case IDCONTINUE: return EButton::Continue;
	case IDIGNORE: return EButton::Ignore;
	case IDNO: return EButton::No;
	case IDOK: return EButton::Ok;
	case IDRETRY: return EButton::Retry;
	case IDTRYAGAIN: return EButton::TryAgain;
	case IDYES: return EButton::Yes;
	default: return EButton::Invalid;
	}
}

MsgBox_sink::MsgBox_sink(EIcon icon, const std::string& caption, EButtons button) :
	m_Icon(icon),
	m_Buttons(button),
	m_PressedButton(EButton::Invalid)
{
	auto _caption = !caption.empty() ? caption : GetDefaultTitle(m_Icon);
	Utf8ToUtf16(_caption.data(), _caption.size(), m_Caption);
	m_Caption.push_back(0);
}

int MsgBox_sink::Write(Context& ctx, const Context::SlicesT& slices, int flags)
{
	(void)flags;

	int size = ctx.GetSize();
	std::vector<uint16_t> utf16Msg;
	utf16Msg.reserve(size + 1);
	for(auto& s : slices)
		Utf8ToUtf16(s.data, s.size, utf16Msg);
	utf16Msg.push_back(0);

	UINT boxflags = GetFlags(m_Buttons, m_Icon);
	int ret = MessageBoxW(nullptr,
		(LPCWSTR)utf16Msg.data(),
		(LPCWSTR)m_Caption.data(),
		boxflags);

	m_PressedButton = GetButton(ret);

	return size;
}

EButton MsgBox_sink::GetPressedButton() const
{
	return m_PressedButton;
}

}

#endif