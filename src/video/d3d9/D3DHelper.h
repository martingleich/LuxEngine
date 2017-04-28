#ifndef INCLUDED_D3DHELPER_H
#define INCLUDED_D3DHELPER_H

#ifdef LUX_COMPILE_WITH_D3D9

#include "video/Color.h"
#include "video/VertexFormats.h"
#include "video/Material.h"
#include "StrippedD3D9.h"

namespace lux
{
namespace video
{

inline D3DFORMAT GetD3DFormat(ColorFormat Format, bool Alpha)
{
	switch((u32)Format) {
	case ColorFormat::R8G8B8:
		return D3DFMT_R8G8B8;
	case ColorFormat::A8R8G8B8:
		return Alpha ? D3DFMT_A8R8G8B8 : D3DFMT_X8B8G8R8;
	case ColorFormat::A1R5G5B5:
		return Alpha ? D3DFMT_A1R5G5B5 : D3DFMT_X1R5G5B5;
	case ColorFormat::R5G6B5:
		return D3DFMT_R5G6B5;

	case ColorFormat::X8:
		return D3DFMT_L8;
	case ColorFormat::X16:
		return D3DFMT_L16;

		// Floating Point Formats
	case ColorFormat::R16F:
		return D3DFMT_R16F;
	case ColorFormat::G16R16F:
		return D3DFMT_G16R16F;
	case ColorFormat::A16B16G16R16F:
		return D3DFMT_A16B16G16R16F;
	case ColorFormat::R32F:
		return D3DFMT_R32F;
	case ColorFormat::G32R32F:
		return D3DFMT_G32R32F;
	case ColorFormat::A32B32G32R32F:
		return D3DFMT_A32B32G32R32F;

	case ColorFormat::UNKNOWN:
	default:
		return D3DFMT_UNKNOWN;
	};
}

inline BYTE GetD3DUsage(VertexElement::EUsage usage)
{
	switch(usage) {
	case VertexElement::EUsage::Position:      return D3DDECLUSAGE_POSITION;
	case VertexElement::EUsage::PositionNT:    return D3DDECLUSAGE_POSITIONT;
	case VertexElement::EUsage::Normal:        return D3DDECLUSAGE_NORMAL;
	case VertexElement::EUsage::Tangent:       return D3DDECLUSAGE_TANGENT;
	case VertexElement::EUsage::Binormal:      return D3DDECLUSAGE_BINORMAL;
	case VertexElement::EUsage::Texcoord0:
	case VertexElement::EUsage::Texcoord1:
	case VertexElement::EUsage::Texcoord2:
	case VertexElement::EUsage::Texcoord3:
		return D3DDECLUSAGE_TEXCOORD;
	case VertexElement::EUsage::Diffuse:
	case VertexElement::EUsage::Specular:
		return D3DDECLUSAGE_COLOR;
	case VertexElement::EUsage::BlendWeight:  return D3DDECLUSAGE_BLENDWEIGHT;
	case VertexElement::EUsage::BlendIndices: return D3DDECLUSAGE_BLENDINDICES;
	case VertexElement::EUsage::Sample:       return D3DDECLUSAGE_SAMPLE;
	default:
		return 0xFF;
	}
}

inline DWORD GetD3DBlendFunc(EBlendOperator Op)
{
	switch(Op) {
	case EBO_ADD: return D3DBLENDOP_ADD;
	case EBO_MAX: return D3DBLENDOP_MAX;
	case EBO_MIN: return D3DBLENDOP_MIN;
	case EBO_REVSUBTRACT: return D3DBLENDOP_REVSUBTRACT;
	case EBO_SUBTRACT: return D3DBLENDOP_SUBTRACT;
	default: return 0;
	}
}

inline ColorFormat GetLuxFormat(D3DFORMAT Format)
{
	switch(Format) {
	case D3DFMT_R8G8B8:
		return ColorFormat::R8G8B8;
	case D3DFMT_A8R8G8B8:
		return ColorFormat::A8R8G8B8;
	case D3DFMT_A1R5G5B5:
		return ColorFormat::A1R5G5B5;
	case D3DFMT_R5G6B5:
		return ColorFormat::R5G6B5;
	case D3DFMT_L8:
		return ColorFormat::X8;
	case D3DFMT_L16:
		return ColorFormat::X16;
	case D3DFMT_R16F:
		return ColorFormat::R16F;
	case D3DFMT_G16R16F:
		return ColorFormat::G16R16F;
	case D3DFMT_A16B16G16R16F:
		return ColorFormat::A16B16G16R16F;
	case D3DFMT_R32F:
		return ColorFormat::R32F;
	case D3DFMT_G32R32F:
		return ColorFormat::G32R32F;
	case D3DFMT_A32B32G32R32F:
		return ColorFormat::A32B32G32R32F;
	default:
		return ColorFormat::UNKNOWN;
	}
}

inline u32 GetBitsPerPixel(D3DFORMAT Format)
{
	// Format suchen
	switch(Format) {
	case D3DFMT_R8G8B8: return 24;
	case D3DFMT_A8R8G8B8: return 32;
	case D3DFMT_X8R8G8B8: return 32;
	case D3DFMT_R5G6B5: return 16;
	case D3DFMT_X1R5G5B5: return 16;
	case D3DFMT_A1R5G5B5: return 16;
	case D3DFMT_A4R4G4B4: return 16;
	case D3DFMT_A2B10G10R10: return 32;
	case D3DFMT_A16B16G16R16: return 64;
	case D3DFMT_G16R16: return 32;
	case D3DFMT_A8P8: return 16;
	case D3DFMT_P8: return 8;
	case D3DFMT_L8: return 8;
	case D3DFMT_A8L8: return 16;
	case D3DFMT_A4L4: return 8;

	case D3DFMT_V8U8: return 16;
	case D3DFMT_Q8W8V8U8: return 32;
	case D3DFMT_V16U16: return 32;
	case D3DFMT_Q16W16V16U16: return 64;

	case D3DFMT_L6V5U5: return 16;
	case D3DFMT_X8L8V8U8: return 32;
	case D3DFMT_A2W10V10U10: return 32;
	case D3DFMT_L16: return 16;

	case D3DFMT_D16_LOCKABLE: return 16;
	case D3DFMT_D32: return 32;
	case D3DFMT_D32F_LOCKABLE: return 32;
	case D3DFMT_D24FS8: return 32;
	case D3DFMT_D15S1: return 16;
	case D3DFMT_D24S8: return 32;
	case D3DFMT_D16: return 16;
	case D3DFMT_D24X8: return 32;
	case D3DFMT_D24X4S4: return 32;
	default: return 0;
	}
}

inline DWORD GetD3DRepeatMode(ETextureRepeat repeat)
{
	switch(repeat) {
	case ETR_WRAP:
		return D3DTADDRESS_WRAP;
	case ETR_MIRROR:
		return D3DTADDRESS_MIRROR;
	case ETR_CLAMP:
		return D3DTADDRESS_CLAMP;
	case ETR_MIRRORONCE:
		return D3DTADDRESS_MIRRORONCE;
	default:
		return D3DTADDRESS_FORCE_DWORD;
	}
}

inline DWORD GetD3DDeclType(VertexElement::EType type)
{
	switch(type) {
	case VertexElement::EType::Float1: return D3DDECLTYPE_FLOAT1;
	case VertexElement::EType::Float2: return D3DDECLTYPE_FLOAT2;
	case VertexElement::EType::Float3: return D3DDECLTYPE_FLOAT3;
	case VertexElement::EType::Float4: return D3DDECLTYPE_FLOAT4;
	case VertexElement::EType::Color: return D3DDECLTYPE_D3DCOLOR;
	case VertexElement::EType::Byte4: return D3DDECLTYPE_UBYTE4;
	case VertexElement::EType::Short2: return D3DDECLTYPE_SHORT2;
	case VertexElement::EType::Short4: return D3DDECLTYPE_SHORT4;
	default: return D3DDECLTYPE_UNUSED;
	}
}

inline D3DCOLORVALUE SColorToD3DColor(const Colorf& color)
{
	D3DCOLORVALUE out = {
		color.r,
		color.g,
		color.b,
		color.a};

	return out;
}

inline DWORD GetD3DTextureFilter(ETextureFilter filter)
{
	if(filter == ETF_LINEAR)
		return D3DTEXF_LINEAR;
	if(filter == ETF_POINT)
		return D3DTEXF_POINT;
	if(filter == ETF_ANISOTROPIC)
		return D3DTEXF_ANISOTROPIC;
	return D3DTEXF_POINT;
}

inline const char* GetD3DXShaderProfile(
	bool isPixel,
	int major, int minor)
{
	if(isPixel) {
		if(major == 1) {
			if(minor == 1)
				return "ps_1_1";
			if(minor == 2)
				return "ps_1_2";
			if(minor == 3)
				return "ps_1_3";
			if(minor == 4)
				return "ps_1_4";
		} else if(major == 2) {
			if(minor == 0)
				return "ps_2_0";
			if(minor == 1)
				return "ps_2_a";
			if(minor == 2)
				return "ps_2_b";
		} else if(major == 3) {
			if(minor == 0)
				return "ps_3_0";
		} else if(major == 4) {
			if(minor == 0)
				return "ps_4_0";
			if(minor == 1)
				return "ps_4_1";
		} else if(major == 5) {
			if(minor == 0)
				return "ps_5_0";
		}
	} else {
		if(major == 1) {
			if(minor == 1)
				return "vs_1_1";
		} else if(major == 2) {
			if(minor == 0)
				return "vs_2_0";
			if(minor == 1)
				return "vs_2_a";
		} else if(major == 3) {
			if(minor == 0)
				return "ps_3_0";
		} else if(major == 4) {
			if(minor == 0)
				return "ps_4_0";
			if(minor == 1)
				return "ps_4_1";
		} else if(major == 5) {
			if(minor == 0)
				return "ps_5_0";
		}
	}

	return nullptr;
}

inline DWORD GetD3DBlend(EBlendFactor factor)
{
	switch(factor) {
	case EBF_ZERO:
		return D3DBLEND_ZERO;
	case EBF_ONE:
		return D3DBLEND_ONE;
	case EBF_SRC_ALPHA:
		return D3DBLEND_SRCALPHA;
	case EBF_ONE_MINUS_SRC_ALPHA:
		return D3DBLEND_INVSRCALPHA;
	case EBF_DST_ALPHA:
		return D3DBLEND_DESTALPHA;
	case EBF_ONE_MINUS_DST_ALPHA:
		return D3DBLEND_INVDESTALPHA;
	default:
		return 0;
	}
}
}
}

#endif // LUX_COMPILE_WITH_D3D9

#endif // #ifndef INCLUDED_D3DHELPER_H