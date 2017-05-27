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

	virtual void Init(u32 Size, ColorFormat lxFormat, bool isDynamic) = 0;

	//! Retrieve access to the texturedata
	/**
	To make finally changes Unlock() must be called.
	MipMaps won't be recalculated automatically.
	\param mode The texturelock mode
	\param face Which face should be locked
	\param mipLevel Which mipmap level should be locked
	\return The locked rectangle
	*/
	virtual LockedRect Lock(ELockMode mode, EFace face, u32 mipLevel = 0) = 0;

	virtual void Unlock() = 0;

	core::Name GetReferableSubType() const
	{
		return core::ResourceType::CubeTexture;
	}
};

struct CubeTextureLock
{
	CubeTextureLock(CubeTexture* t, BaseTexture::ELockMode mode, CubeTexture::EFace face, u32 mipLevel = 0) :
		base(t)
	{
		auto rect = base->Lock(mode, face, mipLevel);
		data = rect.bits;
		pitch = rect.pitch;
	}

	~CubeTextureLock()
	{
		Unlock();
	}

	CubeTextureLock(const CubeTextureLock& old) = delete;

	CubeTextureLock(CubeTextureLock&& old)
	{
		base = old.base;
		data = old.data;
		pitch = old.pitch;
		old.base = nullptr;
	}

	CubeTextureLock& operator=(const CubeTextureLock& other) = delete;
	CubeTextureLock& operator=(CubeTextureLock&& old)
	{
		Unlock();
		base = old.base;
		data = old.data;
		pitch = old.pitch;
		old.base = nullptr;
		return *this;
	}
	void Unlock()
	{
		if(base)
			base->Unlock();
		base = nullptr;
	}

	CubeTexture* base;
	void* data;
	u32 pitch;
};


}
}

#endif