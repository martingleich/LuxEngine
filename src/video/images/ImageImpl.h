#ifndef INCLUDED_IMAGE_IMPL_H
#define INCLUDED_IMAGE_IMPL_H
#include "video/images/Image.h"
#include "core/lxArray.h"
#include "core/lxAlgorithm.h"

namespace lux
{
namespace video
{

class ImageImpl : public Image
{
public:
	ImageImpl();
	~ImageImpl();

	void Clear();
	void Init(const math::dimension2du& size, ColorFormat format);
	void Init(const math::dimension2du& size, ColorFormat format, void* data, bool CopyMemory, bool deleteOnDrop);

	const math::dimension2du& GetDimension() const;
	ColorFormat GetColorFormat() const;
	u32 GetBitsPerPixel() const;
	u32 GetBytesPerPixel() const;
	u32 GetSizeInBytes() const;
	u32 GetSizeInPixels() const;
	Color GetPixel(u32 x, u32 y);
	void SetPixel(u32 x, u32 y, Color Col);
	u32 GetPitch() const;
	void* Lock();
	void Unlock();

	core::Name GetReferableSubType() const
	{
		return core::ResourceType::Image;
	}

	StrongRef<Referable> Clone() const;

private:
	math::dimension2du m_Dimension;
	ColorFormat m_Format;
	u32 m_Pitch;
	u32 m_BytePerPixel;

	u8* m_Data;
	bool m_DeleteOnDrop;

	bool m_Locked;
};

}
}

#endif // INCLUDED_IMAGE_IMPL_H
