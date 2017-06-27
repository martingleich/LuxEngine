#ifndef INCLUDED_CWINDOWBASE_H
#define INCLUDED_CWINDOWBASE_H
#include "gui/Window.h"

namespace lux
{
namespace gui
{

class WindowBase : public Window
{
private:
	WindowEventCallback* m_FirstCallback;

	bool m_IsFullscreen;    // The current fullscreen state of the window, changed by system
	bool m_ShouldFullscreen; // Should the window if possible be in fullscreen mode, changed by user

	bool m_IsMinimized;
	bool m_IsActive;
	bool m_IsFocused;
	bool m_IsMaximized;
	math::dimension2du m_Size;
	math::vector2i m_Position;
	string m_Title;

public:
	WindowBase() :
		m_FirstCallback(nullptr),
		m_ShouldFullscreen(false)
	{
	}

	~WindowBase()
	{
	}

	virtual bool SwitchFullscreen(bool fullscreen) = 0;

	void OnResize(u32 x, u32 y)
	{
		m_Size.Set(x, y);
		WindowEventCallback* callback = m_FirstCallback;
		while(callback) {
			callback->OnResize(*this, m_Size);
			callback = callback->m_Next;
		}
	}

	void OnMove(u32 x, u32 y)
	{
		m_Position.Set(x, y);
		WindowEventCallback* callback = m_FirstCallback;
		while(callback) {
			callback->OnMove(*this, m_Position);
			callback = callback->m_Next;
		}
	}

	bool OnClosing()
	{
		if(!m_FirstCallback)
			return true;

		bool ret = false;
		WindowEventCallback* callback = m_FirstCallback;
		while(callback) {
			ret |= callback->OnClosing(*this);
			callback = callback->m_Next;
		}

		return ret;
	}

	void OnClose()
	{
		WindowEventCallback* callback = m_FirstCallback;
		while(callback) {
			callback->OnClose(*this);
			callback = callback->m_Next;
		}
	}

	void OnStateChange(Window::EStateChange newState)
	{
		switch(newState) {
		case EStateChange::FocusGained:
			m_IsFocused = true;
			break;
		case EStateChange::FocusLost:
			m_IsFocused = false;
			break;
		case EStateChange::Fullscreen:
			m_IsFullscreen = true;
			break;
		case EStateChange::Maximize:
			m_IsMinimized = false;
			m_IsMaximized = true;
			break;
		case EStateChange::Minimize:
			m_IsMinimized = true;
			m_IsMaximized = false;
			break;
		case EStateChange::Normal:
			m_IsMinimized = false;
			m_IsMaximized = false;
			break;
		case EStateChange::Activated:
			m_IsActive = true;
			break;
		case EStateChange::Deactivated:
			m_IsActive = false;
			break;
		}

		WindowEventCallback* callback = m_FirstCallback;
		while(callback) {
			callback->OnStateChanged(*this, newState);
			callback = callback->m_Next;
		}
	}

	void OnTitleChange(const string& title)
	{
		m_Title = title;
	}

	void RegisterCallback(WindowEventCallback* call)
	{
		if(call->m_Window) {
			if(call->m_Window == this)
				return;
			else
				call->m_Window->UnregisterCallback(call);
		}

		call->m_Prev = nullptr;
		if(m_FirstCallback) {
			call->m_Next = m_FirstCallback;
			m_FirstCallback->m_Prev = call;
			m_FirstCallback = call;
		} else {
			call->m_Next = nullptr;
			m_FirstCallback = call;
		}

		call->m_Window = this;
	}

	void UnregisterCallback(WindowEventCallback* call)
	{
		if(call->m_Window != this)
			return;

		if(call->m_Next)
			call->m_Next->m_Prev = call->m_Prev;
		if(call->m_Prev)
			call->m_Prev->m_Next = call->m_Next;

		if(call == m_FirstCallback)
			m_FirstCallback = call->m_Next;

		call->m_Next = nullptr;
		call->m_Prev = nullptr;
		call->m_Window = nullptr;
	}

	const string& GetTitle() const
	{
		return m_Title;
	}

	const math::dimension2du& GetSize() const
	{
		return m_Size;
	}

	const math::vector2i& GetPosition() const
	{
		return m_Position;
	}

	bool SetFullscreen(bool Fullscreen)
	{
		m_ShouldFullscreen = Fullscreen;
		return SwitchFullscreen(m_ShouldFullscreen);
	}

	bool IsMinimized() const
	{
		return m_IsMinimized;
	}
	bool IsMaximized() const
	{
		return m_IsMaximized;
	}
	bool IsFocused() const
	{
		return m_IsFocused;
	}
	bool IsActive() const
	{
		return m_IsActive;
	}
	bool IsFullscreen() const
	{
		return m_ShouldFullscreen;
	}
};

}
}

#endif // !INCLUDED_CWINDOWWIN32_H
