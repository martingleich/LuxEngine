#ifndef INCLUDED_IMAGE_H
#define INCLUDED_IMAGE_H
#include "resources/ResourceSystem.h"
#include "math/dimension2d.h"
#include "video/Color.h"

namespace lux
{
namespace video
{

//! A image in system memory
class Image : public core::Resource
{
public:
	virtual ~Image()
	{
	}

	virtual void Init(const math::dimension2du& size, ColorFormat format) = 0;
	virtual void Init(const math::dimension2du& size, ColorFormat format, void* data, bool CopyMemory, bool deleteOnDrop) = 0;

	//! Get the size of the image in pixel
	virtual const math::dimension2du& GetDimension() const = 0;

	//! Get the colorformat of the image
	virtual ColorFormat GetColorFormat() const = 0;

	//! The number of bits per image pixel
	virtual u32 GetBitsPerPixel() const = 0;

	//! The number of bytes per image pixel
	virtual u32 GetBytesPerPixel() const = 0;

	//! The size of the whole image in bytes
	virtual u32 GetSizeInBytes() const = 0;

	//! The size of the whole image in pixels
	virtual u32 GetSizeInPixels() const = 0;

	//! Get a single pixel of the image
	/**
	If the pixel is another format than A8R8G8B8 is automatically converted
	\param x The x coordinate of the pixel
	\param y The y coordinate of the pixel
	\return The color of the pixel
	*/
	virtual Color GetPixel(u32 x, u32 y) = 0;

	//! Set a single pixel of the image
	/**
	If the pixel is another format than A8R8G8B8 is automatically converted
	\param x The x coordinate of the pixel
	\param y The y coordinate of the pixel
	\param color The new color of the pixel
	*/
	virtual void SetPixel(u32 x, u32 y, Color color) = 0;

	//! Get the pitch of the image
	/**
	The pitch is the number of bytes between the begin of one line and the start of the next.
	\return The pitch of the image
	*/
	virtual u32 GetPitch() const = 0;

	//! Locks the image
	/**
	Use this to retrieve access to the image.
	The image can only be locked by one person at the same time
	\return The rawdata of the image, or null if it couldn't be locked

	\ref Unlock
	*/
	virtual void* Lock() = 0;

	//! Unlocks the image
	/**
	Unlocks a previously locked image.
	If the image isn't locked nothing happens
	*/
	virtual void Unlock() = 0;

	/*
	TODO: Add create empty image method to image system.
	TODO: Allow null image, as empty image
	TODO: SetFormat(newFormat)
	TODO: SetSize(newSize, copyRect)
	TODO: Fill-Methode, füllt das Bild mit einer beliebigen Farbe
	*/
};


class ImageList : public core::Resource
{
public:
	virtual size_t GetImageCount() const = 0;
	virtual void AddImage(Image* img) = 0;
	virtual StrongRef<Image> GetImage(size_t i) const = 0;
	virtual void RemoveImage(Image* img) = 0;
	virtual void Clear() = 0;
};

} 

} 


#endif // INCLUDED_IMAGE_H
