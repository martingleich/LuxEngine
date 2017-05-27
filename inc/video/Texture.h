#ifndef INCLUDED_TEXTURE_H
#define INCLUDED_TEXTURE_H
#include "BaseTexture.h"

namespace lux
{
namespace video
{

class Texture : public BaseTexture
{
public:
	virtual ~Texture()
	{
	}

	virtual void Init(
		const math::dimension2du& Size,
		ColorFormat format,
		u32 MipCount, bool isRendertarget, bool isDynamic) = 0;

	virtual LockedRect Lock(ELockMode mode, u32 mipLevel = 0) = 0;
	virtual void Unlock() = 0;
	virtual bool IsRendertarget() const = 0;

	core::Name GetReferableSubType() const
	{
		return core::ResourceType::Texture;
	}
};

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