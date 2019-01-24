#ifndef INCLUDED_LUX_ICUBETEXTURE_H
#define INCLUDED_LUX_ICUBETEXTURE_H
#include "video/BaseTexture.h"

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

public:
	virtual void Init(int size, ColorFormat lxFormat, bool isRendertarget, bool isDynamic) = 0;

	//! Retrieve access to the texturedata
	/**
	To make finally changes Unlock() must be called.
	\param mode The texturelock mode
	\param face Which face should be locked
	\param mipLevel Which mipmap level should be locked
	\return The locked rectangle
	*/
	virtual LockedRect Lock(ELockMode mode, EFace face) = 0;

	virtual void Unlock() = 0;

	virtual int GetSize() const = 0;

	core::Name GetReferableType() const
	{
		return core::ResourceType::CubeTexture;
	}
};

struct CubeTextureLock
{
	CubeTextureLock(CubeTexture* t, BaseTexture::ELockMode mode, CubeTexture::EFace face) :
		base(t)
	{
		auto rect = base->Lock(mode, face);
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
		if(base) {
			base->Unlock();
			base = nullptr;
		}
	}

	CubeTexture* base;
	void* data;
	u32 pitch;
};

} // namespace video
} // namespace lux

#endif