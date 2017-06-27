#ifdef LUX_COMPILE_WITH_D3D9
#include "core/Logger.h"

#include "video/d3d9/TextureD3D9.h"
#include "video/d3d9/CubeTextureD3D9.h"
#include "video/d3d9/ShaderD3D9.h"
#include "video/d3d9/HardwareBufferManagerD3D9.h"
#include "video/d3d9/VideoDriverD3D9.h"

#include "video/mesh/GeometryImpl.h"

#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"

#include "gui/Window.h"

#include "D3DHelper.h"
#include "D3D9Exception.h"

namespace lux
{
namespace video
{

VideoDriverD3D9::DepthBuffer_d3d9::DepthBuffer_d3d9(IDirect3DSurface9* surface) :
	m_Surface(surface)
{
	if(m_Surface) {
		D3DSURFACE_DESC desc;
		m_Surface->GetDesc(&desc);
		m_Size.width = desc.Width;
		m_Size.height = desc.Height;
	}
}

//////////////////////////////////////////////////////////////////////

VideoDriverD3D9::VideoDriverD3D9() :
	m_HasStencilBuffer(false)
{
}

void VideoDriverD3D9::CleanUp()
{
	m_Renderer->CleanUp();
}

void VideoDriverD3D9::Init(const DriverConfig& config, gui::Window* window)
{
	VideoDriverNull::Init(config, window);

	D3DPRESENT_PARAMETERS PresentParams;

	m_Adapter = 0;

	m_D3D = Direct3DCreate9(D3D_SDK_VERSION);
	if(!m_D3D)
		throw core::RuntimeException("Couldn't create the direct3D9 interface.");

	HWND windowHandle = (HWND)window->GetDeviceWindow();
	// Präsentationsstruktur ausfüllen
	D3DDISPLAYMODE CurrentVideoMode;
	m_D3D->GetAdapterDisplayMode(m_Adapter, &CurrentVideoMode);
	memset(&PresentParams, 0, sizeof(D3DPRESENT_PARAMETERS));
	PresentParams.BackBufferWidth = m_Config.width;
	PresentParams.BackBufferHeight = m_Config.height;
	if(m_Config.windowed) {
		PresentParams.BackBufferFormat = CurrentVideoMode.Format;
	} else {
		PresentParams.BackBufferFormat = m_Config.backbuffer16Bit ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8;
	}
	PresentParams.BackBufferCount = 1;
	PresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	PresentParams.hDeviceWindow = windowHandle;
	PresentParams.Windowed = m_Config.windowed;
	PresentParams.EnableAutoDepthStencil = TRUE;
	PresentParams.Flags = 0;
	PresentParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	PresentParams.PresentationInterval = m_Config.vSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

	// Z-Stencil-Format wählen
	D3DFORMAT aZStencilFormat[] = {D3DFMT_D24X4S4,
		D3DFMT_D24S8,
		D3DFMT_D15S1,
		D3DFMT_D32,
		D3DFMT_D24X8,
		D3DFMT_D16};

	PresentParams.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	D3DFORMAT FallbackFormat = D3DFMT_D24S8;
	for(u32 format = m_Config.stencil ? 0 : 3; format < 6; format++) {
		if(SUCCEEDED(m_D3D->CheckDeviceFormat(m_Adapter,
			D3DDEVTYPE_HAL,
			PresentParams.BackBufferFormat,
			D3DUSAGE_DEPTHSTENCIL,
			D3DRTYPE_SURFACE,
			aZStencilFormat[format]))) {
			//Passt es zum Bildpufferformat
			if(SUCCEEDED(m_D3D->CheckDepthStencilMatch(m_Adapter,
				D3DDEVTYPE_HAL,
				PresentParams.BackBufferFormat,
				PresentParams.BackBufferFormat,
				aZStencilFormat[format]))) {
				// Wieviele ZBits hat das gewählte Format
				u32 numZBits;
				switch(aZStencilFormat[format]) {
				case D3DFMT_D32:
					numZBits = 32;
					break;

				case D3DFMT_D24X8:
				case D3DFMT_D24S8:
				case D3DFMT_D24X4S4:
					numZBits = 24;
					break;

				case D3DFMT_D16:
				case D3DFMT_D15S1:        //Nicht ganz exakt aber was solls
					numZBits = 16;
					break;
				default:
					numZBits = 0; // This will fail later on.
				}

				FallbackFormat = aZStencilFormat[format];

				// Hat das Format genug Bits
				if(m_Config.zBits == numZBits) {
					// Format speichern und abbrechen
					PresentParams.AutoDepthStencilFormat = aZStencilFormat[format];

					// Ist ein Stencilbuffer vorhanden
					switch(PresentParams.AutoDepthStencilFormat) {
					case D3DFMT_D15S1:
					case D3DFMT_D24S8:
					case D3DFMT_D24X8:
					case D3DFMT_D24X4S4:
					case D3DFMT_D24FS8:
						m_HasStencilBuffer = true;
						break;
					default:
						m_HasStencilBuffer = false;
					}

					// Information nach außen zurückgeben
					m_Config.stencil = m_HasStencilBuffer;
					break;
				}
			}
		}
	}

	// Kein passendes Format dabei
	if(PresentParams.AutoDepthStencilFormat == D3DFMT_UNKNOWN) {
		PresentParams.AutoDepthStencilFormat = FallbackFormat;
		log::Warning("The requested z-stencil-format isn't supported, using fallback.");
	}

	// Multisampling einstellen
	if(m_Config.multiSampling > 0) {
		if(m_Config.multiSampling > 16)
			m_Config.multiSampling = 16;

		DWORD numQualities = 0;
		while(m_Config.multiSampling > 0) {
			// Ist das Verfahren vorhanden
			if(SUCCEEDED(m_D3D->CheckDeviceMultiSampleType(m_Adapter,
				D3DDEVTYPE_HAL,
				PresentParams.BackBufferFormat,
				m_Config.windowed,
				(D3DMULTISAMPLE_TYPE)(m_Config.multiSampling),
				&numQualities))) {
				// Aktivieren und abbrechen
				PresentParams.MultiSampleType = static_cast<D3DMULTISAMPLE_TYPE>(m_Config.multiSampling);
				PresentParams.MultiSampleQuality = numQualities - 1;
				break;
			}

			--m_Config.multiSampling;
		}

		if(m_Config.multiSampling <= 0) {
			// Nichts hat funktioniert
			log::Warning("Multisampling isn't supported by the hardware.");
		}
	}

	// Geräteschnittstelle generieren
	IDirect3DDevice9* newDevice;
	HRESULT hr;
	if(m_Config.pureSoftware) {
		if(FAILED(hr = m_D3D->CreateDevice(m_Adapter,
			D3DDEVTYPE_HAL,
			windowHandle,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&PresentParams,
			&newDevice))) {
			throw core::D3D9Exception(hr);
		}
	} else {
		hr = m_D3D->CreateDevice(m_Adapter,
			D3DDEVTYPE_HAL,
			windowHandle,
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&PresentParams,
			&newDevice);
		if(FAILED(hr))
			hr = m_D3D->CreateDevice(m_Adapter,
				D3DDEVTYPE_HAL,
				windowHandle,
				D3DCREATE_MIXED_VERTEXPROCESSING,
				&PresentParams,
				&newDevice);

		if(FAILED(hr))
			hr = m_D3D->CreateDevice(m_Adapter,
				D3DDEVTYPE_HAL,
				windowHandle,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING,
				&PresentParams,
				&newDevice);

		if(FAILED(hr))
			throw core::D3D9Exception(hr);
	}

	m_D3DDevice = newDevice;

	D3DADAPTER_IDENTIFIER9 AdapterIdent;
	m_D3D->GetAdapterIdentifier(m_Adapter, 0, &AdapterIdent);
	log::Info("Using Direct3D-Driver ~d.~d.~d.~d on ~s.",
		HIWORD(AdapterIdent.DriverVersion.HighPart),
		LOWORD(AdapterIdent.DriverVersion.HighPart),
		HIWORD(AdapterIdent.DriverVersion.LowPart),
		LOWORD(AdapterIdent.DriverVersion.LowPart),
		AdapterIdent.Description);

	//Parameter eintragen
	m_PresentParams = PresentParams;

	// Gerätefähigkeiten speichern
	// Aktuellen Status speichern
	m_D3DDevice->GetDeviceCaps(&m_Caps);

	FillCaps();

	InitRendertargetData();

	m_D3DDevice->SetRenderState(D3DRS_AMBIENT, 0xFFFFFFFF);

	m_BufferManager = LUX_NEW(BufferManagerD3D9)(this);
	m_Renderer = new RendererD3D9(this);

	auto refFactory = core::ReferableFactory::Instance();
	refFactory->RegisterType(LUX_NEW(TextureD3D9)(m_D3DDevice));
	refFactory->RegisterType(LUX_NEW(CubeTextureD3D9)(m_D3DDevice));
}

VideoDriverD3D9::~VideoDriverD3D9()
{
	// Destroy buffer manager before the device is destroyed
	m_BufferManager.Reset();

	m_VertexFormats.Clear();
	m_DepthBuffers.Clear();
}

void VideoDriverD3D9::FillCaps()
{
	m_DriverCaps[(u32)EDriverCaps::MaxPrimitives] = m_Caps.MaxPrimitiveCount;
	m_DriverCaps[(u32)EDriverCaps::MaxStreams] = m_Caps.MaxStreams;
	m_DriverCaps[(u32)EDriverCaps::MaxTextureWidth] = m_Caps.MaxTextureWidth;
	m_DriverCaps[(u32)EDriverCaps::MaxTextureHeight] = m_Caps.MaxTextureHeight;
	m_DriverCaps[(u32)EDriverCaps::TexturesPowerOfTwoOnly] = (m_Caps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0 ? 1 : 0;
	m_DriverCaps[(u32)EDriverCaps::TextureSquareOnly] = (m_Caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) != 0 ? 1 : 0;
	m_DriverCaps[(u32)EDriverCaps::MaxSimultaneousTextures] = m_Caps.MaxSimultaneousTextures;
	m_DriverCaps[(u32)EDriverCaps::MaxLights] = m_Caps.MaxActiveLights;
	m_DriverCaps[(u32)EDriverCaps::MaxAnisotropy] = m_Caps.MaxAnisotropy;
}

void VideoDriverD3D9::InitRendertargetData()
{
	HRESULT hr;
	IDirect3DSurface9* backbuffer = nullptr;
	if(FAILED(hr = m_D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer)))
		throw core::D3D9Exception(hr);

	m_BackBufferTarget = backbuffer;

	IDirect3DSurface9* depthStencilBuffer = nullptr;
	if(FAILED(hr = m_D3DDevice->GetDepthStencilSurface(&depthStencilBuffer)))
		throw core::D3D9Exception(hr);

	DepthBuffer_d3d9 depthBuffer(depthStencilBuffer);
	m_DepthBuffers.PushBack(depthBuffer);
}

IDirect3DSurface9* VideoDriverD3D9::GetD3D9MatchingDepthBuffer(IDirect3DSurface9* target)
{
	D3DSURFACE_DESC desc;
	target->GetDesc(&desc);

	math::dimension2du size = math::dimension2du(desc.Width, desc.Height);

	size.GetConstrained(
		GetDeviceCapability(EDriverCaps::TexturesPowerOfTwoOnly) == 1,
		GetDeviceCapability(EDriverCaps::TextureSquareOnly) == 1,
		true,
		math::dimension2du(
			GetDeviceCapability(EDriverCaps::MaxTextureWidth),
			GetDeviceCapability(EDriverCaps::MaxTextureWidth)));

	if(size.width == 0 && size.height == 0)
		return nullptr;

	for(auto it = m_DepthBuffers.First(); it != m_DepthBuffers.End(); ++it) {
		if(size.DoesFitInto(it->GetSize()))
			return it->GetSurface();
	}

	IDirect3DSurface9* depthStencil = nullptr;
	if(SUCCEEDED(m_D3DDevice->CreateDepthStencilSurface(size.width, size.height,
		m_PresentParams.AutoDepthStencilFormat,
		desc.MultiSampleType, desc.MultiSampleQuality,
		TRUE,
		&depthStencil, nullptr))) {
		DepthBuffer_d3d9 buffer(depthStencil);
		m_DepthBuffers.PushBack(buffer);
		return depthStencil;
	}

	return nullptr;
}

StrongRef<Geometry> VideoDriverD3D9::CreateEmptyGeometry(EPrimitiveType primitiveType)
{
	StrongRef<Geometry> out = LUX_NEW(GeometryImpl);
	out->SetPrimitiveType(primitiveType);

	return out;
}

StrongRef<Geometry> VideoDriverD3D9::CreateGeometry(
	const VertexFormat& vertexFormat, EHardwareBufferMapping VertexHWMapping, u32 vertexCount,
	EIndexFormat indexType, EHardwareBufferMapping IndexHWMapping, u32 IndexCount,
	EPrimitiveType primitiveType)
{
	StrongRef<Geometry> out = CreateEmptyGeometry(primitiveType);
	StrongRef<IndexBuffer> ib = m_BufferManager->CreateIndexBuffer();
	ib->SetType(indexType);
	ib->SetHWMapping(IndexHWMapping);
	ib->SetSize(IndexCount);

	StrongRef<VertexBuffer> vb = m_BufferManager->CreateVertexBuffer();
	vb->SetFormat(vertexFormat);
	vb->SetHWMapping(VertexHWMapping);
	vb->SetSize(vertexCount);

	out->SetIndices(ib);
	out->SetVertices(vb);

	return out;
}

StrongRef<Geometry> VideoDriverD3D9::CreateGeometry(const VertexFormat& vertexFormat,
	EPrimitiveType primitiveType,
	u32 primitiveCount,
	bool dynamic)
{
	LUX_UNUSED(primitiveCount);
	return CreateGeometry(vertexFormat, dynamic ? EHardwareBufferMapping::Dynamic : EHardwareBufferMapping::Static, 0,
		EIndexFormat::Bit16, EHardwareBufferMapping::Static, 0,
		primitiveType);
}

bool VideoDriverD3D9::CheckTextureFormat(ColorFormat format, bool cube)
{
	D3DFORMAT d3dFormat = GetD3DFormat(format, format.HasAlpha());

	D3DRESOURCETYPE rType;
	DWORD usage;
	if(cube)
		rType = D3DRTYPE_CUBETEXTURE;
	else
		rType = D3DRTYPE_TEXTURE;

	usage = 0;

	HRESULT hr = m_D3D->CheckDeviceFormat(m_Adapter,
		D3DDEVTYPE_HAL,
		m_PresentParams.BackBufferFormat,
		usage,
		rType,
		d3dFormat);

	return SUCCEEDED(hr);
}

StrongRef<Texture> VideoDriverD3D9::CreateTexture(const math::dimension2du& size, ColorFormat format, u32 mipCount, bool isDynamic)
{
	StrongRef<TextureD3D9> out = LUX_NEW(TextureD3D9)(m_D3DDevice);
	out->Init(size, format, mipCount, false, isDynamic);
	return out;
}

StrongRef<Texture> VideoDriverD3D9::CreateRendertargetTexture(const math::dimension2du& size, ColorFormat format)
{
	StrongRef<TextureD3D9> out = LUX_NEW(TextureD3D9)(m_D3DDevice);
	out->Init(size, format, 1, true, false);

	m_RenderTargets.PushBack(out);
	return out;
}

StrongRef<CubeTexture> VideoDriverD3D9::CreateCubeTexture(u32 size, ColorFormat format, bool isDynamic)
{
	StrongRef<CubeTextureD3D9> out = LUX_NEW(CubeTextureD3D9)(m_D3DDevice);
	out->Init(size, format, isDynamic);
	return out;
}

bool VideoDriverD3D9::GetFittingTextureFormat(ColorFormat& format, math::dimension2du& size, bool cube)
{
	LUX_UNUSED(size);

	// TODO: Handle size
	// TODO: How to use floating-point formats
	bool hasAlpha = format.HasAlpha();
	if(hasAlpha) {
		if(CheckTextureFormat(format, cube))
			format = format;
		else if(CheckTextureFormat(ColorFormat::A8R8G8B8, cube))
			format = ColorFormat::A8R8G8B8;
		else if(CheckTextureFormat(ColorFormat::A1R5G5B5, cube))
			format = ColorFormat::A1R5G5B5;
		else
			return false;
	} else {
		if(CheckTextureFormat(format, cube))
			format = format;
		else if(CheckTextureFormat(ColorFormat::A8R8G8B8, cube))
			format = ColorFormat::A8R8G8B8;
		else if(CheckTextureFormat(ColorFormat::R5G6B5, cube))
			format = ColorFormat::A1R5G5B5;
		else
			return false;
	}

	return true;
}

StrongRef<Shader> VideoDriverD3D9::CreateShader(
	EShaderLanguage language,
	const char* VSCode, const char* VSEntryPoint, u32 VSLength, int VSmajorVersion, int VSminorVersion,
	const char* PSCode, const char* PSEntryPoint, u32 PSLength, int PSmajorVersion, int PSminorVersion,
	core::array<string>* errorList)
{
	if(language != EShaderLanguage::HLSL)
		throw core::InvalidArgumentException("language", "Direct3D9 video driver only supports HLSL shaders.");

	const char* vsProfile = GetD3DXShaderProfile(false, VSmajorVersion, VSminorVersion);
	const char* psProfile = GetD3DXShaderProfile(true, PSmajorVersion, PSminorVersion);

	if(!vsProfile)
		throw core::InvalidArgumentException("vertex shader profile", "Invalid vertex shader profile(~d.~d).");

	if(!psProfile)
		throw core::InvalidArgumentException("pixel shader profile", "Invalid pixel shader profile(~d.~d).");

	StrongRef<ShaderD3D9> out = LUX_NEW(ShaderD3D9)(this);
	out->Init(VSCode, VSEntryPoint, VSLength, vsProfile, PSCode, PSEntryPoint, PSLength, psProfile, errorList);

	return out;
}

const RendertargetD3D9& VideoDriverD3D9::GetBackbufferTarget()
{
	return m_BackBufferTarget;
}

IDirect3DVertexDeclaration9* VideoDriverD3D9::GetD3D9VertexDeclaration(const VertexFormat& format)
{
	VertexFormat_d3d9 vf;
	auto it = m_VertexFormats.Find(format);
	if(it == m_VertexFormats.End()) {
		IDirect3DVertexDeclaration9* d3d = CreateVertexFormat(format);
		vf = VertexFormat_d3d9(d3d);
		m_VertexFormats[format] = vf;
	} else {
		vf = *it;
	}

	return vf.GetD3D();
}

IDirect3DVertexDeclaration9* VideoDriverD3D9::CreateVertexFormat(const VertexFormat& format)
{
	if(!format.IsValid())
		throw core::InvalidArgumentException("format");

	if(format.GetElement(0, VertexElement::EUsage::Position).sematic != VertexElement::EUsage::Position &&
		format.GetElement(0, VertexElement::EUsage::PositionNT).sematic != VertexElement::EUsage::PositionNT)
		throw core::InvalidArgumentException("format", "Missing position usage");

	core::array<D3DVERTEXELEMENT9> d3dElements;
	d3dElements.Resize(format.GetElemCount() + 1);

	for(u32 stream = 0; stream < format.GetStreamCount(); ++stream) {
		for(u32 elem = 0; elem < format.GetElemCount(stream); ++elem) {
			bool error = false;

			auto element = format.GetElement(stream, elem);
			d3dElements[elem].Stream = element.stream;
			d3dElements[elem].Offset = element.offset;
			d3dElements[elem].Method = D3DDECLMETHOD_DEFAULT;

			if(element.sematic == VertexElement::EUsage::Texcoord0)
				d3dElements[elem].UsageIndex = 0;
			else if(element.sematic == VertexElement::EUsage::Texcoord1)
				d3dElements[elem].UsageIndex = 1;
			else if(element.sematic == VertexElement::EUsage::Texcoord2)
				d3dElements[elem].UsageIndex = 2;
			else if(element.sematic == VertexElement::EUsage::Texcoord3)
				d3dElements[elem].UsageIndex = 3;
			else if(element.sematic == VertexElement::EUsage::Specular)
				d3dElements[elem].UsageIndex = 1;
			else
				d3dElements[elem].UsageIndex = 0;

			d3dElements[elem].Type = (BYTE)GetD3DDeclType(element.type);
			if(d3dElements[elem].Type == D3DDECLTYPE_UNUSED)
				error = true;

			d3dElements[elem].Usage = GetD3DUsage(element.sematic);
			if(d3dElements[elem].Usage == 0xFF)
				error = true;

			if(error)
				throw core::Exception("Unknown vertex element type");
		}
	}

	const u32 size = format.GetElemCount();
	d3dElements[size].Stream = 0xFF;
	d3dElements[size].Offset = 0;
	d3dElements[size].Type = D3DDECLTYPE_UNUSED;
	d3dElements[size].Method = 0;
	d3dElements[size].Usage = 0;
	d3dElements[size].UsageIndex = 0;

	IDirect3DVertexDeclaration9* d3dDecl;
	HRESULT hr;
	if(FAILED(hr = m_D3DDevice->CreateVertexDeclaration(d3dElements.Data(), &d3dDecl)))
		throw core::D3D9Exception(hr);

	return d3dDecl;
}

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
