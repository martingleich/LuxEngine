#ifdef LUX_COMPILE_WITH_D3D9
#include "video/d3d9/VideoDriverD3D9.h"
#include "core/Logger.h"

#include "core/ReferableFactory.h"

#include "video/d3d9/TextureD3D9.h"
#include "video/d3d9/CubeTextureD3D9.h"
#include "video/d3d9/ShaderD3D9.h"

#include "video/mesh/Geometry.h"
#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"

#include "video/d3d9/D3DHelper.h"
#include "platform/D3D9Exception.h"
#include "video/d3d9/AuxiliaryTextureD3D9.h"

#include "core/ModuleFactoryRegister.h"

LUX_REGISTER_MODULE("VideoDriver", "Direct3D9", lux::video::VideoDriverD3D9);

namespace lux
{
namespace video
{

VideoDriverD3D9::DepthBuffer_d3d9::DepthBuffer_d3d9(UnknownRefCounted<IDirect3DSurface9> surface) :
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

static IDirect3DDevice9* g_D3DDevice9 = nullptr;
static VideoDriverD3D9* g_Driver = nullptr;

Referable* CreateTexture(const void* origin)
{
	auto out = LUX_NEW(TextureD3D9)(g_D3DDevice9, origin ? *(core::ResourceOrigin*)origin : core::ResourceOrigin());
	g_Driver->AddTextureToList(out);
	return out;
}

Referable* CreateCubeTexture(const void* origin)
{
	auto out = LUX_NEW(CubeTextureD3D9)(g_D3DDevice9, origin ? *(core::ResourceOrigin*)origin : core::ResourceOrigin());
	g_Driver->AddTextureToList(out);
	return out;
}

//////////////////////////////////////////////////////////////////////

VideoDriverD3D9::VideoDriverD3D9(const core::ModuleInitData& data) :
	VideoDriverNull(dynamic_cast<const VideoDriverInitData&>(data)),
	m_ReleasedUnmanagedData(false)
{
	auto initData = dynamic_cast<const VideoDriverInitData&>(data);
	m_Window = (HWND)initData.destHandle;
	CreateDevice(initData.config, m_Window);

	m_BufferManager = LUX_NEW(BufferManagerD3D9)(this);
	m_Renderer = new RendererD3D9(this, m_DeviceState);

	g_D3DDevice9 = m_D3DDevice;
	g_Driver = this;

	AuxiliaryTextureManagerD3D9::Initialize(m_D3DDevice);

	auto refFactory = core::ReferableFactory::Instance();
	refFactory->RegisterType(core::ResourceType::Texture, &lux::video::CreateTexture);
	refFactory->RegisterType(core::ResourceType::CubeTexture, &lux::video::CreateCubeTexture);
}

VideoDriverD3D9::~VideoDriverD3D9()
{
	m_D3DDevice->SetVertexShader(nullptr);
	m_D3DDevice->SetPixelShader(nullptr);

	AuxiliaryTextureManagerD3D9::Destroy();
	
	// Destroy buffer manager before the device is destroyed
	m_BufferManager.Reset();

	m_VertexFormats.Clear();
	m_D3DDevice->SetVertexDeclaration(nullptr);

	m_DepthBuffers.Clear();
}

void VideoDriverD3D9::CreateDevice(const DriverConfig& config, HWND windowHandle)
{
	m_Adapter = config.adapter.As<AdapterD3D9>();
	if(!m_Adapter)
		throw core::InvalidArgumentException("config", "Contains invalid adapter");

	m_D3D = m_Adapter->GetD3D9();

	// Fill presentation params
	m_AdapterFormat = GetD3DFormat(config.display.format);
	D3DPRESENT_PARAMETERS presentParams = GeneratePresentParams(config);

	UINT adapterId = m_Adapter->GetAdapter();
	HRESULT hr;
	hr = m_D3D->CreateDevice(adapterId,
		D3DDEVTYPE_HAL,
		windowHandle,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParams,
		m_D3DDevice.Access());

	if(FAILED(hr))
		hr = m_D3D->CreateDevice(adapterId,
			D3DDEVTYPE_HAL,
			windowHandle,
			D3DCREATE_MIXED_VERTEXPROCESSING,
			&presentParams,
			m_D3DDevice.Access());

	if(FAILED(hr))
		hr = m_D3D->CreateDevice(adapterId,
			D3DDEVTYPE_HAL,
			windowHandle,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&presentParams,
			m_D3DDevice.Access());

	if(FAILED(hr))
		throw core::D3D9Exception(hr);

	// Save present params
	m_PresentParams = presentParams;

	// Save device caps and convert them to platform independent
	m_D3DDevice->GetDeviceCaps(&m_Caps);
	FillCaps();

	// Init rendertarget data
	InitRendertargetData();

	m_DeviceState.Init(&m_Caps, m_D3DDevice);
}

D3DPRESENT_PARAMETERS VideoDriverD3D9::GeneratePresentParams(const DriverConfig& config)
{
	D3DPRESENT_PARAMETERS presentParams = {0};
	presentParams.BackBufferWidth = config.display.width;
	presentParams.BackBufferHeight = config.display.height;
	presentParams.BackBufferFormat = GetD3DFormat(config.backBufferFormat);
	presentParams.FullScreen_RefreshRateInHz = config.windowed ? 0 : config.display.refreshRate;
	presentParams.BackBufferCount = 1;
	presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParams.hDeviceWindow = m_Window;
	presentParams.Windowed = m_Config.windowed;
	presentParams.EnableAutoDepthStencil = TRUE;
	presentParams.Flags = 0;
	presentParams.PresentationInterval = m_Config.vSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	presentParams.AutoDepthStencilFormat = GetD3DFormat(config.zsFormat);
	presentParams.MultiSampleType = static_cast<D3DMULTISAMPLE_TYPE>(m_Config.multiSamples);
	presentParams.MultiSampleQuality = m_Config.multiQuality;

	return presentParams;
}

bool VideoDriverD3D9::Reset(const DriverConfig& config)
{
	auto newAdapter = config.adapter.As<AdapterD3D9>();
	if(newAdapter != nullptr && newAdapter->GetAdapter() != m_Adapter->GetAdapter())
		throw core::RuntimeException("Driverreset must use same adapter.");

	// Release all data
	if(!m_ReleasedUnmanagedData) {
		m_DeviceState.ReleaseUnmanaged();
		m_Renderer->ReleaseUnmanaged();

		m_BackBufferTarget = video::RendertargetD3D9(nullptr);
		m_DepthBuffers.Clear();

		m_BufferManager->ReleaseHardwareBuffers();

		for(auto& tex : m_Textures) {
			auto normalTexD3D = tex.As<TextureD3D9>();
			if(normalTexD3D)
				normalTexD3D->ReleaseUnmanaged();
			else {
				auto cubeTexD3D = tex.As<CubeTextureD3D9>();
				if(cubeTexD3D)
					cubeTexD3D->ReleaseUnmanaged();
			}
		}

		m_ReleasedUnmanagedData = true;
	}

	// Reset device and save new configuration
	auto presentParams = GeneratePresentParams(config);
	HRESULT hr = m_D3DDevice->Reset(&presentParams);
	if(FAILED(hr))
		return false;

	m_HasStencilBuffer = (config.zsFormat.sBits != 0);
	m_Config = config;
	m_PresentParams = presentParams;

	// Restore data
	InitRendertargetData();
	m_Renderer->Reset();
	m_DeviceState.Reset();
	m_BufferManager->RestoreHardwareBuffers();

	for(auto& tex : m_Textures) {
		auto normalTexD3D = tex.As<TextureD3D9>();
		if(normalTexD3D)
			normalTexD3D->RestoreUnmanaged();
		else {
			auto cubeTexD3D = tex.As<CubeTextureD3D9>();
			if(cubeTexD3D)
				cubeTexD3D->RestoreUnmanaged();
		}
	}

	m_ReleasedUnmanagedData = false;
	return true;
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
	m_DriverCaps[(u32)EDriverCaps::MaxSimultaneousRT] = m_Caps.NumSimultaneousRTs;
}

void VideoDriverD3D9::InitRendertargetData()
{
	HRESULT hr;
	UnknownRefCounted<IDirect3DSurface9> backbuffer;
	if(FAILED(hr = m_D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, backbuffer.Access())))
		throw core::D3D9Exception(hr);

	m_BackBufferTarget = backbuffer;

	UnknownRefCounted<IDirect3DSurface9> depthStencilBuffer;
	if(FAILED(hr = m_D3DDevice->GetDepthStencilSurface(depthStencilBuffer.Access())))
		throw core::D3D9Exception(hr);

	m_DepthBuffers.PushBack(depthStencilBuffer);
}

UnknownRefCounted<IDirect3DSurface9> VideoDriverD3D9::GetD3D9MatchingDepthBuffer(IDirect3DSurface9* target)
{
	D3DSURFACE_DESC desc;
	target->GetDesc(&desc);

	math::Dimension2U size = math::Dimension2U(desc.Width, desc.Height);

	size.GetConstrained(
		GetDeviceCapability(EDriverCaps::TexturesPowerOfTwoOnly) == 1,
		GetDeviceCapability(EDriverCaps::TextureSquareOnly) == 1,
		true,
		math::Dimension2U(
			GetDeviceCapability(EDriverCaps::MaxTextureWidth),
			GetDeviceCapability(EDriverCaps::MaxTextureWidth)));

	if(size.width == 0 && size.height == 0)
		return nullptr;

	for(auto it = m_DepthBuffers.First(); it != m_DepthBuffers.End(); ++it) {
		if(size.DoesFitInto(it->GetSize()))
			return it->GetSurface();
	}

	UnknownRefCounted<IDirect3DSurface9> depthStencil;
	if(SUCCEEDED(m_D3DDevice->CreateDepthStencilSurface(size.width, size.height,
		m_PresentParams.AutoDepthStencilFormat,
		desc.MultiSampleType, desc.MultiSampleQuality,
		TRUE,
		depthStencil.Access(), nullptr))) {
		DepthBuffer_d3d9 buffer(depthStencil);
		m_DepthBuffers.PushBack(buffer);
		return depthStencil;
	}

	return nullptr;
}

StrongRef<Geometry> VideoDriverD3D9::CreateEmptyGeometry(EPrimitiveType primitiveType)
{
	StrongRef<Geometry> out = LUX_NEW(Geometry);
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
	ib->SetFormat(indexType);
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

bool VideoDriverD3D9::CheckTextureFormat(ColorFormat format, bool cube, bool rendertarget)
{
	D3DFORMAT d3dFormat = GetD3DFormat(format);

	D3DRESOURCETYPE rType;
	DWORD usage;
	if(cube)
		rType = D3DRTYPE_CUBETEXTURE;
	else
		rType = D3DRTYPE_TEXTURE;

	usage = 0;
	if(rendertarget)
		usage |= D3DUSAGE_RENDERTARGET;

	HRESULT hr = m_D3D->CheckDeviceFormat(m_Adapter->GetAdapter(),
		D3DDEVTYPE_HAL,
		m_AdapterFormat,
		usage,
		rType,
		d3dFormat);

	return SUCCEEDED(hr);
}

bool VideoDriverD3D9::GetFittingTextureFormat(ColorFormat& format, math::Dimension2U& size, bool cube, bool rendertarget)
{
	LUX_UNUSED(size);

	// TODO: Handle size
	// TODO: How to use floating-point formats
	bool hasAlpha = format.HasAlpha();
	if(hasAlpha) {
		if(CheckTextureFormat(format, cube, rendertarget))
			format = format;
		else if(CheckTextureFormat(ColorFormat::A8R8G8B8, cube, rendertarget))
			format = ColorFormat::A8R8G8B8;
		else if(CheckTextureFormat(ColorFormat::A1R5G5B5, cube, rendertarget))
			format = ColorFormat::A1R5G5B5;
		else
			return false;
	} else {
		if(CheckTextureFormat(format, cube, rendertarget))
			format = format;
		else if(CheckTextureFormat(ColorFormat::A8R8G8B8, cube, rendertarget))
			format = ColorFormat::A8R8G8B8;
		else if(CheckTextureFormat(ColorFormat::R5G6B5, cube, rendertarget))
			format = ColorFormat::A1R5G5B5;
		else
			return false;
	}

	return true;
}

StrongRef<Texture> VideoDriverD3D9::CreateTexture(const math::Dimension2U& size, ColorFormat format, u32 mipCount, bool isDynamic)
{
	StrongRef<TextureD3D9> out = LUX_NEW(TextureD3D9)(m_D3DDevice, core::ResourceOrigin());
	out->Init(size, format, mipCount, false, isDynamic);
	AddTextureToList(out);

	return out;
}

StrongRef<Texture> VideoDriverD3D9::CreateRendertargetTexture(const math::Dimension2U& size, ColorFormat format)
{
	StrongRef<TextureD3D9> out = LUX_NEW(TextureD3D9)(m_D3DDevice, core::ResourceOrigin());
	out->Init(size, format, 1, true, false);
	AddTextureToList(out);

	return out;
}

StrongRef<CubeTexture> VideoDriverD3D9::CreateCubeTexture(u32 size, ColorFormat format, bool isDynamic)
{
	StrongRef<CubeTextureD3D9> out = LUX_NEW(CubeTextureD3D9)(m_D3DDevice, core::ResourceOrigin());
	out->Init(size, format, false, isDynamic);
	AddTextureToList(out);

	return out;
}
StrongRef<CubeTexture> VideoDriverD3D9::CreateRendertargetCubeTexture(u32 size, ColorFormat format)
{
	StrongRef<CubeTextureD3D9> out = LUX_NEW(CubeTextureD3D9)(m_D3DDevice, core::ResourceOrigin());
	out->Init(size, format, true, false);
	AddTextureToList(out);

	return out;
}

void VideoDriverD3D9::AddTextureToList(BaseTexture* tex)
{
	bool found = false;
	for(auto& t : m_Textures) {
		if(!t) {
			t = tex;
			found = true;
			break;
		}
	}
	if(!found)
		m_Textures.PushBack(tex);
}

StrongRef<Shader> VideoDriverD3D9::CreateShader(
	EShaderLanguage language,
	const char* VSCode, const char* VSEntryPoint, u32 VSLength, int VSmajorVersion, int VSminorVersion,
	const char* PSCode, const char* PSEntryPoint, u32 PSLength, int PSmajorVersion, int PSminorVersion,
	core::Array<core::String>* errorList)
{
	if(language != EShaderLanguage::HLSL)
		throw core::InvalidArgumentException("language", "Direct3D9 video driver only supports HLSL shaders.");

	const char* vsProfile = GetD3DXShaderProfile(false, VSmajorVersion, VSminorVersion);
	const char* psProfile = GetD3DXShaderProfile(true, PSmajorVersion, PSminorVersion);

	if(!vsProfile)
		throw core::InvalidArgumentException("vertex shader profile", "Invalid vertex shader profile(~d.~d).");

	if(!psProfile)
		throw core::InvalidArgumentException("pixel shader profile", "Invalid pixel shader profile(~d.~d).");

	StrongRef<ShaderD3D9> out = LUX_NEW(ShaderD3D9)(this, m_DeviceState);
	out->Init(VSCode, VSEntryPoint, VSLength, vsProfile, PSCode, PSEntryPoint, PSLength, psProfile, errorList);

	return out;
}

const RendertargetD3D9& VideoDriverD3D9::GetBackbufferTarget()
{
	return m_BackBufferTarget;
}

EDeviceState VideoDriverD3D9::GetDeviceState() const
{
	HRESULT hr = m_D3DDevice->TestCooperativeLevel();
	if(SUCCEEDED(hr))
		return EDeviceState::OK;
	if(hr == D3DERR_DEVICELOST)
		return EDeviceState::DeviceLost;
	if(hr == D3DERR_DEVICENOTRESET)
		return EDeviceState::NotReset;
	return EDeviceState::Error;
}

UnknownRefCounted<IDirect3DVertexDeclaration9> VideoDriverD3D9::GetD3D9VertexDeclaration(const VertexFormat& format)
{
	auto it = m_VertexFormats.Find(format);
	if(it == m_VertexFormats.End()) {
		auto d3d = CreateVertexFormat(format);
		m_VertexFormats[format] = d3d;
		return d3d;
	} else {
		return *it;
	}
}

UnknownRefCounted<IDirect3DVertexDeclaration9> VideoDriverD3D9::CreateVertexFormat(const VertexFormat& format)
{
	if(!format.IsValid())
		throw core::InvalidArgumentException("format");

	if(format.GetElement(0, VertexElement::EUsage::Position).sematic != VertexElement::EUsage::Position &&
		format.GetElement(0, VertexElement::EUsage::PositionNT).sematic != VertexElement::EUsage::PositionNT)
		throw core::InvalidArgumentException("format", "Missing position usage");

	core::Array<D3DVERTEXELEMENT9> d3dElements;
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

	UnknownRefCounted<IDirect3DVertexDeclaration9> d3dDecl;
	HRESULT hr;
	if(FAILED(hr = m_D3DDevice->CreateVertexDeclaration(d3dElements.Data(), d3dDecl.Access())))
		throw core::D3D9Exception(hr);

	return d3dDecl;
}

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
