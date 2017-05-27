#ifndef INCLUDED_BASETEXTURE_H
#define INCLUDED_BASETEXTURE_H
#include "video/Color.h"
#include "math/dimension2d.h"
#include "resources/Resource.h"

namespace lux
{
namespace video
{

//! The base class for all textures
class BaseTexture : public core::Resource
{
public:
	//! Describes a locked texture
	struct LockedRect
	{
		u8* bits; //! Pointer to the texture data
		u32 pitch; //! Number of bytes from one image-line to the next
	};

	//! How should textures be locked
	enum class ELockMode
	{
		//! The complete texture data, will be overwritten
		Overwrite = 0,

		//! Texturedata will only be read
		ReadOnly,

		//! The texturedata will be read and written
		ReadWrite
	};

	struct Filter
	{
		enum EFilter
		{
			Any, //!< Use the default filter
			Point, //!< Use the next pixel to the position
			Linear, //!< Use bilinear filtering
			Anisotropic, //!< Use anistoropic filtering, only for min- and magFilter
		};

		Filter() :
			minFilter(Any),
			magFilter(Any),
			mipFilter(Any)
		{
		}
		EFilter minFilter; //!< The filtering used if the texture is zoomed out
		EFilter magFilter; //!< The filtering used if the texture is zoomed in
		EFilter mipFilter; //!< The filtering used for mip mapping
	};

public:
	virtual ~BaseTexture() {}

	//! The number of mip-maps, these texture contains, 0 for none
	virtual u32 GetLevelCount() const = 0;

	//! Call these to regenerate the mipmaps, after a change of texturedata
	virtual void  RegenerateMIPMaps() = 0;

	//! A pointer to the device depending texture
	virtual void* GetRealTexture() = 0;

	//! Returns the colorformat of the texture
	virtual ColorFormat GetColorFormat() const = 0;

	//! Returns the resolution of a texture surface in pixel
	virtual const math::dimension2du& GetSize() const = 0;

	//! Get the filtering method used for this texture.
	virtual const Filter& GetFiltering() const = 0;

	//! Set the filtering method used for this texture.
	virtual void SetFiltering(const Filter& f) = 0;

	//! Unlock a locked texture
	/**
	No effect if texture is not locked
	*/
	virtual void Unlock() = 0;
};

} // namespace video
} // namespace lux

#endif