#ifndef INCLUDED_TEXTURE_H
#define INCLUDED_TEXTURE_H
#include "BaseTexture.h"
#include "DrawingCanvas.h"

namespace lux
{
namespace video
{
class Texture;

template<>
class DrawingCanvasAuto<Texture>;

class Texture : public BaseTexture
{
public:
	Texture(const core::ResourceOrigin& origin) : BaseTexture(origin) {}

	virtual ~Texture()
	{
	}

	virtual void Init(
		const math::Dimension2U& Size,
		ColorFormat format,
		u32 MipCount, bool isRendertarget, bool isDynamic) = 0;

	virtual LockedRect Lock(ELockMode mode, u32 mipLevel = 0) = 0;
	virtual void Unlock() = 0;
	virtual bool IsRendertarget() const = 0;

	core::Name GetReferableType() const
	{
		return core::ResourceType::Texture;
	}

	inline DrawingCanvasAuto<Texture> GetCanvas(ELockMode mode, u32 mipLevel = 0, bool regenMipMaps=true);
};

template <>
class DrawingCanvasAuto<Texture> : public DrawingCanvas
{
public:
	DrawingCanvasAuto(Texture* tex, const Texture::LockedRect& r, bool _regenMipMaps) :
		DrawingCanvas(r.bits, tex->GetColorFormat(), tex->GetSize(), r.pitch),
		texture(tex),
		regenMipMaps(_regenMipMaps)
	{
	}

	DrawingCanvasAuto(Texture* tex, Texture::ELockMode mode, u32 level, bool _regenMipMaps) :
		DrawingCanvasAuto(tex, tex->Lock(mode, level), _regenMipMaps)
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
		(DrawingCanvas&)*this = std::move(old);
		texture = old.texture;
		regenMipMaps = old.regenMipMaps;
		old.texture = nullptr;
		return *this;
	}

	Texture* texture;
	bool regenMipMaps;
};

inline DrawingCanvasAuto<Texture> Texture::GetCanvas(ELockMode mode, u32 mipLevel, bool regenMipMaps)
{
	return DrawingCanvasAuto<Texture>(this, mode, mipLevel, regenMipMaps);
}

struct TextureLock
{
	TextureLock(Texture* t, BaseTexture::ELockMode mode, bool regMips = true, u32 mipLevel = 0) :
		base(t),
		regenerateMips(regMips)
	{
		auto rect = base->Lock(mode, mipLevel);
		data = rect.bits;
		pitch = rect.pitch;
	}

	~TextureLock()
	{
		Unlock();
	}

	TextureLock(const TextureLock& old) = delete;

	TextureLock(TextureLock&& old)
	{
		base = old.base;
		data = old.data;
		pitch = old.pitch;
		old.base = nullptr;
	}

	TextureLock& operator=(const TextureLock& other) = delete;
	TextureLock& operator=(TextureLock&& old)
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
			if(regenerateMips)
				base->RegenerateMIPMaps();
		}
		base = nullptr;
	}

	Texture* base;
	u8* data;
	u32 pitch;

	bool regenerateMips;
};

}
}

#endif