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
		Free();
	}
	HandleWrapper(HandleWrapper&& old)
	{
		m_Handle = old.m_Handle;
		old.m_Handle = InvalidHandle;
	}
	HandleWrapper& operator=(HandleWrapper&& old)
	{
		Free();
		m_Handle = old.m_Handle;
		old.m_Handle = InvalidHandle;
		return *this;
	}

	HANDLE* Access()
	{
		Free();
		return &m_Handle;
	}

	operator HANDLE() const { return m_Handle; }

private:
	void Free()
	{
		if(m_Handle != InvalidHandle) {
			Release(m_Handle);
			m_Handle = InvalidHandle;
		}
	}
private:
	HANDLE m_Handle;
};

template <typename T, void (*Release)(T)>
class PointerWrapper : public core::Uncopyable
{
public:
	PointerWrapper() :
		m_Handle(nullptr)
	{
	}
	PointerWrapper(T h) :
		m_Handle(h)
	{
	}
	~PointerWrapper()
	{
		Free();
	}
	PointerWrapper(PointerWrapper&& old)
	{
		m_Handle = old.m_Handle;
		old.m_Handle = nullptr;
	}
	PointerWrapper& operator=(PointerWrapper&& old)
	{
		Free();
		m_Handle = old.m_Handle;
		old.m_Handle = nullptr;
		return *this;
	}

	T* Access()
	{
		Free();
		return &m_Handle;
	}

	operator T() const { return m_Handle; }

	T operator->() const { return m_Handle; }

private:
	void Free()
	{
		if(m_Handle != nullptr) {
			Release(m_Handle);
			m_Handle = nullptr;
		}
	}
private:
	T m_Handle;
};

using Win32FileHandle = HandleWrapper<>;

} // namespace lux

#endif // #ifndef INCLUDED_LUX_PLATFORM_WINDOW_UTILS_H