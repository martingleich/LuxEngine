#ifndef INCLUDED_IMAGESYSTEM_H
#define INCLUDED_IMAGESYSTEM_H
#include "core/ReferenceCounted.h"
#include "math/dimension2d.h"
#include "video/Color.h"

namespace lux
{

namespace video
{
class SpriteBank;
class Image;

//! Manage images and textures
class ImageSystem : public ReferenceCounted
{
public:
	LUX_API static void Initialize(ImageSystem* system = nullptr);
	LUX_API static ImageSystem* Instance();
	LUX_API static void Destroy();

	LUX_API ImageSystem();
	LUX_API virtual ~ImageSystem();

	//! Create a new empty image
	/**
	\param size The size of the new image in pixel
	\param format The colorformat of the new image
	\return The newly created image
	*/
	LUX_API StrongRef<Image> CreateImage(const math::Dimension2U& size, ColorFormat format);

	//! Create a new image from data in memory
	/**
	\param size The size of the new image in pixel
	\param format The colorformat of the new image
	\param data The imagedata of the image, the size and the format must be euqal to the previouse params
	\param copyMem Should the data from the original location be copied or only referenced
	\param deleteOnDrop Only if copyMem=true. Should the referenced memory be deleted if the image is destructed(memory is freed via delete[](u8*) operator)
	\return The newly created image
	*/
	LUX_API StrongRef<Image> CreateImage(const math::Dimension2U& size, ColorFormat format, void* data, bool copyMem, bool deleteOnDrop);

	//! Create s new sprite bank
	LUX_API StrongRef<SpriteBank> CreateSpriteBank();
};

}
}

#endif // !INCLUDED_IIMAGESYSTEM_H

