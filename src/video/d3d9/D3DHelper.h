#ifndef INCLUDED_LUX_D3DHELPER_H
#define INCLUDED_LUX_D3DHELPER_H

#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/Color.h"
#include "video/VertexFormat.h"
#include "video/VideoEnums.h"
#include "video/TextureLayer.h"
#include "video/FogData.h"
#include "video/Pass.h"
#include "video/CubeTexture.h"

#include "platform/StrippedD3D9.h"

namespace lux
{
namespace video
{

inline D3DFORMAT GetD3DFormat(ZStencilFormat format)
{
	if(format.zBits == 32) {
		if(format.sBits == 0 && format.stride == 32)
			return D3DFMT_D32;
	} else if(format.zBits == 24) {
		if(format.sBits == 0 && format.stride == 32)
			return D3DFMT_D24X8;
		if(format.sBits == 4 && format.stride == 32)
			return D3DFMT_D24X4S4;
		if(format.sBits == 8 && format.stride == 32)
			return D3DFMT_D24S8;
	} else if(format.zBits == 16) {
		if(format.sBits == 0 && format.stride == 16)
			return D3DFMT_D16;
	} else if(format.zBits == 15) {
		if(format.sBits == 1 && format.stride == 16)
			return D3DFMT_D15S1;
	}

	return D3DFMT_UNKNOWN;
}

inline ZStencilFormat GetZStencil(D3DFORMAT format)
{
	switch(format) {
	case D3DFMT_D32: return ZStencilFormat(32, 0, 32);
	case D3DFMT_D24X8: return ZStencilFormat(24, 0, 32);
	case D3DFMT_D24X4S4: return ZStencilFormat(24, 4, 32);
	case D3DFMT_D24S8: return ZStencilFormat(24, 8, 32);
	case D3DFMT_D16: return ZStencilFormat(16, 0, 16);
	case D3DFMT_D15S1: return ZStencilFormat(15, 1, 16);
	default: return ZStencilFormat(0, 0, 0);
	}
}

inline D3DFORMAT GetD3DFormat(ColorFormat Format)
{
	switch(Format.ToEnum()) {
	case ColorFormat::R8G8B8:
		return D3DFMT_R8G8B8;
	case ColorFormat::X8R8G8B8:
		return D3DFMT_X8R8G8B8;
	case ColorFormat::A8R8G8B8:
		return D3DFMT_A8R8G8B8;
	case ColorFormat::G16R16:
		return D3DFMT_G16R16;
	case ColorFormat::X1R5G5B5:
		return D3DFMT_X1R5G5B5;
	case ColorFormat::A1R5G5B5:
		return D3DFMT_A1R5G5B5;
	case ColorFormat::R5G6B5:
		return D3DFMT_R5G6B5;
	case ColorFormat::A2R10G10B10:
		return D3DFMT_A2R10G10B10;

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

	case ColorFormat::DXT1:
		return D3DFMT_DXT1;
	case ColorFormat::DXT3:
		return D3DFMT_DXT3;
	case ColorFormat::DXT5:
		return D3DFMT_DXT5;

	case ColorFormat::UNKNOWN:
	default:
		return D3DFMT_UNKNOWN;
	};
}

inline BYTE GetD3DUsage(VertexElement::EUsage usage)
{
	lxAssert(usage != VertexElement::EUsage::Unknown);

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
	default: return 0;
	}
}

inline DWORD GetD3DBlendFunc(EBlendOperator op)
{
	lxAssert(op != EBlendOperator::None);

	switch(op) {
	case EBlendOperator::Add: return D3DBLENDOP_ADD;
	case EBlendOperator::Max: return D3DBLENDOP_MAX;
	case EBlendOperator::Min: return D3DBLENDOP_MIN;
	case EBlendOperator::Subtract: return D3DBLENDOP_SUBTRACT;
	case EBlendOperator::RevSubtract: return D3DBLENDOP_REVSUBTRACT;
	default: return 0;
	}
}

inline ColorFormat GetLuxFormat(D3DFORMAT Format)
{
	switch(Format) {
	case D3DFMT_R8G8B8:
		return ColorFormat::R8G8B8;
	case D3DFMT_X8R8G8B8:
		return ColorFormat::X8R8G8B8;
	case D3DFMT_A8R8G8B8:
		return ColorFormat::A8R8G8B8;
	case D3DFMT_X1R5G5B5:
		return ColorFormat::X1R5G5B5;
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

inline int GetBitsPerPixel(D3DFORMAT Format)
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
	case ETextureRepeat::Wrap:
		return D3DTADDRESS_WRAP;
	case ETextureRepeat::Mirror:
		return D3DTADDRESS_MIRROR;
	case ETextureRepeat::Clamp:
		return D3DTADDRESS_CLAMP;
	case ETextureRepeat::MirrorOnce:
		return D3DTADDRESS_MIRRORONCE;
	case ETextureRepeat::Border:
		return D3DTADDRESS_BORDER;
	}

	return 0;
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

inline D3DCOLORVALUE SColorToD3DColor(const ColorF& color)
{
	D3DCOLORVALUE out = {
		color.r,
		color.g,
		color.b,
		color.a};

	return out;
}

inline DWORD GetD3DTextureFilter(BaseTexture::Filter::EFilter filter)
{
	switch(filter) {
	case BaseTexture::Filter::Linear:
		return D3DTEXF_LINEAR;
	case BaseTexture::Filter::Point:
		return D3DTEXF_POINT;
	case BaseTexture::Filter::Anisotropic:
		return D3DTEXF_ANISOTROPIC;
	default: throw core::GenericInvalidArgumentException("filter", "Unknown filter");
	};
}

inline DWORD GetD3DStencilOperator(EStencilOperator op)
{
	switch(op) {
	case EStencilOperator::Keep: return D3DSTENCILOP_KEEP;
	case EStencilOperator::Zero: return D3DSTENCILOP_ZERO;
	case EStencilOperator::Replace: return D3DSTENCILOP_REPLACE;
	case EStencilOperator::Invert: return D3DSTENCILOP_INVERT;
	case EStencilOperator::Increment: return D3DSTENCILOP_INCR;
	case EStencilOperator::Decrement:  return D3DSTENCILOP_DECR;
	case EStencilOperator::IncrementSat: return D3DSTENCILOP_INCRSAT;
	case EStencilOperator::DecrementSat: return D3DSTENCILOP_DECRSAT;
	default:
		throw core::GenericInvalidArgumentException("op", "Unknown operand");
	}
}

inline core::StringView GetD3DXShaderProfile(
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
				return "vs_3_0";
		} else if(major == 4) {
			if(minor == 0)
				return "vs_4_0";
			if(minor == 1)
				return "vs_4_1";
		} else if(major == 5) {
			if(minor == 0)
				return "vs_5_0";
		}
	}

	return core::StringView::EMPTY;
}

inline DWORD GetD3DBlend(EBlendFactor factor)
{
	switch(factor) {
	case EBlendFactor::Zero:
		return D3DBLEND_ZERO;
	case EBlendFactor::One:
		return D3DBLEND_ONE;
	case EBlendFactor::SrcAlpha:
		return D3DBLEND_SRCALPHA;
	case EBlendFactor::OneMinusSrcAlpha:
		return D3DBLEND_INVSRCALPHA;
	case EBlendFactor::DstAlpha:
		return D3DBLEND_DESTALPHA;
	case EBlendFactor::OneMinusDstAlpha:
		return D3DBLEND_INVDESTALPHA;
	}
	throw core::GenericInvalidArgumentException("factor", "Unknown factor");
}

inline DWORD GetD3DComparisonFunc(EComparisonFunc func)
{
	switch(func) {
	case EComparisonFunc::Always:
		return D3DCMP_ALWAYS;
	case EComparisonFunc::Equal:
		return D3DCMP_EQUAL;
	case EComparisonFunc::Greater:
		return D3DCMP_GREATER;
	case EComparisonFunc::GreaterEqual:
		return D3DCMP_GREATEREQUAL;
	case EComparisonFunc::Less:
		return D3DCMP_LESS;
	case EComparisonFunc::LessEqual:
		return D3DCMP_LESSEQUAL;
	case EComparisonFunc::Never:
		return D3DCMP_NEVER;
	case EComparisonFunc::NotEqual:
		return D3DCMP_NOTEQUAL;
	}
	throw core::GenericInvalidArgumentException("func", "Unknown function");
}

inline D3DFORMAT GetD3DIndexFormat(EIndexFormat indexFormat)
{
	switch(indexFormat) {
	case EIndexFormat::Bit16: return D3DFMT_INDEX16;
	case EIndexFormat::Bit32: return D3DFMT_INDEX32;
	}
	throw core::GenericInvalidArgumentException("indexFormat", "Unknown indexformat");
}

inline D3DPRIMITIVETYPE GetD3DPrimitiveType(EPrimitiveType type)
{
	switch(type) {
	case EPrimitiveType::Lines:
		return D3DPT_LINELIST;
	case EPrimitiveType::LineStrip:
		return D3DPT_LINESTRIP;
	case EPrimitiveType::Points:
		return D3DPT_POINTLIST;
	case EPrimitiveType::TriangleFan:
		return D3DPT_TRIANGLEFAN;
	case EPrimitiveType::Triangles:
		return D3DPT_TRIANGLELIST;
	case EPrimitiveType::TriangleStrip:
		return D3DPT_TRIANGLESTRIP;
	}
	throw core::GenericInvalidArgumentException("type", "Unknown primitive type");
}

inline DWORD GetD3DFogType(EFogType type)
{
	switch(type) {
	case EFogType::Exp: return D3DFOG_EXP;
	case EFogType::ExpSq: return D3DFOG_EXP2;
	case EFogType::Linear: return D3DFOG_LINEAR;
	}
	throw core::GenericInvalidArgumentException("type", "Unknown fogtype");
}

inline D3DCUBEMAP_FACES GetD3DCubeMapFace(CubeTexture::EFace face)
{
	static const D3DCUBEMAP_FACES CONV[6] = {D3DCUBEMAP_FACE_POSITIVE_X, D3DCUBEMAP_FACE_NEGATIVE_X,
		D3DCUBEMAP_FACE_POSITIVE_Y, D3DCUBEMAP_FACE_NEGATIVE_Y,
		D3DCUBEMAP_FACE_POSITIVE_Z, D3DCUBEMAP_FACE_NEGATIVE_Z};
	int i = (int)face;
	if(i < 0 || i >= 6)
		throw core::GenericInvalidArgumentException("face", "Unknown cube map face");
	else
		return CONV[i];
}
} // namespace video
} // namesapce lux

#endif // LUX_COMPILE_WITH_D3D9

#endif // #ifndef INCLUDED_LUX_D3DHELPER_H