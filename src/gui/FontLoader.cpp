#include "FontLoader.h"
#include "io/file.h"
#include "core/Logger.h"
#include "video/images/ImageSystem.h"
#include "video/VideoDriver.h"
#include "core/lxhashmap.h"
#include "gui/FontImpl.h"
#include "video/Texture.h"
#include "video/MaterialLibrary.h"

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

bool FontLoader::LoadResource(io::File* file, core::Resource* dst)
{
	if(!m_Driver || !m_ImageSystem || !m_MaterialLibrary)
		return false;

	return LoadFontFromFile(file, dst);
}

const string& FontLoader::GetName() const
{
	static const string name = "Lux Font Loader";
	return name;
}

bool FontLoader::LoadFontFromFile(io::File* file, core::Resource* dst)
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
	};

	struct SCharInfo
	{
		wchar_t character;
		u16 Left;
		u16 Top;
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
	if(Magic != LX_MAKE_FOURCC('F', 'O', 'N', 'T')) {
		return nullptr;
	}

	u32 Version = 0;
	for(int i = 0; i < 4; ++i) {
		Version *= 10;
		Version += VersionStr[i] - '0';
	}

	if(Version != 1)
		return nullptr;

	core::HashMap<u32, gui::CharInfo> charMap;
	video::Texture* fontTexture;
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

				Internal.Left = (float)(CharInfo.Left) * InvWidth;
				Internal.Top = (float)(CharInfo.Top) * InvHeight;

				Internal.Right = (float)(CharInfo.Left + CharInfo.B) * InvWidth;
				Internal.Bottom = (float)(CharInfo.Top + info.height) * InvHeight;

				charMap.Set(CharInfo.character, Internal);
			}
		}
		break;
		case LX_MAKE_FOURCC('I', 'M', 'A', 'G'):
		{
			SImageInfo image;
			file->ReadBinary(sizeof(SImageInfo), &image);

			fontTexture = m_ImageSystem->AddTexture(file->GetName(), math::dimension2du(image.TextureWidth, image.TextureHeight), video::ColorFormat::A8R8G8B8, false);

			video::BaseTexture::SLockedRect rect;
			u8* rowData = (u8*)(fontTexture->Lock(video::BaseTexture::ETLM_OVERWRITE, &rect));

			if(image.Compression == 0) {
				u8 value;
				for(u32 y = 0; y < image.TextureHeight; ++y) {
					u8* pixel = rowData;
					for(u32 x = 0; x < image.TextureWidth; ++x) {
						if(file->ReadBinary(1, &value) == 0) {
							return nullptr;
						}

						*pixel = value;        // Alpha
						++pixel;
						*pixel = value;        // Red
						++pixel;
						*pixel = value;        // Green
						++pixel;
						*pixel = value;        // Blue
						++pixel;
					}

					rowData += rect.pitch;
				}
			}

			fontTexture->Unlock();
			fontTexture->RegenerateMIPMaps();
		}
		break;
		default:
			file->Seek(Header.Size);
		}
	}

	FontCreationData data;
	data.charMap = charMap;
	data.charHeight = info.height;
	CharInfo spaceInfo = data.charMap[' '];
	data.material = video::Material(m_MaterialLibrary->GetMaterialRenderer("font"));
	data.material.Layer(0) = fontTexture;
	data.material.Param("BlendFunc") = (u32)video::Pack_TextureBlendFunc(video::EBF_SRC_ALPHA, video::EBF_ONE_MINUS_SRC_ALPHA);
	data.charDistance = 0.0f;
	data.wordDistance = 1.0f;
	data.lineDistance = 1.0f;
	data.scale = 1.0f;
	data.baseLine = 0.0f;

	gui::FontImpl* real_dst = dynamic_cast<gui::FontImpl*>(dst);
	if(!real_dst)
		return false;
	real_dst->Init(m_Driver, data);
	return true;
}

}
}