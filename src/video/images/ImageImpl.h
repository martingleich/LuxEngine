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

class ImageListImpl : public ImageList
{
public:
	size_t GetImageCount() const
	{
		return m_Images.Size();
	}

	void AddImage(Image* img)
	{
		if(img)
			m_Images.Push_Back(img);
	}

	StrongRef<Image> GetImage(size_t i) const
	{
		if(i < GetImageCount())
			return m_Images[i];
		else
			return nullptr;
	}

	void RemoveImage(Image* img)
	{
		auto newEnd = core::Remove(m_Images.First(), m_Images.End(), img);
		m_Images.Resize(core::IteratorDistance(m_Images.First(), newEnd));
	}

	void Clear()
	{
		m_Images.Clear();
	}

	core::Name GetReferableSubType() const
	{
		return core::ResourceType::ImageList;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(ImageListImpl)(*this);
	}

private:
	core::array<StrongRef<Image>> m_Images;
};

}
}

#endif // INCLUDED_IMAGE_IMPL_H
