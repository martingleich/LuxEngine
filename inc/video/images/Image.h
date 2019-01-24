#ifndef INCLUDED_LUX_IMAGE_H
#define INCLUDED_LUX_IMAGE_H
#include "core/Referable.h"
#include "math/Dimension2.h"
#include "video/Color.h"
#include "core/lxMemory.h"

namespace lux
{
namespace video
{
//! A image in system memory
class Image : public core::Referable
{
public:
	struct LockedRect
	{
		void* data;
		int pitch;
	};
public:
	LUX_API Image();
	LUX_API ~Image();

	LUX_API void Init(const math::Dimension2I& size, ColorFormat format);

	//! Get the size of the image in pixel
	LUX_API const math::Dimension2I& GetSize() const;

	//! Get the colorformat of the image
	LUX_API ColorFormat GetColorFormat() const;

	//! Locks the image
	/**
	Use this to retrieve access to the image.
	The image can only be locked by one person at the same time
	\return The rawdata of the image, or null if it couldn't be locked

	\ref Unlock
	*/
	LUX_API LockedRect Lock();

	//! Unlocks the image
	/**
	Unlocks a previously locked image.
	If the image isn't locked nothing happens
	*/
	LUX_API void Unlock();

	LUX_API core::Name GetReferableType() const;

private:
	math::Dimension2I m_Dimension;
	ColorFormat m_Format;

	core::RawMemory m_Data;

	bool m_Locked;
};

class ImageLock
{
public:
	ImageLock(Image* i) :
		baseImg(i)
	{
		auto rect = baseImg->Lock();
		data = (u8*)rect.data;
		pitch = rect.pitch;
	}

	ImageLock(const ImageLock& old) = delete;

	ImageLock(ImageLock&& old)
	{
		baseImg = old.baseImg;
		data = old.data;
		pitch = old.pitch;
		old.baseImg = nullptr;
	}

	ImageLock& operator=(const ImageLock& other) = delete;
	ImageLock& operator=(ImageLock&& old)
	{
		Unlock();
		baseImg = old.baseImg;
		data = old.data;
		pitch = old.pitch;
		old.baseImg = nullptr;
		return *this;
	}

	~ImageLock()
	{
		Unlock();
	}

	void Unlock()
	{
		if(baseImg)
			baseImg->Unlock();
		baseImg = nullptr;
		data = nullptr;
		pitch;
	}

	Image* baseImg;
	u8* data;
	int pitch;
};

}
}


#endif // INCLUDED_LUX_IMAGE_H
