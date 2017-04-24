#ifndef INCLUDED_D3DHELPER_H
#define INCLUDED_D3DHELPER_H

#ifdef LUX_COMPILE_WITH_D3D9

#include "video/Color.h"
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

}
}

#endif // LUX_COMPILE_WITH_D3D9

#endif // #ifndef INCLUDED_D3DHELPER_H