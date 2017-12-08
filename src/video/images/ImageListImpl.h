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
	LX_REFERABLE_MEMBERS_API(LUX_API);

	size_t GetImageCount() const
	{
		return m_Images.Size();
	}

	void AddImage(Image* img)
	{
		if(img)
			m_Images.PushBack(img);
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
		auto newEnd = core::Remove(m_Images, img);
		m_Images.Resize(core::IteratorDistance(m_Images.First(), newEnd));
	}

	void Clear()
	{
		m_Images.Clear();
	}

private:
	core::Array<StrongRef<Image>> m_Images;
};

}
}

#endif // #ifndef INCLUDED_IMAGELISTIMPL_H
