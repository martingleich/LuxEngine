#ifndef INCLUDED_IMAGELISTIMPL_H
#define INCLUDED_IMAGELISTIMPL_H
#include "video/images/Image.h"
#include "core/lxArray.h"
#include "core/lxAlgorithm.h"

namespace lux
{
namespace video
{

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

#endif // #ifndef INCLUDED_IMAGELISTIMPL_H
