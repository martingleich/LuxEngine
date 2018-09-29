#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "AdapterInformationD3D9.h"

#include "video/DriverConfig.h"
#include "platform/D3D9Exception.h"
#include "D3DHelper.h"

#include "core/ModuleFactory.h"

LUX_REGISTER_MODULE("AdapterList", "Direct3D9", lux::video::AdapterListD3D9);

namespace lux
{
namespace video
{

AdapterD3D9::AdapterD3D9(UnknownRefCounted<IDirect3D9> d3d9, UINT adapter) :
	m_D3D9(d3d9),
	m_Adapter(adapter)
{
	D3DADAPTER_IDENTIFIER9 identifier;
	HRESULT hr = m_D3D9->GetAdapterIdentifier(m_Adapter, 0, &identifier);
	if(FAILED(hr)) {
		m_Name = "unknown";
		m_Vendor = 0;
		m_Device = 0;
	} else {
		m_Name = identifier.Description;
		m_Vendor = identifier.VendorId;
		m_Device = identifier.DeviceId;
	}
}

const core::String& AdapterD3D9::GetName() const
{
	return m_Name;
}

u32 AdapterD3D9::GetVendor() const
{
	return m_Vendor;
}

u32 AdapterD3D9::GetDevice() const
{
	return m_Device;
}

const core::String& AdapterD3D9::GetDriverType() const
{
	return DriverType::Direct3D9;
}

UINT AdapterD3D9::GetAdapter() const
{
	return m_Adapter;
}

UnknownRefCounted<IDirect3D9> AdapterD3D9::GetD3D9() const
{
	return m_D3D9;
}

core::Array<DisplayMode> AdapterD3D9::GenerateDisplayModes(bool windowed)
{
	LUX_UNUSED(windowed);

	core::Array<DisplayMode> outModes;
	HRESULT hr;
	D3DDISPLAYMODE displayMode;
	if(FAILED(hr = m_D3D9->GetAdapterDisplayMode(m_Adapter, &displayMode)))
		throw core::D3D9Exception(hr);

	D3DFORMAT displayFormat = displayMode.Format;

	// Use the current format to enumerate modes
	UINT modeCount = m_D3D9->GetAdapterModeCount(m_Adapter, displayFormat);
	for(UINT i = 0; i < modeCount; ++i) {
		if(SUCCEEDED(hr = m_D3D9->EnumAdapterModes(m_Adapter, displayFormat, i, &displayMode))) {
			DisplayMode m;
			m.width = displayMode.Width;
			m.height = displayMode.Height;
			m.refreshRate = displayMode.RefreshRate;
			m.format = video::GetLuxFormat(displayMode.Format);
			if(m.format != video::ColorFormat::UNKNOWN)
				outModes.PushBack(m);
		}
	}

	return outModes;
}

core::Array<ColorFormat> AdapterD3D9::GenerateBackbufferFormats(const DisplayMode& mode, bool windowed)
{
	core::Array<ColorFormat> outFormats;
	LUX_UNUSED(windowed);

	D3DFORMAT formats[] = {D3DFMT_A8R8G8B8, D3DFMT_R5G6B5, D3DFMT_A1R5G5B5};
	D3DDEVTYPE devType = D3DDEVTYPE_HAL;
	D3DFORMAT adapterFormat = GetD3DFormat(mode.format);
	HRESULT hr;
	for(auto format : formats) {
		// Check if format is supported
		hr = m_D3D9->CheckDeviceFormat(
			m_Adapter,
			devType,
			adapterFormat,
			D3DUSAGE_RENDERTARGET,
			D3DRTYPE_SURFACE,
			format);
		if(FAILED(hr))
			continue;

		// Check if device type is supported with format
		hr = m_D3D9->CheckDeviceType(
			m_Adapter,
			devType,
			adapterFormat,
			format,
			windowed ? TRUE : FALSE);
		if(FAILED(hr))
			continue;

		if(windowed && adapterFormat != format) {
			// Check if a format conversion is possible
			hr = m_D3D9->CheckDeviceFormatConversion(
				m_Adapter,
				devType,
				format,
				adapterFormat);
			if(FAILED(hr))
				continue;
		}

		// The format should work.
		outFormats.PushBack(GetLuxFormat(format));
	}

	return outFormats;
}

core::Array<ZStencilFormat> AdapterD3D9::GenerateZStencilFormats(const DisplayMode& mode, bool windowed, ColorFormat backBuffer)
{
	core::Array<ZStencilFormat> out;
	LUX_UNUSED(windowed);

	D3DFORMAT formats[] = {
		D3DFMT_D32,
		D3DFMT_D24X8, D3DFMT_D24S8, D3DFMT_D24X4S4,
		D3DFMT_D16,
		D3DFMT_D15S1};

	D3DDEVTYPE devType = D3DDEVTYPE_HAL;
	D3DFORMAT adapterFormat = GetD3DFormat(mode.format);
	D3DFORMAT backBufferFormat = GetD3DFormat(backBuffer);

	HRESULT hr;
	for(auto format : formats) {
		// Check if format is supported
		hr = m_D3D9->CheckDeviceFormat(
			m_Adapter,
			devType,
			adapterFormat,
			D3DUSAGE_DEPTHSTENCIL,
			D3DRTYPE_SURFACE,
			format);
		if(FAILED(hr))
			continue;

		// Check if device type is supported with format
		hr = m_D3D9->CheckDepthStencilMatch(
			m_Adapter,
			devType,
			adapterFormat,
			backBufferFormat,
			format);
		if(FAILED(hr))
			continue;

		// The format should work.
		out.PushBack(GetZStencil(format));
	}

	return out;
}

core::Array<int> AdapterD3D9::GenerateMultisampleLevels(const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat)
{
	LUX_UNUSED(mode);

	core::Array<int> out;
	D3DMULTISAMPLE_TYPE samplingTypes[] = {
		D3DMULTISAMPLE_NONE,
		D3DMULTISAMPLE_NONMASKABLE,
		D3DMULTISAMPLE_2_SAMPLES,
		D3DMULTISAMPLE_3_SAMPLES,
		D3DMULTISAMPLE_4_SAMPLES,
		D3DMULTISAMPLE_5_SAMPLES,
		D3DMULTISAMPLE_6_SAMPLES,
		D3DMULTISAMPLE_7_SAMPLES,
		D3DMULTISAMPLE_8_SAMPLES,
		D3DMULTISAMPLE_9_SAMPLES,
		D3DMULTISAMPLE_10_SAMPLES,
		D3DMULTISAMPLE_11_SAMPLES,
		D3DMULTISAMPLE_12_SAMPLES,
		D3DMULTISAMPLE_13_SAMPLES,
		D3DMULTISAMPLE_14_SAMPLES,
		D3DMULTISAMPLE_15_SAMPLES,
		D3DMULTISAMPLE_16_SAMPLES};

	D3DDEVTYPE devType = D3DDEVTYPE_HAL;
	//D3DFORMAT adapterFormat = GetD3DFormat(mode.format, false);
	D3DFORMAT backBufferFormat = GetD3DFormat(backBuffer);
	D3DFORMAT zStencilFormat = GetD3DFormat(zsFormat);

	HRESULT hr;

	DWORD numQualities;
	for(auto samplingType : samplingTypes) {
		hr = m_D3D9->CheckDeviceMultiSampleType(
			m_Adapter,
			devType,
			backBufferFormat,
			windowed ? TRUE : FALSE,
			samplingType,
			&numQualities);
		if(FAILED(hr) || numQualities == 0)
			continue;

		hr = m_D3D9->CheckDeviceMultiSampleType(
			m_Adapter,
			devType,
			zStencilFormat,
			windowed ? TRUE : FALSE,
			samplingType,
			&numQualities);

		if(FAILED(hr) || numQualities == 0)
			continue;

		out.PushBack(samplingType);
	}

	return out;
}

int AdapterD3D9::GetNumMultisampleQualities(const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat, int level)
{
	LUX_UNUSED(mode);
	LUX_UNUSED(zsFormat);

	D3DDEVTYPE devType = D3DDEVTYPE_HAL;
	//D3DFORMAT adapterFormat = GetD3DFormat(mode.format);
	D3DFORMAT backBufferFormat = GetD3DFormat(backBuffer);
	//D3DFORMAT zStencilFormat = GetD3DFormat(zsFormat);
	D3DMULTISAMPLE_TYPE sampleType = (D3DMULTISAMPLE_TYPE)level;

	HRESULT hr;

	DWORD numQualities;

	hr = m_D3D9->CheckDeviceMultiSampleType(
		m_Adapter,
		devType,
		backBufferFormat,
		windowed ? TRUE : FALSE,
		sampleType,
		&numQualities);
	if(FAILED(hr))
		return 0;

	return (int)numQualities;
}

///////////////////////////////////////////////////////////////////////////////

AdapterListD3D9::AdapterListD3D9(const core::ModuleInitData& data)
{
	LUX_UNUSED(data);

	m_D3D9.TakeOwnership(Direct3DCreate9(D3D_SDK_VERSION));
	if(!m_D3D9)
		throw core::GenericRuntimeException("Couldn't create the Direct3D9 interface.");

	UINT count = m_D3D9->GetAdapterCount();
	for(UINT i = 0; i < count; ++i)
		m_Adapters.PushBack(LUX_NEW(AdapterD3D9)(m_D3D9, i));
}

u32 AdapterListD3D9::GetAdapterCount() const
{
	return m_Adapters.Size();
}

StrongRef<Adapter> AdapterListD3D9::GetAdapter(u32 idx) const
{
	return m_Adapters.At(idx);
}

} // namespace video
} // namespace lux

#endif