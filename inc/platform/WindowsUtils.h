#ifndef INCLUDED_LUX_PLATFORM_WINDOW_UTILS_H
#define INCLUDED_LUX_PLATFORM_WINDOW_UTILS_H
#include "platform/Win32Exception.h"
#include "core/HelperTemplates.h"

namespace lux
{
inline void Win32CloseHandleWrapper(HANDLE h)
{
	CloseHandle(h);
}
template <void (*Release)(HANDLE)=Win32CloseHandleWrapper, HANDLE InvalidHandle=INVALID_HANDLE_VALUE>
class HandleWrapper : core::Uncopyable
{
public:
	HandleWrapper() :
		m_Handle(InvalidHandle)
	{
	}
	HandleWrapper(HANDLE h) :
		m_Handle(h)
	{
	}
	~HandleWrapper()
	{
		if(m_Handle != InvalidHandle) {
			Release(m_Handle);
			m_Handle = InvalidHandle;
		}
	}
	HandleWrapper(HandleWrapper&& old)
	{
		m_Handle = old.m_Handle;
		old.m_Handle = InvalidHandle;
	}
	HandleWrapper& operator=(HandleWrapper&& old)
	{
		this->~HandleWrapper();
		m_Handle = old.m_Handle;
		old.m_Handle = InvalidHandle;
		return *this;
	}

	HANDLE* Access()
	{
		this->~HandleWrapper();
		return &m_Handle;
	}

	operator HANDLE() const { return m_Handle; }

private:
	HANDLE m_Handle;
};

using Win32FileHandle = HandleWrapper<>;

} // namespace lux

#endif // #ifndef INCLUDED_LUX_PLATFORM_WINDOW_UTILS_H