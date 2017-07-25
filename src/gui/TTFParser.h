#ifndef INCLUDED_TTF_PARSER_H
#define INCLUDED_TTF_PARSER_H
#include "core/LuxBase.h"
#include "core/lxString.h"
#include "core/lxUnicodeConversion.h"

namespace lux
{

class TTFParser
{
public:
	TTFParser(void* data, size_t size)
	{
		m_Data = (u8*)data;
		m_Cursor = m_Data;
		m_End = m_Data + size;
		m_IsValid = true;

		bool result = true;
		u32 nameSize = 0, nameOffset = 0;
		if(result)
			result = FindNameTable(nameOffset, nameSize);
		if(result)
			result = ReadFontFamily(nameOffset, nameSize);

		m_IsValid = result;
	}

	bool FindNameTable(u32& outOffset, u32& outSize)
	{
		u32 version = ReadFixed();
		if(version != 0x00010000)
			return false;

		u32 numTables = ReadUShort();
		// u32 searchRange = ReadUShort();
		// u32 entrySelector = ReadUShort();
		// u32 rangeShift = ReadUShort();
		Seek(3 * 2);
		if(!m_IsValid)
			return false;

		for(u32 i = 0; i < numTables; ++i) {
			u32 tag = ReadULong();
			u32 checkSum = ReadULong();
			u32 offset = ReadULong();
			u32 length = ReadULong();

			LUX_UNUSED(checkSum);

			if(tag == 0x6E616D65) { // name-table
				outOffset = offset;
				outSize = length;
				return true;
			}
		}

		return false;
	}

	bool ReadFontFamily(u32 nameOffset, u32 nameSize)
	{
		LUX_UNUSED(nameSize);

		SetCursor(nameOffset);

		u32 format = ReadUShort();
		u32 count = ReadUShort();
		u32 data_off = ReadUShort();

		if(format != 0)
			return false;

		if(!m_IsValid)
			return false;

		for(u32 i = 0; i < count; ++i) {
			u32 platformId = ReadUShort();
			u32 encId = ReadUShort();
			u32 langId = ReadUShort();
			u32 nameId = ReadUShort();
			u32 len = ReadUShort();
			u32 off = ReadUShort();

			// Ref: https://www.microsoft.com/typography/otspec/name.htm
			LUX_UNUSED(langId);
			// TODO: Allow diffrent plaforms
			// TODO: Names could be localised
			// TODO: Are other formats than unicode possible.
			if(nameId == 1 && (platformId == 0 || (platformId == 3 && encId == 1))) { // Font-Family
				if(!m_IsValid)
					return false;
				SetCursor(nameOffset + data_off + off);

				core::Array<u16> utf16Buffer;
				utf16Buffer.Reserve(len / 2);
				for(u32 j = 0; j < len / 2; ++j)
					utf16Buffer.PushBack((u16)ReadUShort());
				utf16Buffer.PushBack(0);
				m_FontFamily = core::UTF16ToString(utf16Buffer.Data());

				return m_IsValid;
			}
		}

		return false;
	}

	const String& GetFontFamily() const
	{
		return m_FontFamily;
	}

	bool IsValid() const
	{
		return m_IsValid;
	}

	void ReadBinary(u32 count, void* dst)
	{
		u8* cur = (u8*)dst;
		for(u32 i = 0; i < count; ++i) {
			*cur = (u8)ReadByte();
			++cur;
		}
	}

	u32 ReadByte()
	{
		if(!m_IsValid)
			return 0;

		if(m_Cursor != m_End) {
			u32 out = *m_Cursor;
			m_Cursor++;
			return out;
		} else {
			m_IsValid = false;
			return 0;
		}
	}

	u32 ReadUShort()
	{
		if(!m_IsValid)
			return 0;

		u32 b1 = ReadByte();
		u32 b2 = ReadByte();
		return b1 << 8 | b2;
	}

	u32 ReadULong()
	{
		if(!m_IsValid)
			return 0;

		u32 b1 = ReadByte();
		u32 b2 = ReadByte();
		u32 b3 = ReadByte();
		u32 b4 = ReadByte();
		return b1 << 24 | b2 << 16 | b3 << 8 | b4;
	}

	u32 ReadFixed()
	{
		return ReadULong();
	}

	s32 ReadLong()
	{
		u32 data = ReadULong();
		s32 out;
		memcpy(&out, &data, 4);
		return out;
	}

	u32 GetCursor()
	{
		return (u32)(m_Cursor - m_Data);
	}

	void SetCursor(u32 byte)
	{
		if(m_Data + byte >= m_End) {
			m_Cursor = m_End;
			m_IsValid = false;
		} else {
			m_Cursor = m_Data + byte;
		}
	}

	void Seek(u32 byte)
	{
		if(m_Cursor + byte >= m_End) {
			m_Cursor = m_End;
			m_IsValid = false;
		} else {
			m_Cursor += byte;
		}
	}

private:
	String m_FontFamily;

	bool m_IsValid;

	u8* m_Data;
	u8* m_Cursor;
	u8* m_End;
};

}

#endif // #ifndef INCLUDED_TTF_PARSER_H