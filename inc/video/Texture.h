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

	virtual bool Init(
		const math::dimension2du& Size,
		ColorFormat format,
		u32 MipCount, bool isRendertarget, bool isDynamic) = 0;

	virtual void* Lock(ETextureLockMode mode, SLockedRect* locked = nullptr, u32 mipLevel = 0) = 0;
	virtual void Unlock() = 0;
	virtual bool IsRendertarget() const = 0;

	core::Name GetReferableSubType() const
	{
		return core::ResourceType::Texture;
	}
};

}    // namespace video
}    // namespace lux

#endif