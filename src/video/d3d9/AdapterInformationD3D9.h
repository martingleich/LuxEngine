#ifndef INCLUDED_ADAPTER_INFORMATION_D3D9_H
#define INCLUDED_ADAPTER_INFORMATION_D3D9_H
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/DriverConfig.h"

#include "UnknownRefCounted.h"
#include "StrippedD3D9.h"

namespace lux
{
namespace video
{
/**
Only things with full hardware support are listed.
*/
class AdapterD3D9 : public Adapter
{
public:
	AdapterD3D9(UnknownRefCounted<IDirect3D9> d3d9, UINT adapter);
	const String& GetName() const;
	u32 GetVendor() const;
	u32 GetDevice() const;
	EDriverType GetDriverType() const;
	UINT GetAdapter() const;
	UnknownRefCounted<IDirect3D9> GetD3D9() const;
	core::Array<DisplayMode> GenerateDisplayModes(bool windowed);
	core::Array<ColorFormat> GenerateBackbufferFormats(const DisplayMode& mode, bool windowed);
	core::Array<ZStencilFormat> GenerateZStencilFormats(const DisplayMode& mode, bool windowed, ColorFormat backBuffer);
	core::Array<u32> GenerateMultisampleLevels(const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat);
	u32 GetNumMultisampleQualities(const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat, u32 level);

private:
	UnknownRefCounted<IDirect3D9> m_D3D9;

	UINT m_Adapter;

	String m_Name;
	u32 m_Vendor;
	u32 m_Device;
};

///////////////////////////////////////////////////////////////////////////////

class AdapterListD3D9 : public AdapterList
{
public:
	AdapterListD3D9();
	u32 GetAdapterCount() const;
	StrongRef<Adapter> GetAdapter(u32 idx) const;

private:
	UnknownRefCounted<IDirect3D9> m_D3D9;
	core::Array<StrongRef<AdapterD3D9>> m_Adapters;
};

} // namespace video
} // namespace lux

#endif
#endif // #ifndef INCLUDED_ADAPTER_INFORMATION_D3D9_H