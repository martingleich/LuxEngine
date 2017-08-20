#ifndef INCLUDED_CWINDOWBASE_H
#define INCLUDED_CWINDOWBASE_H
#include "gui/Window.h"

namespace lux
{
namespace gui
{

class WindowBase : public Window
{
public:
	WindowBase() :
		m_ShouldFullscreen(false),
		m_ClearBackground(true)
	{
	}

	~WindowBase()
	{
	}

	virtual bool SwitchFullscreen(bool fullscreen) = 0;

	bool SetFullscreen(bool fullscreen)
	{
		m_ShouldFullscreen = fullscreen;
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
		return m_IsActivated;
	}
	bool IsFullscreen() const
	{
		return m_ShouldFullscreen;
	}

	bool IsVisible() const
	{
		return !IsMinimized();
	}

	void SetClearBackground(bool clear)
	{
		m_ClearBackground = clear;
	}

	bool ClearBackground() const
	{
		return m_ClearBackground;
	}

protected:
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
			m_IsActivated = true;
			break;
		case EStateChange::Deactivated:
			m_IsActivated = false;
			break;
		}

		onStateChange.Broadcast(this, newState);
	}

	void OnMove(float x, float y)
	{
		float w = m_FinalRect.GetWidth();
		float h = m_FinalRect.GetHeight();
		m_FinalRect.left = x;
		m_FinalRect.top = y;
		m_FinalRect.right = x + w;
		m_FinalRect.bottom = y + h;
		OnFinalRectChange();
		onMove.Broadcast(this, math::Vector2F(x, y));
	}

	void OnResize(float w, float h)
	{
		m_FinalRect.right = m_FinalRect.left + w;
		m_FinalRect.bottom = m_FinalRect.right + h;
		OnFinalRectChange();
		OnInnerRectChange();
		onResize.Broadcast(this, math::Dimension2F(w, h));
	}

	void OnTitleChange(const String& title)
	{
		m_Text = title;
	}

private:
	bool m_ShouldFullscreen; // Should the window if possible be in fullscreen mode, changed by user

	bool m_IsFullscreen;    // The current fullscreen state of the window, changed by system
	bool m_IsMinimized;
	bool m_IsMaximized;

	bool m_IsActivated;
	bool m_IsFocused;
	bool m_ClearBackground;
};

} // namespace gui
} // namespace lux

#endif // !INCLUDED_WINDOWBASE_H
