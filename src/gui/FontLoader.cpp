#include "FontLoader.h"
#include "io/File.h"
#include "core/Logger.h"
#include "video/images/ImageSystem.h"
#include "video/images/Image.h"
#include "video/VideoDriver.h"
#include "core/lxHashMap.h"
#include "gui/FontImpl.h"
#include "video/MaterialLibrary.h"
#include "video/AlphaSettings.h"

namespace lux
{
namespace gui
{

core::Name FontLoader::GetResourceType(io::File* file, core::Name requestedType)
{
	if(requestedType && requestedType != core::ResourceType::Font)
		return core::Name::INVALID;

	u32 magic = 0;
	const u32 bytes = file->ReadBinary(sizeof(u32), &magic);
	if(bytes != 4 || magic != LX_MAKE_FOURCC('F', 'O', 'N', 'T'))
		return core::Name::INVALID;
	else
		return core::ResourceType::Font;
}

void FontLoader::LoadResource(io::File* file, core::Resource* dst)
{
	if(!video::VideoDriver::Instance() || !video::MaterialLibrary::Instance())
		throw core::Exception("Font loader is missing driver, imageSystem or materialLibrary");

	LoadFontFromFile(file, dst);
}

const String& FontLoader::GetName() const
{
	static const String name = "Lux Font Loader";
	return name;
}

void FontLoader::LoadFontFromFile(io::File* file, core::Resource* dst)
{
#pragma pack(push, 1)
	struct SFontInfo
	{
		u16 height : 16;
		float LineSpace;
		float CharSpace;
		u16 charCount;
		u16 TextureWidth : 16;
		u16 TextureHeight : 16;

		SFontInfo() :
			height(0),
			LineSpace(0.0f),
			CharSpace(0.0f),
			charCount(0),
			TextureWidth(0),
			TextureHeight(0)
		{
		}
	};

	struct SCharInfo
	{
		u16 character;
		u16 left;
		u16 top;
		u8 B;
		signed char C;
		signed char A;
	};

	struct SImageInfo
	{
		u16 TextureWidth : 16;
		u16 TextureHeight : 16;
		u16 Compression : 16;
	};

	struct SChunkHead
	{
		u32 type;
		u32 Size;
	};
#pragma pack(pop)

	u32 Magic;
	char VersionStr[4];
	file->ReadBinary(sizeof(u32), &Magic);
	file->ReadBinary(sizeof(u32), &VersionStr);
	if(Magic != LX_MAKE_FOURCC('F', 'O', 'N', 'T'))
		throw core::FileFormatException("Invalid magic number", "font");

	u32 Version = 0;
	for(int i = 0; i < 4; ++i) {
		Version *= 10;
		Version += VersionStr[i] - '0';
	}

	if(Version != 1)
		throw core::FileFormatException("Unsupported version", "font");

	core::HashMap<u32, gui::CharInfo> charMap;
	StrongRef<video::Image> fontImage;
	SFontInfo info;
	while(file->IsEOF() == false) {
		SChunkHead Header;
		if(file->ReadBinary(sizeof(SChunkHead), &Header) != sizeof(SChunkHead))
			break;

		switch(Header.type) {
		case LX_MAKE_FOURCC('I', 'N', 'F', 'O'):
		{
			file->ReadBinary(sizeof(SFontInfo), &info);
			charMap.Reserve(info.charCount);
			const float InvHeight = 1.0f / (info.TextureHeight - 1.0f);
			const float InvWidth = 1.0f / (info.TextureWidth - 1.0f);
			SCharInfo CharInfo;
			gui::CharInfo Internal;
			for(int c = 0; c < info.charCount; ++c) {
				file->ReadBinary(sizeof(SCharInfo), &CharInfo);
				Internal.A = (float)(CharInfo.A);
				Internal.B = (float)(CharInfo.B);
				Internal.C = (float)(CharInfo.C);

				Internal.left = (float)(CharInfo.left) * InvWidth;
				Internal.top = (float)(CharInfo.top) * InvHeight;

				Internal.right = (float)(CharInfo.left + CharInfo.B) * InvWidth;
				Internal.bottom = (float)(CharInfo.top + info.height) * InvHeight;

				charMap.Set(CharInfo.character, Internal);
			}
		}
		break;
		case LX_MAKE_FOURCC('I', 'M', 'A', 'G'):
		{
			SImageInfo image;
			file->ReadBinary(sizeof(SImageInfo), &image);
			fontImage = video::ImageSystem::Instance()->CreateImage(math::Dimension2U(image.TextureWidth, image.TextureHeight), video::ColorFormat::X8);

			{
				video::ImageLock imgLock(fontImage);

				u8* rowData = imgLock.data;
				if(image.Compression == 0) {
					u8 value;
					for(u32 y = 0; y < image.TextureHeight; ++y) {
						u8* pixel = rowData;
						for(u32 x = 0; x < image.TextureWidth; ++x) {
							if(file->ReadBinary(1, &value) == 0)
								throw core::FileFormatException("Unexpected end of file", "font");

							*pixel = value;
							++pixel;
						}

						rowData += imgLock.pitch;
					}
				}
			}
		}
		break;
		default:
			file->Seek(Header.Size);
		}
	}

	{
		video::ImageLock imgLock(fontImage);

		FontCreationData data;
		data.desc.name = file->GetName();
		data.desc.size = 0;
		data.desc.antialiased = false;
		data.desc.italic = false;
		data.desc.weight = EFontWeight::Normal;

		data.charMap = charMap;
		data.charHeight = info.height;
		data.charDistance = 0.0f;
		data.wordDistance = 1.0f;
		data.lineDistance = 1.0f;
		data.scale = 1.0f;
		data.baseLine = 0.0f;
		data.image = (FontPixel*)imgLock.data;
		data.imageSize = fontImage->GetSize();

		gui::FontImpl* real_dst = dynamic_cast<gui::FontImpl*>(dst);
		if(!real_dst)
			throw core::Exception("Passed wrong resource type to loader");
		real_dst->Init(data);
	}
}

}
}
