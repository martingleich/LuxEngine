#ifndef INCLUDED_ICUBETEXTURE_H
#define INCLUDED_ICUBETEXTURE_H
#include "BaseTexture.h"

namespace lux
{
namespace video
{

//! A texture representing an unfolded cube
class CubeTexture : public BaseTexture
{
public:
	//! The different faces of a cube texture
	enum class EFace
	{
		PosX = 0,
		NegX,
		PosY,
		NegY,
		PosZ,
		NegZ
	};

	virtual ~CubeTexture()
	{
	}

	virtual bool Init(u32 Size, ColorFormat lxFormat, bool isDynamic) = 0;

	//! Retrieve access to the texturedata
	/**
	To make finally changes Unlock() must be called.
	MipMaps won't be recalculated automatically.
	\param mode The texturelock mode
	\param face Which face should be locked
	\param locked output The locked rect
	\param mipLevel Which mipmap level should be locked
	\return A pointer to the texturedata
	*/
	virtual void* Lock(ETextureLockMode mode, EFace face, SLockedRect* locked = nullptr, u32 mipLevel = 0) = 0;

	virtual void Unlock() = 0;

	core::Name GetReferableSubType() const
	{
		return core::ResourceType::CubeTexture;
	}
};

}    

}    


#endif