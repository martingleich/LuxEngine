#ifndef INCLUDED_IBASETEXTURE_H
#define INCLUDED_IBASETEXTURE_H
#include "video/Color.h"
#include "math/dimension2d.h"
#include "resources/ResourceSystem.h"

namespace lux
{
namespace video
{

//! The base class for all textures
class BaseTexture : public core::Resource
{
public:
	//! Describes a locked texture
	struct SLockedRect
	{
		void* bits; //! Pointer to the texture data
		u32 pitch; //! Number of bytes from one image-line to the next
	};

	//! How should textures be locked
	enum ETextureLockMode
	{
		//! The complete texture data, will be overwritten
		ETLM_OVERWRITE = 0,

		//! Texturedata will only be read
		ETLM_READ_ONLY,

		//! The texturedata will be read and written
		ETLM_READ_WRITE
	};

public:
	virtual ~BaseTexture()
	{
	}

	//! The number of mip-maps, these texture contains, 0 for none
	virtual u32 GetLevelCount() const
	{
		return 0;
	}

	//! Call these to regenerate the mipmaps, after a change of texturedata
	virtual void  RegenerateMIPMaps() = 0;

	//! A pointer to the device depending texture
	virtual void* GetRealTexture() = 0;

	//! Returns the colorformat of the texture
	virtual ColorFormat GetColorFormat() const = 0;

	//! Returns the resolution of a texture surface in pixel
	virtual const math::dimension2du& GetDimension() const = 0;

	//! Unlock a locked texture
	/**
	No effect if texture is not locked
	*/
	virtual void Unlock() = 0;
};

}    // namespace video
}    // namespace lux

#endif