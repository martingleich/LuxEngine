#ifndef INCLUDED_LUX_ICUBETEXTURE_H
#define INCLUDED_LUX_ICUBETEXTURE_H
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

	CubeTexture(const core::ResourceOrigin& origin) : BaseTexture(origin) {}
	virtual ~CubeTexture() {}

	virtual void Init(int size, ColorFormat lxFormat, bool isRendertarget, bool isDynamic) = 0;

	//! Retrieve access to the texturedata
	/**
	To make finally changes Unlock() must be called.
	MipMaps won't be recalculated automatically.
	\param mode The texturelock mode
	\param face Which face should be locked
	\param mipLevel Which mipmap level should be locked
	\return The locked rectangle
	*/
	virtual LockedRect Lock(ELockMode mode, EFace face, int mipLevel = 0) = 0;

	virtual void Unlock(bool regenMipMaps) = 0;

	core::Name GetReferableType() const
	{
		return core::ResourceType::CubeTexture;
	}

	inline DrawingCanvasAuto<CubeTexture> GetCanvas(ELockMode mode, EFace face, int mipLevel = 0, bool regenMipMaps = false);
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

	DrawingCanvasAuto(CubeTexture* tex, CubeTexture::ELockMode mode, CubeTexture::EFace face, int level, bool _regenMipMaps) :
		DrawingCanvasAuto(tex, tex->Lock(mode, face, level), _regenMipMaps)
	{
	}

	DrawingCanvasAuto(const DrawingCanvasAuto& other) = delete;

	DrawingCanvasAuto(DrawingCanvasAuto&& old)
	{
		(DrawingCanvas&)*this = std::move(old);
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
			texture->Unlock(regenMipMaps);
			texture = nullptr;
		}
	}

	DrawingCanvasAuto& operator=(const DrawingCanvasAuto& other) = delete;

	DrawingCanvasAuto& operator=(DrawingCanvasAuto&& old)
	{
		Unlock();
		(DrawingCanvas&)*this = std::move(old);
		texture = old.texture;
		regenMipMaps = old.regenMipMaps;
		old.texture = nullptr;
		return *this;
	}

	CubeTexture* texture;
	bool regenMipMaps;
};

inline DrawingCanvasAuto<CubeTexture> CubeTexture::GetCanvas(ELockMode mode, EFace face, int mipLevel, bool regenMipMaps)
{
	return DrawingCanvasAuto<CubeTexture>(this, mode, face, mipLevel, regenMipMaps);
}

struct CubeTextureLock
{
	CubeTextureLock(CubeTexture* t, BaseTexture::ELockMode mode, CubeTexture::EFace face, int mipLevel = 0, bool _regenMipMaps = true) :
		base(t),
		regenMipsMaps(_regenMipMaps)
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
		if(base) {
			base->Unlock(regenMipsMaps);
			base = nullptr;
		}
	}

	CubeTexture* base;
	void* data;
	u32 pitch;
	bool regenMipsMaps;
};

} // namespace video
} // namespace lux

#endif