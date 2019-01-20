#include "FontFormat.h"
#include "gui/FontRaster.h"
#include "io/File.h"

// #define FONT_NO_COMPRESS 
namespace lux
{
namespace gui
{

namespace
{

enum class EImageCompressionV1
{
	None = 0,
	RLE = 1
};

enum class EFontWeightsV1
{
	ExtraThin = 0,
	Thin = 1,
	Normal = 2,
	Bolt = 3,
	ExtraBolt = 4,
};

enum EFontFlagsV1
{
	FontFlagV1_Italic,
	FontFlagV1_Antialiased,
};

static const u32 CharInfoV1Size = 6 * 4;
struct CharInfoV1
{
	u32 character;
	float left;
	float top;

	float A;
	float B;
	float C;
};

/*
	u32 = Unsigned 32 bit int
	u8 = Unsiged 8 bit int
	Fraction = Fixed point cecimal number safed as signed 32 bit int divided by 10000

	u32 nameBytes; maximal 512 byte
	u8[nameBytes] name;
	u32 fontSize;
	u32 borderSize;
	u32 weight;
	u32 flags;
	u32 charCount;

	u32 channelCount;
	u32 imageWidth;
	u32 imageHeight;
	u32 imageCompression;

	Fraction charHeight;

	Fraction scale;
	Fraction charDistance;
	Fraction wordDistance;
	Fraction lineDistance;
	Fraction baseLine;
	Fraction slanting;
*/
static const u32 FontInfoV1SizeWithoutName = 52;
struct FontInfoV1
{
	core::String name;

	int fontSize;
	int borderSize;
	EFontWeightsV1 weight;
	bool italic;
	bool antialiased;
	int charCount;

	int channelCount;
	int imageWidth;
	int imageHeight;
	EImageCompressionV1 imageCompression;

	float charHeight;

	float scale;
	float charDistance;
	float wordDistance;
	float lineDistance;
	float baseLine;
	float slanting;
};

struct Context
{
	Context(io::File* f) :
		m_File(f),
		m_BufferSize(0),
		m_BufferCursor(0)
	{
	}

	u32 ReadHeader()
	{
		char versionStr[5];
		u32 magic = ReadU32();
		ReadBytes(4, versionStr);
		versionStr[4] = 0;
		if(magic != LX_MAKE_FOURCC('F', 'O', 'N', 'T'))
			throw core::FileFormatException("Invalid magic number", "lxf");

		m_Version = (u32)core::StringConverter::ParseInt(versionStr);

		return m_Version;
	}

	void WriteHeader(const char version[4])
	{
		u32 magic = LX_MAKE_FOURCC('F', 'O', 'N', 'T');
		m_File->WriteBinary(&magic, 4);
		m_File->WriteBinary(version, 4);
	}

	void ReadV1(FontCreationData& outData)
	{
		// Read and convert font info
		FontInfoV1 info;
		ReadV1Info(info);
		outData.baseLine = info.baseLine;
		outData.charHeight = info.charHeight;
		outData.desc.borderSize = info.borderSize;
		outData.desc.antialiased = info.antialiased;
		outData.desc.italic = info.italic;
		outData.desc.name = info.name;
		outData.desc.size = info.fontSize;
		outData.desc.weight = ConvertV1Weight(info.weight);
		outData.channelCount = info.channelCount;
		outData.imageSize.width = info.imageWidth;
		outData.imageSize.height = info.imageHeight;
		outData.baseSettings.charDistance = info.charDistance;
		outData.baseSettings.lineDistance = info.lineDistance;
		outData.baseSettings.scale = info.scale;
		outData.baseSettings.slanting = info.slanting;
		outData.baseSettings.wordDistance = info.wordDistance;

		// Read char settings
		float invHeight = 1.0f / info.imageHeight;
		float invWidth = 1.0f / info.imageWidth;
		outData.charMap.Reserve(info.charCount);
		for(int i = 0; i < info.charCount; ++i) {
			CharInfoV1 v1Info;
			ReadCharInfo(v1Info);
			CharInfo c;
			c.A = v1Info.A;
			c.B = v1Info.B;
			c.C = v1Info.C;
			c.left = v1Info.left;
			c.top = v1Info.top;
			c.bottom = v1Info.top + info.charHeight*invHeight;
			c.right = v1Info.left + c.B*invWidth;
			outData.charMap[v1Info.character] = c;
		}

		// Read image
		u32 imageBytes = info.channelCount * info.imageWidth * info.imageHeight;
		m_ImageData.SetMinSize(imageBytes);
		if(info.imageCompression == EImageCompressionV1::None) {
			ReadBytes(imageBytes, m_ImageData);
		} else if(info.imageCompression == EImageCompressionV1::RLE) {
			core::RawMemory compressed;
			for(int c = 0; c < info.channelCount; ++c) {
				u32 compressedBytes = ReadU32();
				compressed.SetMinSize(compressedBytes);
				ReadBytes(compressedBytes, compressed);
				u8* outCur = m_ImageData;
				for(u32 i = 0; i < compressedBytes; i += 2) {
					int r = ((u8*)compressed)[i] + 1;
					u8 v = ((u8*)compressed)[i + 1];
					for(int j = 0; j < r; ++j) {
						outCur[c] = v;
						outCur += info.channelCount;
					}
				}
			}
		} else {
			Error("Unknown image compression");
		}
		outData.image = m_ImageData;
	}

	void WriteV1(FontRaster* font)
	{
		// Write font info
		FontInfoV1 info;
		info.antialiased = font->GetDescription().antialiased;
		info.baseLine = font->GetBaseLine();
		info.charCount = font->GetCharMap().Size();

		info.charHeight = font->GetFontHeight();
		info.fontSize = font->GetDescription().size;
		info.borderSize = font->GetDescription().borderSize;
#ifdef FONT_NO_COMPRESS
		info.imageCompression = EImageCompressionV1::None;
#else
		info.imageCompression = EImageCompressionV1::RLE;
#endif
		const void* imgPtr;
		int w, h, channels;
		font->GetRawData(imgPtr, w, h, channels);
		info.imageHeight = h;
		info.imageWidth = w;
		info.channelCount = channels;
		info.italic = font->GetDescription().italic;
		info.name = font->GetDescription().name;
		info.weight = ConvertWeightToV1(font->GetDescription().weight);

		info.scale = font->GetBaseFontSettings().scale;
		info.charDistance = font->GetBaseFontSettings().charDistance;
		info.wordDistance = font->GetBaseFontSettings().wordDistance;
		info.lineDistance = font->GetBaseFontSettings().lineDistance;
		info.slanting = font->GetBaseFontSettings().slanting;

		WriteV1Info(info);

		// Write char info
		auto& charMap = font->GetCharMap();
		for(auto it = charMap.begin(); it != charMap.end(); ++it) {
			auto& v = it->value;
			CharInfoV1 c;
			c.character = it->key;
			c.A = v.A;
			c.B = v.B;
			c.C = v.C;
			c.left = v.left;
			c.top = v.top;

			WriteCharInfo(c);
		}

		// Write image
#ifdef FONT_NO_COMPRESS
		const u8* cur = (const u8*)imgPtr;
		auto lineSize = info.imageWidth * info.channelCount;
		for(u32 i = 0; i < info.imageHeight; ++i) {
			WriteBytes(lineSize, cur);
			cur += lineSize;
		}
#else
		// Write rle
		core::Array<u8> outData;
		for(int c = 0; c < info.channelCount; ++c) {
			outData.Clear();
			const u8* cur = (const u8*)imgPtr;
			auto pitch = info.imageWidth * info.channelCount;

			int runLen = 0;
			u8 lastByte = 0;
			for(int i = 0; i < info.imageHeight; ++i) {
				for(int j = 0; j < info.imageWidth; ++j) {
					u8 byte = cur[info.channelCount*j + c];
					if(i == 0 && j == 0)
						lastByte = byte;

					if((i == info.imageHeight - 1 && j == info.imageWidth - 1) || byte != lastByte || runLen == 256) {
						outData.PushBack((u8)(runLen - 1));
						outData.PushBack(lastByte);
						runLen = 0;
					}
					lastByte = byte;
					++runLen;
				}
				cur += pitch;
			}
			WriteU32(outData.Size());
			WriteBytes(outData.Size(), outData.Data_c());
		}
#endif
	}

private:
	void ReadV1Info(FontInfoV1& out)
	{
		u32 nameLength = ReadU32();
		if(nameLength > 512)
			Error("Font name is to long");
		Preload(nameLength + FontInfoV1SizeWithoutName);
		core::Array<u8> rawData;
		if(nameLength > 0) {
			rawData.Resize(nameLength);
			ReadBytes(nameLength, rawData.Data());
			out.name.Append((char*)rawData.Data_c(), rawData.Size());
		} else {
			out.name.Clear();
		}

		out.fontSize = ReadU32();
		out.borderSize = ReadU32();
		out.weight = (EFontWeightsV1)ReadU32();
		u32 flags = ReadU32();
		out.italic = (flags & FontFlagV1_Italic) != 0;
		out.antialiased = (flags & FontFlagV1_Antialiased) != 0;
		out.charCount = ReadU32();
		out.channelCount = ReadU32();
		out.imageWidth = ReadU32();
		out.imageHeight = ReadU32();
		out.imageCompression = (EImageCompressionV1)ReadU32();

		out.charHeight = ReadFraction();
		out.scale = ReadFraction();
		out.charDistance = ReadFraction();
		out.wordDistance = ReadFraction();
		out.lineDistance = ReadFraction();
		out.baseLine = ReadFraction();
		out.slanting = ReadFraction();
	}

	void WriteV1Info(const FontInfoV1& info)
	{
		if(info.name.Size() > 512)
			Error("Font name is to long");
		WriteU32((u32)info.name.Size());
		if(!info.name.IsEmpty())
			WriteBytes(info.name.Size(), info.name.Data());
		WriteU32((u32)info.fontSize);
		WriteU32((u32)info.borderSize);
		WriteU32((u32)info.weight);
		u32 flags = 0;
		if(info.italic)
			flags |= FontFlagV1_Italic;
		if(info.antialiased)
			flags |= FontFlagV1_Antialiased;
		WriteU32(flags);
		WriteU32((u32)info.charCount);
		WriteU32((u32)info.channelCount);
		WriteU32((u32)info.imageWidth);
		WriteU32((u32)info.imageHeight);
		WriteU32((u32)info.imageCompression);

		WriteFraction(info.charHeight);
		WriteFraction(info.scale);
		WriteFraction(info.charDistance);
		WriteFraction(info.wordDistance);
		WriteFraction(info.lineDistance);
		WriteFraction(info.baseLine);
		WriteFraction(info.slanting);
	}

	[[noreturn]] void Error(core::StringView message = "format is invalid")
	{
		throw core::FileFormatException(message, "lxf");
	}

	void Preload(u32 bytes)
	{
		lxAssert(m_BufferCursor == m_BufferSize);

		m_Buffer.SetMinSize(bytes);
		m_File->ReadBinary(bytes, m_Buffer);

		m_BufferSize = bytes;
		m_BufferCursor = 0;
	}

	u32 ReadU32()
	{
		u32 v;
		ReadBytes(4, &v);
		return v;
	}

	float ReadFraction()
	{
		s32 i;
		ReadBytes(4, &i);
		return i / 10000.0f;
	}

	void ReadCharInfo(CharInfoV1& out)
	{
		out.character = ReadU32();
		out.left = ReadFraction();
		out.top = ReadFraction();

		out.A = ReadFraction();
		out.B = ReadFraction();
		out.C = ReadFraction();
	}

	void WriteCharInfo(const CharInfoV1& info)
	{
		WriteU32(info.character);
		WriteFraction(info.left);
		WriteFraction(info.top);

		WriteFraction(info.A);
		WriteFraction(info.B);
		WriteFraction(info.C);
	}
	void ReadBytes(u32 count, void* dst)
	{
		if(m_BufferSize == m_BufferCursor) {
			m_File->ReadBinary(count, dst);
		} else {
			lxAssert(m_BufferCursor + count <= m_BufferSize);
			memcpy(dst, (u8*)m_Buffer + m_BufferCursor, count);
			m_BufferCursor += count;
		}
	}

	void WriteU32(u32 v)
	{
		WriteBytes(4, &v);
	}

	void WriteFraction(float f)
	{
		s32 value = (s32)(f*10000.0f);
		WriteBytes(4, &value);
	}

	void WriteBytes(int count, const void* data)
	{
		m_File->WriteBinary(data, count);
	}

	EFontWeight ConvertV1Weight(EFontWeightsV1 v1)
	{
		switch(v1) {
		case EFontWeightsV1::Bolt: return EFontWeight::Bolt;
		case EFontWeightsV1::ExtraBolt: return EFontWeight::ExtraBolt;
		case EFontWeightsV1::ExtraThin: return EFontWeight::ExtraThin;
		case EFontWeightsV1::Normal: return EFontWeight::Normal;
		case EFontWeightsV1::Thin: return EFontWeight::Thin;
		default: Error("Unknown font weight.");
		}
	}

	EFontWeightsV1 ConvertWeightToV1(EFontWeight w)
	{
		switch(w) {
		case EFontWeight::Bolt: return EFontWeightsV1::Bolt;
		case EFontWeight::ExtraBolt: return EFontWeightsV1::ExtraBolt;
		case EFontWeight::ExtraThin: return EFontWeightsV1::ExtraThin;
		case EFontWeight::Normal: return EFontWeightsV1::Normal;
		case EFontWeight::Thin: return EFontWeightsV1::Thin;
		default: Error("Unknown font weight.");
		}
	}
private:
	io::File* m_File;
	core::RawMemory m_ImageData;
	core::RawMemory m_Buffer;
	u32 m_BufferSize;
	u32 m_BufferCursor;
	u32 m_Version;
};

}

///////////////////////////////////////////////////////////////////////////////

core::Name FontLoader::GetResourceType(io::File* file, core::Name requestedType)
{
	if(!requestedType.IsEmpty() && requestedType != core::ResourceType::Font)
		return core::Name::INVALID;

	u32 magic = 0;
	auto bytes = file->ReadBinaryPart(sizeof(u32), &magic);
	if(bytes != 4 || magic != LX_MAKE_FOURCC('F', 'O', 'N', 'T'))
		return core::Name::INVALID;
	else
		return core::ResourceType::Font;
}

void FontLoader::LoadResource(io::File* file, core::Resource* dst)
{
	auto font = dynamic_cast<gui::FontRaster*>(dst);
	if(!font)
		throw core::InvalidOperationException("Passed wrong type to loader");

	Context ctx(file);
	u32 version = ctx.ReadHeader();
	gui::FontCreationData createData;
	if(version == 1)
		ctx.ReadV1(createData);
	else
		throw core::FileFormatException("Font file version is not supported", "lxf");

	font->Init(createData);
}

const core::String& FontLoader::GetName() const
{
	static const core::String name = "Lux Font Loader";
	return name;
}

///////////////////////////////////////////////////////////////////////////////

bool FontWriter::CanWriteType(const core::String& ext, core::Name requestedType)
{
	if(requestedType != core::ResourceType::Font)
		return false;

	return ext.IsEmpty() || ext.Equal("lxf", core::EStringCompare::CaseInsensitive) || ext.Equal("font", core::EStringCompare::CaseInsensitive);
}

void FontWriter::WriteResource(io::File* file, core::Resource* resource)
{
	auto font = dynamic_cast<gui::FontRaster*>(resource);
	if(!font)
		throw core::InvalidOperationException("Passed wrong type to loader");

	Context ctx(file);
	ctx.WriteHeader("0001");
	ctx.WriteV1(font);
}

const core::String& FontWriter::GetName() const
{
	static const core::String name = "Lux Font Writer";
	return name;
}

///////////////////////////////////////////////////////////////////////////////

}
}
