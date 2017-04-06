#include "ImageLoaderTGA.h"
#include "io/File.h"
#include "video/images/Image.h"

#include "../external/libtga/src/tga.h"

namespace lux
{
namespace video
{

static uint32_t tga_proc_read(tga_struct* tga, uint32_t size, void* buffer)
{
	io::File* file = (io::File*)tga_get_io_data(tga);
	return file->ReadBinary(size, buffer);
}

static uint32_t tga_proc_seek(tga_struct* tga, uint32_t offset)
{
	io::File* file = (io::File*)tga_get_io_data(tga);
	bool success = file->Seek(offset, io::ESeekOrigin::Start);

	return success ? 0 : 1;
}
ImageLoaderTGA::ImageLoaderTGA() :
	m_Tga(nullptr),
	m_Info(nullptr),
	m_CurFile(nullptr)
{
}

const string& ImageLoaderTGA::GetName() const
{
	static const string name = "Lux TGA-Loader";
	return name;
}

core::Name ImageLoaderTGA::GetResourceType(io::File* file, core::Name requestedType)
{
	if(requestedType && requestedType != core::ResourceType::Image)
		return core::Name::INVALID;

	LoadTgaInfo(file, true);
	if(m_Info)
		return core::ResourceType::Image;
	else
		return core::Name::INVALID;
}

bool ImageLoaderTGA::LoadResource(io::File* file, core::Resource* dst)
{
	bool result;

	video::Image* img = dynamic_cast<video::Image*>(dst);
	if(!img)
		return false;

	math::dimension2du size;
	video::ColorFormat format;
	result = LoadImageFormat(file, size, format);
	if(!result)
		return false;

	img->Init(size, format);

	void* data = img->Lock();
	if(!data)
		return false;
	result = LoadImageToMemory(file, data);
	img->Unlock();
	if(!result)
		return false;

	return true;
}

bool ImageLoaderTGA::LoadImageFormat(io::File* file, math::dimension2du& outSize, video::ColorFormat& outFormat)
{
	LoadTgaInfo(file, false);
	if(!m_Info)
		return false;

	uint32_t flags = tga_info_get_flags(m_Info);
	flags &= ~TGA_FLAG_COLOR_BGR;
	flags |= TGA_FLAG_COLOR_RGB;
	flags &= ~TGA_FLAG_BOTTOM;
	flags &= ~TGA_FLAG_RIGHT;
	flags &= ~TGA_FLAG_RLE;

	bool result = true;
	if(tga_info_set_flags(m_Info, flags))
		result = false;

	if(!result && !tga_is_info_valid(m_Info))
		result = false;

	if(!result) {
		CleanUp();
		return false;
	}

	if(flags & TGA_FLAG_ALPHA)
		outFormat = video::ColorFormat::A8R8G8B8;
	else
		outFormat = video::ColorFormat::R8G8B8;

	outSize.width = tga_info_get_width(m_Info);
	outSize.height = tga_info_get_height(m_Info);

	return true;
}

bool ImageLoaderTGA::LoadImageToMemory(io::File* file, void* dest)
{
	if(file != m_CurFile)
		return false;

	uint32_t result = tga_read_image(m_Tga, m_Info, dest);

	CleanUp();

	return (result == TGA_OK);
}

void ImageLoaderTGA::CleanUp()
{
	if(m_Info) {
		tga_destroy_info(m_Info);
		m_Info = nullptr;
	}
	if(m_Tga) {
		tga_destroy(m_Tga);
		m_Tga = nullptr;
	}

	m_CurFile = nullptr;
}

bool ImageLoaderTGA::LoadTgaInfo(io::File* file, bool silent)
{
	if(file == m_CurFile)
		return true;

	CleanUp();
	m_CurFile = file;

	m_Tga = tga_init_read(nullptr);
	if(!m_Tga)
		return false;

	tga_set_silent(m_Tga, silent?1:0);

	uint32_t result = TGA_OK;
	result |= tga_set_io_read_proc(m_Tga, tga_proc_read);
	result |= tga_set_io_seek_proc(m_Tga, tga_proc_seek);
	result |= tga_set_io_data(m_Tga, (void*)file);

	if(result != TGA_OK) {
		CleanUp();
		return false;
	}

	m_Info = tga_init_info(m_Tga);
	if(!m_Info) {
		CleanUp();
		return false;
	}

	if(tga_read_info(m_Tga, m_Info)) {
		CleanUp();
		return false;
	}

	return true;
}

}
}
