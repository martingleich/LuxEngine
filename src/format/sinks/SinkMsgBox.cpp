#ifdef FORMAT_WINDOWS
#include "format/sinks/SinkMsgBox.h"
#include "format/UnicodeConversion.h"
#include <string>
#include <codecvt>
#include <locale>

#include <Windows.h>

namespace format
{
static const char* GetDefaultTitle(EIcon icon)
{
	switch(icon) {
	case EIcon::Error:
		return u8"Error";
	case EIcon::Warning:
		return u8"Warning";
	case EIcon::Information:
		return u8"Information";
	case EIcon::Question:
		return u8"Question";
	case EIcon::None:
		return u8"MsgBox";
	default:
		return nullptr;
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
	case EButtons::AbortRetryIgnore:
		return MB_ABORTRETRYIGNORE;
	case EButtons::CancleTryContinue:
		return MB_CANCELTRYCONTINUE;
	case EButtons::Ok:
		return MB_OK;
	case EButtons::OkCancel:
		return MB_OKCANCEL;
	case EButtons::RetryCancel:
		return MB_RETRYCANCEL;
	case EButtons::YesNo:
		return MB_YESNO;
	case EButtons::YesNoCancel:
		return MB_YESNOCANCEL;
	default:
		return 0;
	}
}

static UINT GetFlags(EButtons buttons, EIcon icon)
{
	return GetIconFlags(icon) | GetButtonsFlags(buttons);
}

static EButton GetButton(int i)
{
	switch(i) {
	case IDABORT:
		return EButton::Abort;
	case IDCANCEL:
		return EButton::Cancel;
	case IDCONTINUE:
		return EButton::Continue;
	case IDIGNORE:
		return EButton::Ignore;
	case IDNO:
		return EButton::No;
	case IDOK:
		return EButton::Ok;
	case IDRETRY:
		return EButton::Retry;
	case IDTRYAGAIN:
		return EButton::TryAgain;
	case IDYES:
		return EButton::Yes;
	default:
		return EButton::Invalid;
	}
}

size_t MsgBox_sink::Write(Context& ctx, const Slice* firstSlice, int flags)
{
	(void)flags;
	
	size_t size = 1;
	for(auto slice = firstSlice; slice; slice = slice->GetNext())
		size += slice->size;

	char* str = ctx.AllocByte(size);
	char* c = str;

	for(auto slice = firstSlice; slice; slice = slice->GetNext()) {
		memcpy(c, slice->data, slice->size);
		c += slice->size;
	}

	*c++ = 0;

	const char* caption = m_Caption ? m_Caption : GetDefaultTitle(m_Icon);
	UINT boxflags = GetFlags(m_Buttons, m_Icon);
	int ret;

	auto utf16Caption = Utf8ToUtf16(caption, strlen(caption)+1);
	std::vector<uint16_t> utf16Msg;
	if(ctx.stringType == StringType::Ascii || ctx.stringType == StringType::Unicode)
		utf16Msg = Utf8ToUtf16(str, size);
	else if(ctx.stringType == StringType::CodePoint)
		utf16Msg = CodePointsToUtf16((const uint32_t*)str, size / 4);
	else
		throw not_implemeted_exception("StringType not supported.");

	ret = MessageBoxW(nullptr,
		(LPCWSTR)utf16Msg.data(),
		(LPCWSTR)utf16Caption.data(),
		boxflags);

	m_PressedButton = GetButton(ret);

	return size-1;
}

EButton MsgBox_sink::GetPressedButton() const
{
	return m_PressedButton;
}

}

#endif