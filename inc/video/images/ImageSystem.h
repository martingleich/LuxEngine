#ifndef INCLUDED_LUX_IMAGESYSTEM_H
#define INCLUDED_LUX_IMAGESYSTEM_H
#include "core/ReferenceCounted.h"
#include "math/Dimension2.h"
#include "video/Color.h"

namespace lux
{

namespace video
{
class Texture;
class CubeTexture;
class VideoDriver;
class SpriteBank;
class Image;

//! Manage images and textures
class ImageSystem : public ReferenceCounted
{
	ImageSystem();
public:
	LUX_API static void Initialize();
	LUX_API static ImageSystem* Instance();
	LUX_API static void Destroy();

	LUX_API ~ImageSystem();

	//! Create a new empty image
	/**
	\param size The size of the new image in pixel
	\param format The colorformat of the new image
	\return The newly created image
	*/
	LUX_API StrongRef<Image> CreateImage(const math::Dimension2I& size, ColorFormat format);

	//! Create new sprite bank
	LUX_API StrongRef<SpriteBank> CreateSpriteBank();

	LUX_API StrongRef<Texture> CreateFittingTexture(
		const math::Dimension2I& size,
		ColorFormat format = ColorFormat::R8G8B8,
		int mipCount = 0,
		bool isDynamic = false);
	LUX_API StrongRef<CubeTexture> CreateFittingCubeTexture(
		int size,
		ColorFormat format = ColorFormat::R8G8B8,
		bool isDynamic = false);
	LUX_API StrongRef<Texture> CreateFittingRendertargetTexture(
		const math::Dimension2I& size,
		ColorFormat format);

private:
	VideoDriver* m_Driver;
};

}
}

#endif // !INCLUDED_LUX_IIMAGESYSTEM_H

