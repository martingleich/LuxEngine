#ifndef INCLUDED_ICUBETEXTURE_H
#define INCLUDED_ICUBETEXTURE_H
#include "video/BaseTexture.h"
#include "video/DrawingCanvas.h"

namespace lux
{
namespace video
{
class CubeTexture;

template<>
class DrawingCanvasAuto<CubeTexture>;

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

	inline DrawingCanvasAuto<CubeTexture> GetCanvas(ELockMode mode, EFace face, u32 mipLevel = 0, bool regenMipMaps=false);
};

template <>
class DrawingCanvasAuto<CubeTexture> : public DrawingCanvas
{
public:
	DrawingCanvasAuto(CubeTexture* tex, const CubeTexture::LockedRect& r, bool _regenMipMaps) :
		DrawingCanvas(r.bits, tex->GetColorFormat(), tex->GetSize(), r.pitch),
		texture(tex),
		regenMipMaps(_regenMipMaps)
	{
	}

	DrawingCanvasAuto(CubeTexture* tex, CubeTexture::ELockMode mode, CubeTexture::EFace face, u32 level, bool _regenMipMaps) :
		DrawingCanvasAuto(tex, tex->Lock(mode, face, level), _regenMipMaps)
	{
	}

	DrawingCanvasAuto(const DrawingCanvasAuto& other) = delete;

	DrawingCanvasAuto(DrawingCanvasAuto&& old)
	{
		texture = old.texture;
		regenMipMaps = old.regenMipMaps;
		old.texture = nullptr;
	}

	~DrawingCanvasAuto()
	{
		Unlock();
	}

	void Unlock()
	{
		if(texture) {
			texture->Unlock();
			if(regenMipMaps)
				texture->RegenerateMIPMaps();
			texture = nullptr;
		}
	}

	DrawingCanvasAuto& operator=(const DrawingCanvasAuto& other) = delete;

	DrawingCanvasAuto& operator=(DrawingCanvasAuto&& old)
	{
		Unlock();
		texture = old.texture;
		regenMipMaps = old.regenMipMaps;
		old.texture = nullptr;
		return *this;
	}

	CubeTexture* texture;
	bool regenMipMaps;
};

inline DrawingCanvasAuto<CubeTexture> CubeTexture::GetCanvas(ELockMode mode, EFace face, u32 mipLevel, bool regenMipMaps)
{
	return DrawingCanvasAuto<CubeTexture>(this, mode, face, mipLevel, regenMipMaps);
}

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