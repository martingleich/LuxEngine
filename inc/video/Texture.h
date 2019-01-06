#ifndef INCLUDED_LUX_TEXTURE_H
#define INCLUDED_LUX_TEXTURE_H
#include "BaseTexture.h"

namespace lux
{
namespace video
{
class Texture;

class Texture : public BaseTexture
{
public:
	Texture(const core::ResourceOrigin& origin) : BaseTexture(origin) {}
	virtual ~Texture() {}

	virtual void Init(
		const math::Dimension2I& size,
		ColorFormat format,
		int mipCount, bool isRendertarget, bool isDynamic) = 0;

	virtual LockedRect Lock(ELockMode mode, int mipLevel = 0) = 0;
	virtual void Unlock(bool regenMipMaps) = 0;

	core::Name GetReferableType() const
	{
		return core::ResourceType::Texture;
	}
};

struct TextureLock
{
	TextureLock(Texture* t, BaseTexture::ELockMode mode, bool regMips = true, int mipLevel = 0) :
		base(t),
		regenMipMaps(regMips)
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
			base->Unlock(regenMipMaps);
			base = nullptr;
		}
	}

	Texture* base;
	u8* data;
	u32 pitch;

	bool regenMipMaps;
};

}
}

#endif