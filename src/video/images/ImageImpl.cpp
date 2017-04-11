#include "ImageImpl.h"
#include "core/lxMemory.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS(lux::video::ImageImpl);

namespace lux
{
namespace video
{

ImageImpl::ImageImpl() :
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

void ImageImpl::Init(const math::dimension2du& size, ColorFormat format)
{
	Clear();

	m_Dimension = size;
	m_Format = format;
	m_DeleteOnDrop = true;
	m_Locked = false;
	m_BytePerPixel = m_Format.GetBytePerPixel();
	m_Pitch = m_Dimension.width * m_BytePerPixel;
	if(size.GetArea() != 0)
		m_Data = LUX_NEW_ARRAY(u8, m_BytePerPixel * m_Dimension.GetArea());
	else
		m_Data = nullptr;
}

void ImageImpl::Init(const math::dimension2du& size, ColorFormat format, void* data, bool CopyMemory, bool deleteOnDrop)
{
	Clear();

	m_Dimension = size;
	m_Format = format;
	m_DeleteOnDrop = deleteOnDrop;
	m_Locked = false;
	m_BytePerPixel = m_Format.GetBytePerPixel();
	m_Pitch = m_Dimension.width * m_BytePerPixel;

	if(CopyMemory) {
		if(m_Dimension.GetArea() != 0) {
			m_Data = LUX_NEW_ARRAY(u8, m_BytePerPixel * m_Dimension.GetArea());
			memcpy(m_Data, data, m_BytePerPixel * m_Dimension.GetArea());
		} else {
			m_Data = nullptr;
		}
	} else {
		m_Data = (u8*)data;
	}
}

const math::dimension2du& ImageImpl::GetDimension() const
{
	return m_Dimension;
}

ColorFormat ImageImpl::GetColorFormat() const
{
	return m_Format;
}

u32 ImageImpl::GetBitsPerPixel() const
{
	return m_BytePerPixel * 8;
}

u32 ImageImpl::GetBytesPerPixel() const
{
	return m_BytePerPixel;
}

u32 ImageImpl::GetSizeInBytes() const
{
	return m_Pitch * m_Dimension.height;
}

u32 ImageImpl::GetSizeInPixels() const
{
	return m_Dimension.width * m_Dimension.height;
}

Color ImageImpl::GetPixel(u32 x, u32 y)
{
	return Color(m_Format.FormatToA8R8G8B8(m_Data + (y * m_Pitch + x*m_BytePerPixel)));
}

void ImageImpl::SetPixel(u32 x, u32 y, Color Col)
{
	void* dst = (m_Data + (y * m_Pitch + x*m_BytePerPixel));
	m_Format.A8R8G8B8ToFormat((u32)Col, (u8*)dst);
}

u32 ImageImpl::GetPitch() const
{
	return m_Pitch;
}

void* ImageImpl::Lock()
{
	if(m_Locked == false) {
		m_Locked = true;
		return m_Data;
	}

	return nullptr;
}

void ImageImpl::Unlock()
{
	m_Locked = false;
}

StrongRef<Referable> ImageImpl::Clone() const
{
	return LUX_NEW(ImageImpl);
}


} 

} 

