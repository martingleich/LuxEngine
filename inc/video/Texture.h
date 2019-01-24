#ifndef INCLUDED_LUX_TEXTURE_H
#define INCLUDED_LUX_TEXTURE_H
#include "video/BaseTexture.h"
#include "core/Resource.h"

namespace lux
{
namespace video
{

class Texture : public BaseTexture
{
public:
	virtual void Init(
		const math::Dimension2I& size,
		ColorFormat format,
		int mipCount, bool isRendertarget, bool isDynamic) = 0;

	virtual const math::Dimension2I& GetSize() const = 0;
	virtual LockedRect Lock(ELockMode mode, int mipLevel) = 0;
	virtual void Unlock(bool regenMipMaps, int mipLevel) = 0;
	virtual int GetMipMapCount() = 0;

	core::Name GetReferableType() const override
	{
		return core::ResourceType::Texture;
	}
};

struct TextureLock
{
	TextureLock(Texture* t, BaseTexture::ELockMode mode, bool regMips = true, int _mipLevel = 0) :
		base(t),
		regenMipMaps(regMips),
		mipLevel(_mipLevel)
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
		mipLevel = old.mipLevel;
		regenMipMaps = old.regenMipMaps;
		old.base = nullptr;
	}

	TextureLock& operator=(const TextureLock& other) = delete;
	TextureLock& operator=(TextureLock&& old)
	{
		if(this == &old)
			return *this;

		std::swap(base, old.base);
		std::swap(data, old.data);
		std::swap(pitch, old.pitch);
		std::swap(mipLevel, old.mipLevel);
		std::swap(regenMipMaps, old.regenMipMaps);
		return *this;
	}
	
	void Unlock()
	{
		if(base) {
			base->Unlock(regenMipMaps, mipLevel);
			base = nullptr;
		}
	}

	Texture* base;
	u8* data;
	u32 pitch;

	bool regenMipMaps;
	int mipLevel;
};

}
}

#endif