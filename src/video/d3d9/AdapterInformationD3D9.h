#ifndef INCLUDED_LUX_ADAPTER_INFORMATION_D3D9_H
#define INCLUDED_LUX_ADAPTER_INFORMATION_D3D9_H
#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/DriverConfig.h"

#include "platform/UnknownRefCounted.h"
#include "platform/StrippedD3D9.h"

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
	const core::String& GetName() const override;
	u32 GetVendor() const override;
	u32 GetDevice() const override;
	core::Name GetDriverType() const override;
	core::Array<DisplayMode> GenerateDisplayModes(bool windowed) override;
	core::Array<ColorFormat> GenerateBackbufferFormats(const DisplayMode& mode, bool windowed) override;
	core::Array<ZStencilFormat> GenerateZStencilFormats(const DisplayMode& mode, bool windowed, ColorFormat backBuffer) override;
	core::Array<int> GenerateMultisampleLevels(const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat) override;
	int GetNumMultisampleQualities(const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat, int level) override;

	UINT GetAdapter() const;
	UnknownRefCounted<IDirect3D9> GetD3D9() const;
private:
	UnknownRefCounted<IDirect3D9> m_D3D9;

	UINT m_Adapter;

	core::String m_Name;
	u32 m_Vendor;
	u32 m_Device;
};

///////////////////////////////////////////////////////////////////////////////

class AdapterListD3D9 : public AdapterList
{
public:
	AdapterListD3D9();
	int GetCount() const override;
	StrongRef<Adapter> GetAdapter(int idx) const override;
	StrongRef<Adapter> GetDefaultAdapter() const override;

private:
	UnknownRefCounted<IDirect3D9> m_D3D9;
	core::Array<StrongRef<AdapterD3D9>> m_Adapters;
};

} // namespace video
} // namespace lux

#endif
#endif // #ifndef INCLUDED_LUX_ADAPTER_INFORMATION_D3D9_H