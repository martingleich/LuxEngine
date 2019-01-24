#include "video/images/Image.h"
#include "core/lxMemory.h"

LX_REGISTER_REFERABLE_CLASS(lux::video::Image, "lux.resource.Image");

namespace lux
{
namespace video
{

Image::Image()
{
}

Image::~Image()
{
}

void Image::Init(const math::Dimension2I& size, ColorFormat format)
{
	m_Dimension = size;
	m_Format = format;
	m_Locked = false;

	m_Data.SetMinSize(m_Dimension.GetArea() * m_Format.GetBytePerPixel(), core::RawMemory::ZERO);
}

const math::Dimension2I& Image::GetSize() const
{
	return m_Dimension;
}

ColorFormat Image::GetColorFormat() const
{
	return m_Format;
}

Image::LockedRect Image::Lock()
{
	if(m_Locked)
		throw core::InvalidOperationException("Image is already locked");

	m_Locked = true;
	LockedRect r;
	r.data = m_Data;
	r.pitch = m_Dimension.width * m_Format.GetBytePerPixel();
;
	return r;
}

void Image::Unlock()
{
	m_Locked = false;
}

core::Name Image::GetReferableType() const
{
	return core::ResourceType::Image;
}

}
}

