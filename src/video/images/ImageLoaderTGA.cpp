#include "ImageLoaderTGA.h"
#include "io/File.h"
#include "video/images/Image.h"
#include "math/dimension2d.h"
#include "video/Color.h"

#include "../external/libtga/src/tga.h"

namespace lux
{
namespace video
{

namespace
{
struct Context
{
	tga_struct* tga;
	tga_info* info;

	math::dimension2du size;
	video::ColorFormat format;

	Context() :
		tga(nullptr),
		info(nullptr)
	{}

	~Context()
	{
		tga_destroy_info(info);
		tga_destroy(tga);
	}
};
}

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

static bool LoadTgaInfo(Context& ctx, io::File* file, bool silent)
{
	ctx.tga = tga_init_read(nullptr);
	if(!ctx.tga)
		return false;

	tga_set_silent(ctx.tga, silent ? 1 : 0);

	uint32_t result = TGA_OK;
	result |= tga_set_io_read_proc(ctx.tga, tga_proc_read);
	result |= tga_set_io_seek_proc(ctx.tga, tga_proc_seek);
	result |= tga_set_io_data(ctx.tga, (void*)file);

	if(result != TGA_OK)
		return false;

	ctx.info = tga_init_info(ctx.tga);
	if(!ctx.info)
		return false;

	if(tga_read_info(ctx.tga, ctx.info))
		return false;

	return true;
}

static bool LoadImageFormat(Context& ctx)
{
	uint32_t flags = tga_info_get_flags(ctx.info);
	flags &= ~TGA_FLAG_COLOR_BGR;
	flags |= TGA_FLAG_COLOR_RGB;
	flags &= ~TGA_FLAG_BOTTOM;
	flags &= ~TGA_FLAG_RIGHT;
	flags &= ~TGA_FLAG_RLE;

	bool result = true;
	if(tga_info_set_flags(ctx.info, flags))
		result = false;

	if(!result && !tga_is_info_valid(ctx.info))
		result = false;

	if(!result)
		return false;

	if(flags & TGA_FLAG_ALPHA)
		ctx.format = video::ColorFormat::A8R8G8B8;
	else
		ctx.format = video::ColorFormat::R8G8B8;

	ctx.size.width = tga_info_get_width(ctx.info);
	ctx.size.height = tga_info_get_height(ctx.info);

	return true;
}

static bool LoadImageToMemory(Context& ctx, void* dest)
{
	uint32_t result = tga_read_image(ctx.tga, ctx.info, dest);

	return (result == TGA_OK);
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

	Context ctx;
	if(LoadTgaInfo(ctx, file, true))
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

	Context ctx;
	if(!LoadTgaInfo(ctx, file, false))
		return false;

	if(!LoadImageFormat(ctx))
		return false;

	img->Init(ctx.size, ctx.format);

	void* data = img->Lock();
	if(!data)
		return false;
	result = LoadImageToMemory(ctx, data);
	img->Unlock();
	if(!result)
		return false;

	return true;
}


}
}
