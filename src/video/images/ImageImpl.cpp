#include "ImageImpl.h"
#include "core/lxMemory.h"
#include "video/DrawingCanvas.h"

LX_REGISTER_RESOURCE_CLASS("lux.resource.Image", lux::video::ImageImpl);

namespace lux
{
namespace video
{

ImageImpl::ImageImpl(const core::ResourceOrigin& origin) :
	Image(origin),
	m_Data(nullptr)
{
}

ImageImpl::~ImageImpl()
{
	Clear();
}

void ImageImpl::Clear()
{
	if(m_DeleteOnDrop)
		LUX_FREE_ARRAY(m_Data);
	m_Data = nullptr;
}

void ImageImpl::Init(const math::Dimension2U& size, ColorFormat format)
{
	Clear();

	m_Dimension = size;
	m_Format = format;
	m_DeleteOnDrop = true;
	m_Locked = false;
	m_BitPerPixel = m_Format.GetBitsPerPixel();
	m_Pitch = (m_Dimension.width * m_BitPerPixel)/8;
	if(size.GetArea() != 0)
		m_Data = LUX_NEW_ARRAY(u8, (m_BitPerPixel * m_Dimension.GetArea())/8);
	else
		m_Data = nullptr;
}

void ImageImpl::Init(const math::Dimension2U& size, ColorFormat format, void* data, bool CopyMemory, bool deleteOnDrop)
{
	Clear();

	m_Dimension = size;
	m_Format = format;
	m_DeleteOnDrop = deleteOnDrop;
	m_Locked = false;
	m_BitPerPixel = m_Format.GetBitsPerPixel();
	m_Pitch = (m_Dimension.width * m_BitPerPixel)/8;

	if(CopyMemory) {
		if(m_Dimension.GetArea() != 0) {
			m_Data = LUX_NEW_ARRAY(u8, (m_BitPerPixel * m_Dimension.GetArea())/8);
			memcpy(m_Data, data, (m_BitPerPixel * m_Dimension.GetArea())/8);
		} else {
			m_Data = nullptr;
		}
	} else {
		m_Data = (u8*)data;
	}
}

const math::Dimension2U& ImageImpl::GetSize() const
{
	return m_Dimension;
}

ColorFormat ImageImpl::GetColorFormat() const
{
	return m_Format;
}

u32 ImageImpl::GetBitsPerPixel() const
{
	return m_BitPerPixel;
}

u32 ImageImpl::GetSizeInBytes() const
{
	return (m_BitPerPixel * m_Dimension.GetArea())/8;
}

u32 ImageImpl::GetSizeInPixels() const
{
	return m_Dimension.GetArea();
}

Color ImageImpl::GetPixel(u32 x, u32 y)
{
	lxAssert(!m_Format.IsCompressed());

	return Color(m_Format.FormatToA8R8G8B8(m_Data + (y * m_Pitch + (x*m_BitPerPixel)/8)));
}

void ImageImpl::SetPixel(u32 x, u32 y, Color Col)
{
	lxAssert(!m_Format.IsCompressed());

	void* dst = (m_Data + (y * m_Pitch + (x*m_BitPerPixel)/8));
	m_Format.A8R8G8B8ToFormat((u32)Col, (u8*)dst);
}

void ImageImpl::Fill(Color color)
{
	DrawingCanvas c(m_Data, m_Format, m_Dimension, m_Pitch);
	c.Clear(color);
}

u32 ImageImpl::GetPitch() const
{
	return m_Pitch;
}

void* ImageImpl::Lock()
{
	if(m_Locked)
		throw core::Exception("Image is already locked");

	m_Locked = true;
	return m_Data;
}

void ImageImpl::Unlock()
{
	m_Locked = false;
}

}
}

