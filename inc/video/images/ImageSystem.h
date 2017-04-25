#ifndef INCLUDED_IIMAGESYSTEM_H
#define INCLUDED_IIMAGESYSTEM_H
#include "resources/ResourceSystem.h"
#include "math/dimension2d.h"
#include "video/Color.h"

namespace lux
{

namespace io
{
class FileSystem;
class File;
}

namespace video
{

class VideoDriver;
class BaseTexture;
class Texture;
class CubeTexture;
class SpriteBank;

class Image;
class ImageLoader;
class ImageWriter;

//! How should new textures created
enum ETextureCreationFlag : u32
{
	//! Forces a 16 bit colordepth, can not be used with ETCF_ALWAYS_32_BIT
	ETCF_ALWAYS_16_BIT = 0x00000001,

	//! Forces a 32 bit colordepth, can not be used with ETCF_ALWAYS_16_BIT
	ETCF_ALWAYS_32_BIT = 0x00000002,

	//! Should mip maps be generated
	ETCF_CREATE_MIP_MAPS = 0x00000004,

	//! Should the texture have an alpha channel
	/**
	Is set to 1 if no alpha is available
	*/
	ETCF_ALPHA_CHANNEL = 0x00000008,
};

//! Manage images and textures
class ImageSystem : public ReferenceCounted
{
public:
	virtual ~ImageSystem()
	{
	}

	//! Create a new empty image
	/**
	\param size The size of the new image in pixel
	\param format The colorformat of the new image
	\return The newly created image or NULL if an error occured
	*/
	virtual StrongRef<Image> CreateImage(const math::dimension2du& size, ColorFormat format) = 0;

	//! Create a new image from data in memory
	/**
	\param size The size of the new image in pixel
	\param format The colorformat of the new image
	\param data The imagedata of the image, the size and the format must be euqal to the previouse params
	\param copyMem Should the data from the original location be copied or only referenced
	\param deleteOnDrop Only if copyMem=true. Should the referenced memory be deleted if the image is destructed.
	\return The newly created image or NULL if an error occured
	*/
	virtual StrongRef<Image> CreateImage(const math::dimension2du& size, ColorFormat format, void* data, bool copyMem, bool deleteOnDrop) = 0;

	//! Write a image from memory to file
	/**
	The filetype is determined by the ending of the name of file
	\param image The image to write to memory
	\param file The file where the image is written must be a writable file
	\return True if the image was written otherwise false
	*/
	virtual bool WriteImageToFile(Image* image, io::File* file) = 0;

	//! Write a image from memory to file
	/**
	The filetype is determined by the ending of the path
	\param image The image to write to memory
	\param path The path were the image is written
	\return True if the image was written otherwise false
	*/
	virtual bool WriteImageToFile(Image* image, const io::path& path) = 0;

	//! Write a image from memory to file
	/**
	The filetype is determined by the ending of the name of file
	\param size The size of the image
	\param format The colorformat of the image
	\param data A pointer to the image data
	\param file The file where the image is written must be a writable file
	\return True if the image was written otherwise false
	*/
	virtual bool WriteImageDataToFile(const math::dimension2du& size, ColorFormat format, void* data, io::File* file) = 0;

	//! Write a image from memory to file
	/**
	The filetype is determined by the ending of the path
	\param size The size of the image
	\param format The colorformat of the image
	\param data A pointer to the image data
	\param path The path where the image is written
	\return True if the image was written otherwise false
	*/
	virtual bool WriteImageDataToFile(const math::dimension2du& size, ColorFormat format, void* data, const io::path& path) = 0;

	//! Add a new image writer
	/**
	Here you can specifiey own image writer to extend the engine with new formats.
	New image writers overwrite old ones.
	\param writer The new image writer
	*/
	virtual void AddExternalImageWriter(ImageWriter* writer) = 0;

	//! Retrieve the number of available image writers
	/**
	\return The number of image writers
	*/
	virtual size_t GetImageWriterCount() const = 0;

	//! Retrieve a image writer based on an index
	/**
	\param index The index of the image writer.
	\return The specified image writer or NULL if no such loader exists
	*/
	virtual StrongRef<ImageWriter> GetImageWriter(size_t index) = 0;

	//! Retrieve a image writer based on the file extension of the given file
	/**
	\param name The name of a file you would like to write
	\return The image writer which can write this file
	*/
	virtual StrongRef<ImageWriter> GetImageWriter(const io::path& name) = 0;

	virtual bool SetTextureCreationFlag(ETextureCreationFlag flag, bool set) = 0;
	virtual bool GetTextureCreationFlag(ETextureCreationFlag flag) const = 0;

	virtual StrongRef<Texture> AddTexture(const string& name, const math::dimension2du& size, ColorFormat format, bool isDynamic = false) = 0;
	virtual StrongRef<Texture> AddTexture(const string& name, Image* image, bool isDynamic = false) = 0;

	virtual StrongRef<Texture> AddChromaKeyedTexture(video::Image* image, video::Color key) = 0;
	virtual StrongRef<Texture> GetChromaKeyedTexture(const io::path& p, video::Color key) = 0;
	virtual StrongRef<Texture> GetChromaKeyedTexture(const io::path& p, const math::vector2i& pos) = 0;

	virtual StrongRef<CubeTexture> AddCubeTexture(const string& name, StrongRef<Image> images[6]) = 0;
	virtual StrongRef<CubeTexture> AddCubeTexture(const string& name, ColorFormat format, u32 size) = 0;

	virtual StrongRef<Texture> AddRendertargetTexture(const string& name, const math::dimension2du& size, ColorFormat format) = 0;

	virtual StrongRef<SpriteBank> CreateSpriteBank() = 0;

	//virtual Texture* AddRendertargetCubeTexture(u32 size, const string& name = "rt", ColorFormat format = UNKNOWN) = 0;
};


}
}

#endif // !INCLUDED_IIMAGESYSTEM_H

