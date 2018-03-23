#ifndef INCLUDED_LUX_IMAGE_IMPL_H
#define INCLUDED_LUX_IMAGE_IMPL_H
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
	ImageImpl(const core::ResourceOrigin& origin);
	~ImageImpl();

	void Clear();
	void Init(const math::Dimension2I& size, ColorFormat format);
	void Init(const math::Dimension2I& size, ColorFormat format, void* data, bool CopyMemory, bool deleteOnDrop);

	const math::Dimension2I& GetSize() const;
	ColorFormat GetColorFormat() const;
	int GetBitsPerPixel() const;
	int GetBytesPerPixel() const;
	int GetSizeInBytes() const;
	int GetSizeInPixels() const;
	Color GetPixel(int x, int y);
	void SetPixel(int x, int y, Color Col);
	void Fill(Color color=video::Color::Black);
	int GetPitch() const;
	void* Lock();
	void Unlock();

	core::Name GetReferableType() const
	{
		return core::ResourceType::Image;
	}

private:
	math::Dimension2I m_Dimension;
	ColorFormat m_Format;
	int m_Pitch;
	int m_BitPerPixel;

	u8* m_Data;
	bool m_DeleteOnDrop;

	bool m_Locked;
};

}
}

#endif // INCLUDED_LUX_IMAGE_IMPL_H
