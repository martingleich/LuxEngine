#ifdef LUX_COMPILE_WITH_D3D9
#include "core/Logger.h"

#include "video/d3d9/D3DHelper.h"
#include "video/d3d9/TextureD3D9.h"
#include "video/d3d9/CubeTextureD3D9.h"
#include "video/d3d9/ShaderD3D9.h"
#include "video/d3d9/HardwareBufferManagerD3D9.h"
#include "video/d3d9/VideoDriverD3D9.h"

#include "video/LightData.h"
#include "video/SubMeshImpl.h"

#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"

#include "gui/Window.h"

namespace lux
{
namespace video
{

VideoDriverD3D9::VertexFormat_d3d9::VertexFormat_d3d9() : m_D3DDeclaration(nullptr)
{
}

VideoDriverD3D9::VertexFormat_d3d9::VertexFormat_d3d9(IDirect3DVertexDeclaration9* d3dDecl) :
	m_D3DDeclaration(d3dDecl)
{
}

VideoDriverD3D9::VertexFormat_d3d9::VertexFormat_d3d9(const VertexFormat_d3d9& other) :
	m_D3DDeclaration(other.m_D3DDeclaration)
{
	if(m_D3DDeclaration)
		m_D3DDeclaration->AddRef();
}

VideoDriverD3D9::VertexFormat_d3d9& VideoDriverD3D9::VertexFormat_d3d9::operator=(const VertexFormat_d3d9& other)
{
	if(m_D3DDeclaration)
		m_D3DDeclaration->Release();
	if(other.m_D3DDeclaration)
		other.m_D3DDeclaration->AddRef();

	m_D3DDeclaration = other.m_D3DDeclaration;

	return *this;
}

VideoDriverD3D9::VertexFormat_d3d9::VertexFormat_d3d9(VertexFormat_d3d9&& old) : m_D3DDeclaration(old.m_D3DDeclaration)
{
	old.m_D3DDeclaration = nullptr;
}

VideoDriverD3D9::VertexFormat_d3d9& VideoDriverD3D9::VertexFormat_d3d9::operator=(VertexFormat_d3d9&& old)
{
	if(m_D3DDeclaration)
		m_D3DDeclaration->Release();
	m_D3DDeclaration = old.m_D3DDeclaration;
	old.m_D3DDeclaration = nullptr;

	return *this;
}

VideoDriverD3D9::VertexFormat_d3d9::~VertexFormat_d3d9()
{
	if(m_D3DDeclaration)
		m_D3DDeclaration->Release();
	m_D3DDeclaration = nullptr;
}

IDirect3DVertexDeclaration9* VideoDriverD3D9::VertexFormat_d3d9::GetD3D() const
{
	return m_D3DDeclaration;
}

VideoDriverD3D9::DepthBuffer_d3d9::DepthBuffer_d3d9() :
	m_Surface(nullptr)
{
}

VideoDriverD3D9::DepthBuffer_d3d9::DepthBuffer_d3d9(IDirect3DSurface9* surface) :
	m_Surface(surface)
{
	if(m_Surface) {
		D3DSURFACE_DESC desc;
		m_Surface->GetDesc(&desc);
		m_Size.width = desc.Width;
		m_Size.height = desc.Height;
		m_Surface->AddRef();
	}
}

VideoDriverD3D9::DepthBuffer_d3d9& VideoDriverD3D9::DepthBuffer_d3d9::operator=(const DepthBuffer_d3d9& other)
{
	if(m_Surface)
		m_Surface->Release();
	if(other.m_Surface)
		other.m_Surface->AddRef();

	m_Surface = other.m_Surface;
	m_Size = other.m_Size;

	return *this;
}

VideoDriverD3D9::DepthBuffer_d3d9::~DepthBuffer_d3d9()
{
	if(m_Surface)
		m_Surface->Release();
}

const math::dimension2du& VideoDriverD3D9::DepthBuffer_d3d9::GetSize() const
{
	return m_Size;
}

IDirect3DSurface9* VideoDriverD3D9::DepthBuffer_d3d9::GetSurface() const
{
	return m_Surface;
}

VideoDriverD3D9::Rendertarget_d3d9::Rendertarget_d3d9() :
	m_Texture(nullptr),
	m_Surface(nullptr)
{
}

VideoDriverD3D9::Rendertarget_d3d9::Rendertarget_d3d9(Texture* texture) :
	m_Texture(texture),
	m_Surface(nullptr)
{
	if(m_Texture) {
		lxAssert(m_Texture->IsRendertarget());
		m_Size = m_Texture->GetDimension();
		IDirect3DTexture9* d3dTexture = (IDirect3DTexture9*)texture->GetRealTexture();
		if(FAILED(d3dTexture->GetSurfaceLevel(0, &m_Surface))) {
			m_Texture = nullptr;
			m_Surface = nullptr;
			m_Size.Set(0, 0);
		}
	}
}

VideoDriverD3D9::Rendertarget_d3d9::Rendertarget_d3d9(IDirect3DSurface9* surface) :
	m_Texture(nullptr),
	m_Surface(surface)
{
	if(m_Surface) {
		D3DSURFACE_DESC desc;
		if(SUCCEEDED(m_Surface->GetDesc(&desc))) {
			lxAssert(desc.Usage == D3DUSAGE_RENDERTARGET);
			m_Size.width = desc.Width;
			m_Size.height = desc.Height;
		}
	}
}

const math::dimension2du& VideoDriverD3D9::Rendertarget_d3d9::GetSize() const
{
	return m_Size;
}

StrongRef<Texture> VideoDriverD3D9::Rendertarget_d3d9::GetTexture()
{
	return m_Texture;
}

IDirect3DSurface9* VideoDriverD3D9::Rendertarget_d3d9::GetSurface()
{
	return m_Surface;
}

//////////////////////////////////////////////////////////////////////

VideoDriverD3D9::VideoDriverD3D9(core::Timer* timer, core::ReferableFactory* refFactory) :
	VideoDriverNull(timer, refFactory),
	m_HasStencilBuffer(false),
	m_3DTransformsChanged(false),
	m_2DTransformChanged(false),
	m_PresentResult(true),
	m_ResetRenderstates(true)
{
}

bool VideoDriverD3D9::Init(const DriverConfig& config, gui::Window* window)
{
	if(!VideoDriverNull::Init(config, window))
		return false;

	D3DPRESENT_PARAMETERS PresentParams;

	m_Adapter = 0;

	m_D3D = Direct3DCreate9(D3D_SDK_VERSION);
	if(!m_D3D) {
		log::Error("Couldn't create the direct3D9 interface.");
		return false;
	}

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
	D3DFORMAT FallbackFormat;
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
	if(m_Config.pureSoftware) {
		if(FAILED(m_D3D->CreateDevice(m_Adapter,
			D3DDEVTYPE_HAL,
			windowHandle,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&PresentParams,
			&m_D3DDevice))) {
			log::Error("Failed to create Direct3DDevice.");
			return false;
		}
	} else {
		HRESULT hr;

		hr = m_D3D->CreateDevice(m_Adapter,
			D3DDEVTYPE_HAL,
			windowHandle,
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&PresentParams,
			&m_D3DDevice);
		if(FAILED(hr))
			hr = m_D3D->CreateDevice(m_Adapter,
				D3DDEVTYPE_HAL,
				windowHandle,
				D3DCREATE_MIXED_VERTEXPROCESSING,
				&PresentParams,
				&m_D3DDevice);

		if(FAILED(hr))
			hr = m_D3D->CreateDevice(m_Adapter,
				D3DDEVTYPE_HAL,
				windowHandle,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING,
				&PresentParams,
				&m_D3DDevice);

		if(FAILED(hr)) {
			log::Error("Failed to create the direct3dDevice.");
			return false;
		}
	}

	// Print Information
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
	ResetAllRenderstates();

	SetVertexFormat(VertexFormat::STANDARD, true);
	InitRendertargetData();

	// Das Renderstate wird nicht direkt benutzt also setzten wir alle Känale auf Durchzug
	m_D3DDevice->SetRenderState(D3DRS_AMBIENT, 0xFFFFFFFF);

	m_Textures.Resize(m_Caps.MaxSimultaneousTextures);

	SetAmbient(0);
	SetFog();

	m_CurrentRendermode = ERM_NONE;

	m_BufferManager = LUX_NEW(BufferManagerD3D9)(this);

	m_RefFactory->RegisterType(LUX_NEW(TextureD3D9)(m_D3DDevice));
	m_RefFactory->RegisterType(LUX_NEW(CubeTextureD3D9)(m_D3DDevice));

	return true;
}

VideoDriverD3D9::~VideoDriverD3D9()
{
	if(m_D3DDevice) {
		IdentityMaterial.SetRenderer(nullptr);
		WorkMaterial.SetRenderer(nullptr);

		m_BufferManager = nullptr;

		m_VertexFormats.Clear();
		m_DepthBuffers.Clear();

		m_CurrentRendertarget = Rendertarget_d3d9();

		m_D3DDevice->Release();
	}

	if(m_D3D)
		m_D3D->Release();
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
}

bool VideoDriverD3D9::InitRendertargetData()
{
	IDirect3DSurface9* backbuffer = nullptr;
	if(FAILED(m_D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer)))
		return false;
	m_CurrentRendertarget = Rendertarget_d3d9(backbuffer);

	IDirect3DSurface9* depthStencilBuffer = nullptr;
	if(FAILED(m_D3DDevice->GetDepthStencilSurface(&depthStencilBuffer)))
		return false;
	DepthBuffer_d3d9 depthBuffer(depthStencilBuffer);
	m_DepthBuffers.Push_Back(depthBuffer);

	return true;
}

IDirect3DSurface9* VideoDriverD3D9::GetMatchingDepthBuffer(IDirect3DSurface9* target)
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
		m_DepthBuffers.Push_Back(buffer);
		return depthStencil;
	}

	return nullptr;
}

bool VideoDriverD3D9::SetRendertarget(Texture* texture)
{
	if(texture && !texture->IsRendertarget()) {
		log::Error("Tried to set a non rendertarget texture as rendertarget.");
		return false;
	}

	if(m_CurrentRendertarget.GetTexture() == texture)
		return true;

	Rendertarget_d3d9 newRendertarget;
	bool result = true;
	if(texture == nullptr) {
		if(m_PreviousRendertarget)
			newRendertarget = Rendertarget_d3d9(m_PreviousRendertarget);
		else
			newRendertarget = Rendertarget_d3d9(m_PreviousRendertargetSurface);
	} else {
		newRendertarget = Rendertarget_d3d9(texture);
	}

	if(FAILED(m_D3DDevice->SetRenderTarget(0, newRendertarget.GetSurface()))) {
		result = false;
		log::Error("Can't set new rendertarget texture.");
	}

	IDirect3DSurface9* depthStencil = nullptr;
	if(result) {
		depthStencil = GetMatchingDepthBuffer(newRendertarget.GetSurface());
		if(depthStencil == nullptr) {
			result = false;
			log::Error("Can't find matching depth buffer for rendertarget.");
		}
	}

	if(result) {
		if(FAILED(m_D3DDevice->SetDepthStencilSurface(depthStencil))) {
			m_D3DDevice->SetRenderTarget(0, m_CurrentRendertarget.GetSurface());
			result = false;
			log::Error("Can't enable new depth-stencil buffer.");
		}
	}

	if(result) {
		m_PreviousRendertarget = m_CurrentRendertarget.GetTexture().GetWeak();
		m_PreviousRendertargetSurface = m_CurrentRendertarget.GetSurface();
		m_CurrentRendertarget = newRendertarget;

		m_3DTransformsChanged = true;
		m_2DTransformChanged = true;
	}

	return result;
}

Texture* VideoDriverD3D9::GetRendertarget()
{
	return m_CurrentRendertarget.GetTexture();
}

bool VideoDriverD3D9::Present()
{
	HRESULT hr = m_D3DDevice->Present(NULL, NULL, NULL, NULL);
	if(FAILED(hr))
		log::Error("Present failed.");

	m_PresentResult = SUCCEEDED(hr);

	return m_PresentResult;
}

bool VideoDriverD3D9::BeginScene(bool clearColor, bool clearZBuffer)
{
	u32 flags = 0;
	if(clearColor)
		flags = D3DCLEAR_TARGET;
	if(clearZBuffer)
		flags |= D3DCLEAR_ZBUFFER;
	if(m_HasStencilBuffer)
		flags |= D3DCLEAR_STENCIL;

	HRESULT hr = S_OK;

	if(flags) {
		const D3DCOLOR d3dClear = (u32)m_ClearColor;
		if(FAILED(hr = m_D3DDevice->Clear(
			0, nullptr,
			flags,
			d3dClear, m_ClearDepth, 0))) {
			log::Error("Couldn't clean the backbuffer before beginning a scene.");
		}
	}

	if(FAILED(hr) || FAILED(hr = m_D3DDevice->BeginScene()))
		log::Error("Couldn't begin a new scene.");

	return SUCCEEDED(hr);
}

bool VideoDriverD3D9::EndScene()
{
	if(FAILED(m_D3DDevice->EndScene())) {
		log::Error("Couldn't finish scene.");
		return false;
	}

	m_RenderStatistics->RegisterFrame();

	return true;
}

bool VideoDriverD3D9::AddLight(const LightData& light)
{
	if(!VideoDriverNull::AddLight(light))
		return false;

	DWORD lightId = (DWORD)GetLightCount() - 1;

	D3DLIGHT9 D3DLight;

	switch(light.type) {
	case LightData::EType::Point:
		D3DLight.Type = D3DLIGHT_POINT;
		break;
	case LightData::EType::Spot:
		D3DLight.Type = D3DLIGHT_SPOT;
		break;
	case LightData::EType::Directional:
		D3DLight.Type = D3DLIGHT_DIRECTIONAL;
		break;
	}

	D3DLight.Position = *((D3DVECTOR*)(&light.position));
	D3DLight.Direction = *((D3DVECTOR*)(&light.direction));

	D3DLight.Range = math::Max(0.0f, light.range);
	D3DLight.Falloff = light.falloff;

	D3DCOLORVALUE Specular = {1.0f, 1.0f, 1.0f, 1.0f};
	D3DCOLORVALUE Ambient = {0.0f, 0.0f, 0.0f, 0.0f};
	D3DLight.Diffuse = SColorToD3DColor(light.color);
	D3DLight.Specular = Specular;
	D3DLight.Ambient = Ambient;

	D3DLight.Attenuation0 = 0.0f;
	D3DLight.Attenuation1 = 0.0f;
	D3DLight.Attenuation2 = 1.0f / light.range;

	D3DLight.Theta = light.innerCone * 2.0f;
	D3DLight.Phi = light.outerCone * 2.0f;

	if(SUCCEEDED(m_D3DDevice->SetLight(lightId, &D3DLight))) {
		if(FAILED(m_D3DDevice->LightEnable(lightId, TRUE)))
			return false;
		else
			return true;
	}

	return false;
}

void VideoDriverD3D9::ClearLights()
{
	for(size_t i = 0; i < GetLightCount(); ++i)
		m_D3DDevice->LightEnable((DWORD)i, FALSE);

	VideoDriverNull::ClearLights();
}

StrongRef<SubMesh> VideoDriverD3D9::CreateSubMesh(
	const VertexFormat& vertexFormat, EHardwareBufferMapping VertexHWMapping, u32 vertexCount,
	EIndexFormat indexType, EHardwareBufferMapping IndexHWMapping, u32 IndexCount,
	EPrimitiveType primitiveType)
{
	StrongRef<SubMesh> out = LUX_NEW(SubMeshImpl);
	StrongRef<IndexBuffer> ib = m_BufferManager->CreateIndexBuffer();
	if(!ib)
		return nullptr;
	ib->SetType(indexType);
	ib->SetHWMapping(IndexHWMapping);
	ib->SetSize(IndexCount);

	StrongRef<VertexBuffer> vb = m_BufferManager->CreateVertexBuffer();
	if(!vb)
		return nullptr;
	vb->SetFormat(vertexFormat);
	vb->SetHWMapping(VertexHWMapping);
	vb->SetSize(vertexCount);

	out->SetIndices(ib);
	out->SetVertices(vb);
	out->SetPrimitiveType(primitiveType);

	return out;
}

StrongRef<SubMesh> VideoDriverD3D9::CreateSubMesh(const VertexFormat& vertexFormat,
	bool Dynamic,
	EPrimitiveType primitiveType,
	u32 primitiveCount)
{
	LUX_UNUSED(primitiveCount);
	return CreateSubMesh(vertexFormat, Dynamic ? EHardwareBufferMapping::Dynamic : EHardwareBufferMapping::Static, 0,
		EIndexFormat::Bit16, EHardwareBufferMapping::Static, 0,
		primitiveType);
}

void VideoDriverD3D9::DrawSubMesh(const SubMesh* subMesh, u32 primitveCount)
{
	if(!subMesh)
		return;

	m_BufferManager->EnableBuffer(subMesh->GetVertices());
	m_BufferManager->EnableBuffer(subMesh->GetIndices());

	if(primitveCount == -1)
		primitveCount = subMesh->GetPrimitveCount();

	if(primitveCount == 0)
		return;

	BufferManagerD3D9* d3d9Manager = (BufferManagerD3D9*)(BufferManager*)m_BufferManager;

	BufferManagerD3D9::VertexStream vs;
	BufferManagerD3D9::IndexStream is;
	d3d9Manager->GetVertexStream(0, vs);
	d3d9Manager->GetIndexStream(is);

	const EPrimitiveType pt = subMesh->GetPrimitiveType();
	const u32 vertexCount = subMesh->GetVertexCount();
	const VertexFormat& vertexFormat = subMesh->GetVertexFormat();
	const EIndexFormat indexType = subMesh->GetIndices() ?
		subMesh->GetIndexType() :
		EIndexFormat::Bit16;

	Draw3DPrimitiveList(pt,
		primitveCount,
		vs.data,
		vertexCount,
		vertexFormat,
		is.data,
		indexType);
}

bool VideoDriverD3D9::Draw3DPrimitiveList(EPrimitiveType primitiveType,
	u32 primitveCount,
	const void* vertices,
	u32 vertexCount,
	const VertexFormat& vertexFormat,
	const void* indices,
	EIndexFormat indexType)
{
	return DrawPrimitiveList(primitiveType, primitveCount, vertices, vertexCount, vertexFormat, indices, indexType, true);
}

bool VideoDriverD3D9::Draw2DPrimitiveList(EPrimitiveType primitiveType,
	u32 primitveCount,
	const void* vertices,
	u32 vertexCount,
	const VertexFormat& vertexFormat,
	const void* indices,
	EIndexFormat indexType)
{
	return DrawPrimitiveList(primitiveType, primitveCount, vertices, vertexCount, vertexFormat, indices, indexType, false);
}

bool VideoDriverD3D9::DrawPrimitiveList(EPrimitiveType primitiveType,
	u32 primitveCount,
	const void* vertices,
	u32 vertexCount,
	const VertexFormat& vertexFormat,
	const void* indices,
	EIndexFormat indexType,
	bool is3D)
{
	// Vertextyp setzen und Größe speichern
	SetVertexFormat(vertexFormat);
	const u32 stride = vertexFormat.GetStride(0);

	// Indexformat speichern
	D3DFORMAT IndexFormat = D3DFMT_UNKNOWN;
	u32 indexSize;
	switch(indexType) {
	case EIndexFormat::Bit16:
		IndexFormat = D3DFMT_INDEX16;
		indexSize = 2;
		break;
	case EIndexFormat::Bit32:
		IndexFormat = D3DFMT_INDEX32;
		indexSize = 4;
		break;
	default:
		// Abbrechen
		return false;
	}

	// material aktivieren
	if(is3D) {
		SetRenderstates3DMode();
	} else {
		SetRenderstates2DMode();
	}

	// Materialrenderer beginnt zu Zeichnen
	if(m_CurrentRendermode == ERM_3D) {
		if(m_CurrentMaterial.GetRenderer()) {
			if(!m_CurrentMaterial.GetRenderer()->OnRender(0))
				return true;
		}
	}

	BufferManagerD3D9* d3d9Manager = (BufferManagerD3D9*)(BufferManager*)m_BufferManager;

	BufferManagerD3D9::VertexStream vs;
	BufferManagerD3D9::IndexStream is;

	u32 vertexOffset = 0;
	u32 indexOffset = 0;
	if(d3d9Manager->GetVertexStream(0, vs))
		vertexOffset = vs.offset;

	if(d3d9Manager->GetIndexStream(is))
		indexOffset = is.offset;

	u32 pass = 1;
	do {
		// Je nach Primitive zeichnen
		HRESULT hr = S_OK;
		float fTemp;
		switch(primitiveType) {
		case EPT_POINT_SPRITES:
		case EPT_POINTS:

			// Renderstates setzen
			if(primitiveType == EPT_POINT_SPRITES) {
				m_D3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
			}
			m_D3DDevice->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);
			fTemp = m_CurrentPipeline.Thickness / GetScreenSize().width;
			m_D3DDevice->SetRenderState(D3DRS_POINTSIZE, *reinterpret_cast<u32*>(&fTemp));
			fTemp = 1.0f;
			m_D3DDevice->SetRenderState(D3DRS_POINTSCALE_A, *reinterpret_cast<u32*>(&fTemp));
			m_D3DDevice->SetRenderState(D3DRS_POINTSCALE_B, *reinterpret_cast<u32*>(&fTemp));
			m_D3DDevice->SetRenderState(D3DRS_POINTSIZE_MIN, *reinterpret_cast<u32*>(&fTemp));
			fTemp = 0.0f;
			m_D3DDevice->SetRenderState(D3DRS_POINTSCALE_C, *reinterpret_cast<u32*>(&fTemp));

			if(!vertices) {
				// Aus Hardware zeichnen
				hr = m_D3DDevice->DrawPrimitive(D3DPT_POINTLIST, vertexOffset, primitveCount);
			} else {
				// Aus Software zeichnen
				hr = m_D3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, primitveCount, (const u8*)vertices + vertexOffset*stride, stride);
			}

			// Renderstates löschen
			m_D3DDevice->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
			if(primitiveType == EPT_POINT_SPRITES) {
				m_D3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
			}

			break;

		case EPT_LINE_STRIP:
		case EPT_LINES:
		case EPT_TRIANGLE_STRIP:
		case EPT_TRIANGLE_FAN:
		case EPT_TRIANGLES:
		{
			static D3DPRIMITIVETYPE Types[] = {D3DPT_LINESTRIP, D3DPT_LINELIST, D3DPT_TRIANGLESTRIP, D3DPT_TRIANGLEFAN, D3DPT_TRIANGLELIST};

			if(!vertices) {
				// Aus Hardware zeichnen
				hr = m_D3DDevice->DrawIndexedPrimitive(Types[primitiveType - 1], 0, 0, vertexCount, indexOffset, primitveCount);
			} else if(indices) {
				// Aus Software zeichnen
				hr = m_D3DDevice->DrawIndexedPrimitiveUP(Types[primitiveType - 1], 0, vertexCount, primitveCount,
					(const u8*)indices + indexSize*indexOffset, IndexFormat, (const u8*)vertices + vertexOffset*stride, stride);
			} else {
				// Aus Software zeichnen
				hr = m_D3DDevice->DrawPrimitiveUP(Types[primitiveType - 1], primitveCount, (const u8*)vertices + vertexOffset*stride, stride);
			}
		}
		break;
		}

		if(FAILED(hr)) {
			log::Error("Error while drawing.");
			return false;
		}

		m_RenderStatistics->AddPrimitves(primitveCount);
	} while(m_CurrentMaterial.GetRenderer()->OnRender(pass++));

	return true;
}

bool VideoDriverD3D9::Draw3DLine(const math::vector3f& start,
	const math::vector3f& end,
	Color colorStart,
	Color colorEnd,
	bool disableZ)
{
	SetVertexFormat(VertexFormat::STANDARD);
	Vertex3D Vertices[2];
	Vertices[0].position = start;
	Vertices[0].color = colorStart;
	Vertices[1].position = end;
	Vertices[1].color = colorEnd;

	SetRenderstates3DMode(false);
	if(m_CurrentPipeline.Lighting)
		m_D3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	if(m_CurrentPipeline.FogEnabled)
		m_D3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	if(disableZ)
		m_D3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	if(colorStart.HasAlpha() || colorEnd.HasAlpha()) {
		m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		m_D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_D3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_D3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	}
	m_CurrentPipeline.Lighting = false;
	m_CurrentPipeline.FogEnabled = false;

	// Zeichnen
	if(FAILED(m_D3DDevice->DrawPrimitiveUP(D3DPT_LINELIST,
		1,
		Vertices,
		sizeof(Vertex3D)))) {
		return false;
	}

	if(disableZ)
		m_D3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	if(colorStart.HasAlpha() || colorEnd.HasAlpha()) {
		m_D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}

	return true;
}

bool VideoDriverD3D9::Draw3DBox(const math::aabbox3df& box, Color color)
{
	// Vertizes und Indizes erstellen
	SetVertexFormat(VertexFormat::STANDARD);

	Vertex3D Vertices[8];
	math::vector3f Edges[8];
	box.GetCorners(Edges);
	Vertices[0].position = Edges[0];
	Vertices[0].color = color;
	Vertices[1].position = Edges[1];
	Vertices[1].color = color;
	Vertices[2].position = Edges[2];
	Vertices[2].color = color;
	Vertices[3].position = Edges[3];
	Vertices[3].color = color;
	Vertices[4].position = Edges[4];
	Vertices[4].color = color;
	Vertices[5].position = Edges[5];
	Vertices[5].color = color;
	Vertices[6].position = Edges[6];
	Vertices[6].color = color;
	Vertices[7].position = Edges[7];
	Vertices[7].color = color;

	u16 Indices[24] = {0, 4, 1, 5, 3, 7, 2, 6, 0, 1, 4, 5, 6, 7, 2, 3, 1, 3, 5, 7, 0, 2, 4, 6};

	SetRenderstates3DMode(false);
	if(m_CurrentPipeline.Lighting)
		m_D3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	if(m_CurrentPipeline.FogEnabled)
		m_D3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	m_CurrentPipeline.Lighting = false;
	m_CurrentPipeline.FogEnabled = false;

	// Zeichnen
	if(FAILED(m_D3DDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST,
		0,
		8,
		12,
		Indices,
		D3DFMT_INDEX16,
		Vertices,
		sizeof(Vertex3D)))) {
		return false;
	}

	return true;
}

bool VideoDriverD3D9::Draw2DImage(Texture* texture, const math::vector2i& position, const math::recti* clip, const video::Material* material)
{
	return Draw2DImage(texture, math::recti(position.x, position.y, position.x + texture->GetDimension().width, position.y + texture->GetDimension().height),
		math::rectf(0.0f, 0.0f, 1.0f, 1.0f), Color::White, false, clip, material);
}

bool VideoDriverD3D9::Draw2DImage(Texture* texture,
	const math::recti& DstRect,
	const math::rectf& SrcRect,
	Color color, bool UseAlpha,
	const math::recti* clip,
	const video::Material* material)
{
	if(UseAlpha)
		log::Warning("VideoDriverD3D9::Draw2DImage UseAlpha is not implemented.");

	if(material) {
		Set2DMaterial(*material);
		SetRenderstates2DMode(true);
	} else {
		SetRenderstates2DMode(false);
		SetActiveTexture(0, texture);
	}

	SetVertexFormat(VertexFormat::STANDARD_2D);

	if(clip) {
		m_D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
		RECT Scissor;
		Scissor.left = clip->Left;
		Scissor.right = clip->Right;
		Scissor.bottom = clip->Bottom;
		Scissor.top = clip->Top;
		m_D3DDevice->SetScissorRect(&Scissor);
	}

	Vertex2D Vertices[4];
	// 01
	// 23
	Vertices[0].position.Set((float)DstRect.Left, (float)DstRect.Top);
	Vertices[0].texture.Set(SrcRect.Left, SrcRect.Top);
	Vertices[0].color = color;
	Vertices[1].position.Set((float)DstRect.Right, (float)DstRect.Top);
	Vertices[1].texture.Set(SrcRect.Right, SrcRect.Top);
	Vertices[1].color = color;
	Vertices[2].position.Set((float)DstRect.Left, (float)DstRect.Bottom);
	Vertices[2].texture.Set(SrcRect.Left, SrcRect.Bottom);
	Vertices[2].color = color;
	Vertices[3].position.Set((float)DstRect.Right, (float)DstRect.Bottom);
	Vertices[3].texture.Set(SrcRect.Right, SrcRect.Bottom);
	Vertices[3].color = color;

	if(FAILED(m_D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, Vertices, sizeof(Vertex2D)))) {
		return false;
	}

	if(clip) {
		m_D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	}
	return true;
}

bool VideoDriverD3D9::Draw2DRectangle(const math::recti& rect, Color color,
	const math::recti* clip, const video::Material* material)
{
	if(material) {
		Set2DMaterial(*material);
		SetRenderstates2DMode(true);
	} else {
		SetActiveTexture(0, nullptr);
		SetRenderstates2DMode(false);
	}

	SetVertexFormat(VertexFormat::STANDARD_2D);

	if(clip) {
		m_D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
		RECT Scissor;
		Scissor.left = clip->Left;
		Scissor.right = clip->Right;
		Scissor.bottom = clip->Bottom;
		Scissor.top = clip->Top;
		m_D3DDevice->SetScissorRect(&Scissor);
	}

	Vertex2D Vertices[4];
	// 01
	// 23
	Vertices[0].position.Set((float)rect.Left, (float)rect.Top);
	Vertices[0].texture.Set(0.0f, 0.0f);
	Vertices[0].color = color;
	Vertices[1].position.Set((float)rect.Right, (float)rect.Top);
	Vertices[1].texture.Set(1.0f, 0.0f);
	Vertices[1].color = color;
	Vertices[2].position.Set((float)rect.Left, (float)rect.Bottom);
	Vertices[2].texture.Set(0.0f, 1.0f);
	Vertices[2].color = color;
	Vertices[3].position.Set((float)rect.Right, (float)rect.Bottom);
	Vertices[3].texture.Set(1.0f, 1.0f);
	Vertices[3].color = color;

	if(FAILED(m_D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, Vertices, sizeof(Vertex2D)))) {
		return false;
	}

	if(clip) {
		m_D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	}

	return true;
}

bool VideoDriverD3D9::Draw2DRectangle(const math::recti& rect,
	Color LeftUpColor, Color RightUpColor,
	Color LeftDownColor, Color RightDownColor,
	const math::recti* clip,
	const video::Material* material)
{
	if(material) {
		Set2DMaterial(*material);
		SetRenderstates2DMode(true);
	} else {
		SetActiveTexture(0, nullptr);
		SetRenderstates2DMode(false);
	}

	SetVertexFormat(VertexFormat::STANDARD_2D);

	if(clip) {
		m_D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
		RECT Scissor;
		Scissor.left = clip->Left;
		Scissor.right = clip->Right;
		Scissor.bottom = clip->Bottom;
		Scissor.top = clip->Top;
		m_D3DDevice->SetScissorRect(&Scissor);
	}

	Vertex2D Vertices[4];
	// 01
	// 23
	Vertices[0].position.Set((float)rect.Left, (float)rect.Top);
	Vertices[0].texture.Set(0.0f, 0.0f);
	Vertices[0].color = LeftUpColor;
	Vertices[1].position.Set((float)rect.Right, (float)rect.Top);
	Vertices[1].texture.Set(1.0f, 0.0f);
	Vertices[1].color = RightUpColor;
	Vertices[2].position.Set((float)rect.Left, (float)rect.Bottom);
	Vertices[2].texture.Set(0.0f, 1.0f);
	Vertices[2].color = LeftDownColor;
	Vertices[3].position.Set((float)rect.Right, (float)rect.Bottom);
	Vertices[3].texture.Set(1.0f, 1.0f);
	Vertices[3].color = RightDownColor;

	if(FAILED(m_D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, Vertices, sizeof(Vertex2D))))
		return false;

	if(clip)
		m_D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	return true;
}

bool VideoDriverD3D9::Draw2DLine(const math::vector2i& start, const math::vector2i& end,
	Color colorStart, Color colorEnd, const math::recti* clip)
{
	if(clip)
		log::Warning("VideoDriverD3D9::Draw2DLine clip is not implemented.");

	SetVertexFormat(VertexFormat::STANDARD_2D);

	Vertex2D Vertices[2];
	Vertices[0].position.Set((float)start.x, (float)start.y);
	Vertices[0].color = colorStart;
	Vertices[1].position.Set((float)end.x, (float)end.y);
	Vertices[1].color = colorEnd;

	SetRenderstates2DMode(false);
	if(m_CurrentPipeline.Lighting)
		m_D3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	if(m_CurrentPipeline.FogEnabled)
		m_D3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	m_CurrentPipeline.Lighting = false;
	m_CurrentPipeline.FogEnabled = false;

	// Zeichnen
	if(FAILED(m_D3DDevice->DrawPrimitiveUP(D3DPT_LINELIST,
		1,
		Vertices,
		sizeof(Vertex2D)))) {
		return false;
	}

	return true;
}

bool VideoDriverD3D9::CheckTextureFormat(ColorFormat format, bool alpha, bool cube)
{
	D3DFORMAT d3dFormat = GetD3DFormat(format, alpha);

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

StrongRef<Texture> VideoDriverD3D9::CreateTexture(const math::dimension2du& Size, ColorFormat Format, u32 MipCount, bool Alpha, bool isDynamic)
{
	StrongRef<TextureD3D9> out = LUX_NEW(TextureD3D9)(m_D3DDevice);
	if(!out->Init(Size, Format, MipCount, false, isDynamic))
		out = nullptr;

	return out;
}

StrongRef<Texture> VideoDriverD3D9::CreateRendertargetTexture(const math::dimension2du& size, ColorFormat format, bool alpha)
{
	StrongRef<TextureD3D9> out = LUX_NEW(TextureD3D9)(m_D3DDevice);
	if(!out->Init(size, format, 1, true, false))
		out = nullptr;

	return out;
}

StrongRef<CubeTexture> VideoDriverD3D9::CreateCubeTexture(u32 Size, ColorFormat Format, bool Alpha, bool isDynamic)
{
	StrongRef<CubeTextureD3D9> out = LUX_NEW(CubeTextureD3D9)(m_D3DDevice);
	if(!out->Init(Size, Format, isDynamic))
		out = nullptr;

	return out;
}

StrongRef<Shader> VideoDriverD3D9::CreateShader(
	EShaderLanguage language,
	const char* VSCode, const char* VSEntryPoint, u32 VSLength, int VSmajorVersion, int VSminorVersion,
	const char* PSCode, const char* PSEntryPoint, u32 PSLength, int PSmajorVersion, int PSminorVersion)
{
	if(language != EShaderLanguage::HLSL) {
		log::Error("Direct3D9 video driver only supports HLSL shaders.");
		return nullptr;
	}

	const char* vsProfile = GetD3DXShaderProfile(false, VSmajorVersion, VSminorVersion);
	const char* psProfile = GetD3DXShaderProfile(true, PSmajorVersion, PSminorVersion);

	if(!vsProfile) {
		log::Error("Invalid vertex shader profile(~d.~d).", VSmajorVersion, VSminorVersion);
		return nullptr;
	}

	if(!psProfile) {
		log::Error("Invalid pixel shader profile(~d.~d).", PSmajorVersion, PSminorVersion);
		return nullptr;
	}

	StrongRef<ShaderD3D9> out = LUX_NEW(ShaderD3D9)(this);
	if(!out->Init(VSCode, VSEntryPoint, VSLength, vsProfile, PSCode, PSEntryPoint, PSLength, psProfile))
		out = nullptr;

	return out;
}

void VideoDriverD3D9::SetActiveTexture(u32 stage,
	BaseTexture* texture)
{
	if(stage >= m_Caps.MaxSimultaneousTextures) {
		log::Error("Invalid texture stage.");
		return;
	}

	if(m_Textures[stage] != texture) {
		if(texture) {
			if(SUCCEEDED(m_D3DDevice->SetTexture(stage, (IDirect3DBaseTexture9*)(texture->GetRealTexture()))))
				m_Textures[stage] = texture;
		} else {
			m_D3DDevice->SetTexture(stage, nullptr);
			m_Textures[stage] = nullptr;
		}
	}
}

BaseTexture* VideoDriverD3D9::GetActiveTexture(u32 stage) const
{
	if(stage >= m_Caps.MaxSimultaneousTextures) {
		log::Error("Invalid texture stage.");
		return nullptr;
	}

	return m_Textures[stage];
}

void VideoDriverD3D9::Set2DTransform(const math::matrix4& m)
{
	if(m_Use2DTransform) {
		m_D3DDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)(m.DataRowMajor()));
		m_SceneValues->SetMatrix(scene::SceneValues::MAT_WORLD, m);
		m_2DTransformChanged = true;
	}

	m_2DTranform = m;
}

void VideoDriverD3D9::Use2DTransform(bool Use)
{
	m_Use2DTransform = Use;
	m_2DTransformChanged = true;
}

void VideoDriverD3D9::SetTransform(ETransformState transform, const math::matrix4& m)
{
	switch(transform) {
	case ETS_VIEW:
		m_D3DDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)(m.DataRowMajor()));
		m_SceneValues->SetMatrix(scene::SceneValues::MAT_VIEW, m);
		break;
	case ETS_PROJECTION:
		m_D3DDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)(m.DataRowMajor()));
		m_SceneValues->SetMatrix(scene::SceneValues::MAT_PROJ, m);
		break;
	case ETS_WORLD:
		m_D3DDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)(m.DataRowMajor()));
		m_SceneValues->SetMatrix(scene::SceneValues::MAT_WORLD, m);
		break;
	default:
		return;
	}

	m_Transforms[transform] = m;
	m_3DTransformsChanged = true;
}

const math::matrix4& VideoDriverD3D9::GetTransform(ETransformState transform) const
{
	return m_Transforms[transform];
}

void VideoDriverD3D9::SetVertexFormat(const VertexFormat& vertexFormat, bool reset)
{
	if(vertexFormat != m_CurrentVertexFormat || reset) {
		VertexFormat_d3d9 vf;
		auto it = m_VertexFormats.Find(vertexFormat);
		if(it == m_VertexFormats.End()) {
			IDirect3DVertexDeclaration9* d3d = CreateVertexFormat(vertexFormat);
			vf = VertexFormat_d3d9(d3d);
			m_VertexFormats[vertexFormat] = vf;
		} else {
			vf = *it;
		}

		IDirect3DVertexDeclaration9* D3DDecl = vf.GetD3D();

		if(!D3DDecl || FAILED(m_D3DDevice->SetVertexDeclaration(D3DDecl))) {
			log::Error("Can't set vertex format.");
			return;
		}

		m_CurrentVertexFormat = vertexFormat;
	}
}

const VertexFormat& VideoDriverD3D9::GetVertexFormat() const
{
	return m_CurrentVertexFormat;
}

void VideoDriverD3D9::SetFog(Color color,
	EFogType fogType,
	float start,
	float end,
	float density,
	bool pixelFog,
	bool rangeFog)
{
	m_Fog.color = color;
	m_Fog.type = fogType;
	m_Fog.start = start;
	m_Fog.end = end;
	m_Fog.density = density;
	m_Fog.pixel = pixelFog;
	m_Fog.range = rangeFog;

	m_Fog.isDirty = true;
}

void VideoDriverD3D9::GetFog(Color* color,
	EFogType* fogType,
	float* start,
	float* end,
	float* density,
	bool* pixelFog,
	bool* rangeFog) const
{
	if(color)
		*color = m_Fog.color;
	if(fogType)
		*fogType = m_Fog.type;
	if(start)
		*start = m_Fog.start;
	if(end)
		*end = m_Fog.end;
	if(density)
		*density = m_Fog.density;
	if(pixelFog)
		*pixelFog = m_Fog.pixel;
	if(rangeFog)
		*rangeFog = m_Fog.range;
}

void VideoDriverD3D9::SetTextureLayer(const MaterialLayer& layer, u32 textureLayer, bool resetAll)
{
	SetActiveTexture(textureLayer, layer.texture);

	// Nur wenn eine Textur vorhanden ist
	if(layer.texture || resetAll) {
		// Der Aufbau von ETextureRepeat ist der gleich wie von D3DTEXTUREADDRESS
		m_D3DDevice->SetSamplerState(textureLayer, D3DSAMP_ADDRESSU, GetD3DRepeatMode(layer.repeat.u));
		m_D3DDevice->SetSamplerState(textureLayer, D3DSAMP_ADDRESSV, GetD3DRepeatMode(layer.repeat.v));

		u32 filterMag = GetD3DTextureFilter(m_CurrentPipeline.MagFilter);
		u32 filterMin = GetD3DTextureFilter(m_CurrentPipeline.MinFilter);
		u32 filterMip = m_CurrentPipeline.TrilinearFilter ? D3DTEXF_LINEAR : D3DTEXF_POINT;

		m_D3DDevice->SetSamplerState(textureLayer, D3DSAMP_MIPFILTER, filterMip);
		if(filterMag == D3DTEXF_ANISOTROPIC || filterMin == D3DTEXF_ANISOTROPIC) {
			u16 ani = m_CurrentPipeline.Anisotropic;
			if(ani == 0)
				ani = (u16)m_Caps.MaxAnisotropy;
			if(ani > m_Caps.MaxAnisotropy)
				ani = (u16)m_Caps.MaxAnisotropy;
			m_D3DDevice->SetSamplerState(textureLayer, D3DSAMP_MAXANISOTROPY, ani);
		}
		m_D3DDevice->SetSamplerState(textureLayer, D3DSAMP_MINFILTER, filterMin);
		m_D3DDevice->SetSamplerState(textureLayer, D3DSAMP_MAGFILTER, filterMag);
	}
}

void VideoDriverD3D9::PushPipelineOverwrite(const PipelineOverwrite& over)
{
	m_PipelineOverwrites.Push_Back(over);
}

void VideoDriverD3D9::PopPipelineOverwrite()
{
	m_PipelineOverwrites.Pop_Back();
}

void VideoDriverD3D9::EnablePipeline(const PipelineSettings& settings, bool resetAll)
{
	PipelineSettings pipeline = settings;
	for(auto it = m_PipelineOverwrites.First(); it != m_PipelineOverwrites.End(); ++it)
		it->ApplyTo(pipeline);

	// Beleuchtung
	if(m_CurrentPipeline.Lighting != pipeline.Lighting ||
		resetAll)
		m_D3DDevice->SetRenderState(D3DRS_LIGHTING, pipeline.Lighting);

	// Nebel
	if(m_CurrentPipeline.FogEnabled != pipeline.FogEnabled ||
		resetAll)
		m_D3DDevice->SetRenderState(D3DRS_FOGENABLE, pipeline.FogEnabled);

	// Z-Buffer verlgeich
	if(m_CurrentPipeline.ZBufferFunc != pipeline.ZBufferFunc ||
		resetAll) {
		m_D3DDevice->SetRenderState(D3DRS_ZFUNC, pipeline.ZBufferFunc);
	}

	// Z-Schreiberlaubnis
	if(m_CurrentPipeline.ZWriteEnabled != pipeline.ZWriteEnabled ||
		resetAll)
		m_D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, pipeline.ZWriteEnabled ? TRUE : FALSE);

	if(m_CurrentPipeline.NormalizeNormals != pipeline.NormalizeNormals ||
		resetAll)
		m_D3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS, pipeline.NormalizeNormals ? TRUE : FALSE);

	// Füllmodus
	if(m_CurrentPipeline.Wireframe != pipeline.Wireframe ||
		m_CurrentPipeline.Pointcloud != pipeline.Pointcloud ||
		resetAll) {
		u32 dwFlag = D3DFILL_SOLID;
		if(pipeline.Wireframe)
			dwFlag = D3DFILL_WIREFRAME;
		if(pipeline.Pointcloud)
			dwFlag = D3DFILL_POINT;

		m_D3DDevice->SetRenderState(D3DRS_FILLMODE, dwFlag);
	}

	// Shading
	if(m_CurrentPipeline.GouraudShading != pipeline.GouraudShading ||
		resetAll) {
		u32 dwFlag = D3DSHADE_FLAT;
		if(pipeline.GouraudShading)
			dwFlag = D3DSHADE_GOURAUD;

		m_D3DDevice->SetRenderState(D3DRS_SHADEMODE, dwFlag);
	}

	// Culling
	if(m_CurrentPipeline.BackfaceCulling != pipeline.BackfaceCulling ||
		m_CurrentPipeline.FrontfaceCulling != pipeline.FrontfaceCulling ||
		resetAll) {
		u32 dwFlag = D3DCULL_NONE;
		if(pipeline.BackfaceCulling)
			dwFlag = D3DCULL_CCW;
		else if(pipeline.FrontfaceCulling)
			dwFlag = D3DCULL_CW;
		m_D3DDevice->SetRenderState(D3DRS_CULLMODE, dwFlag);
	}

	// Dicke
	if(m_CurrentPipeline.Thickness != pipeline.Thickness ||
		resetAll) {
		float fTemp = pipeline.Thickness;
		m_D3DDevice->SetRenderState(D3DRS_POINTSIZE, *reinterpret_cast<u32*>(&fTemp));
	}

	// Alpha-Blending
	if(pipeline.Blending.Operator == EBO_NONE) {
		m_D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	} else {
		m_D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_D3DDevice->SetRenderState(D3DRS_DESTBLEND, GetD3DBlend(pipeline.Blending.DstBlend));
		m_D3DDevice->SetRenderState(D3DRS_SRCBLEND, GetD3DBlend(pipeline.Blending.SrcBlend));
		m_D3DDevice->SetRenderState(D3DRS_BLENDOP, GetD3DBlendFunc(pipeline.Blending.Operator));
	}

	// TODO: Own Rendersystem for fog
	if(m_Fog.isDirty) {
		m_Fog.isDirty = false;
		if(m_Fog.pixel) {
			// TODO: Use real type conversion, instead using the enum directly
			m_D3DDevice->SetRenderState(D3DRS_FOGTABLEMODE, m_Fog.type);
		} else {
			m_D3DDevice->SetRenderState(D3DRS_FOGVERTEXMODE, m_Fog.type);
			m_D3DDevice->SetRenderState(D3DRS_RANGEFOGENABLE, m_Fog.range);
		}

		m_D3DDevice->SetRenderState(D3DRS_FOGCOLOR, (u32)m_Fog.color);
		m_D3DDevice->SetRenderState(D3DRS_FOGSTART, *reinterpret_cast<u32*>(&m_Fog.start));
		m_D3DDevice->SetRenderState(D3DRS_FOGEND, *reinterpret_cast<u32*>(&m_Fog.end));
		m_D3DDevice->SetRenderState(D3DRS_FOGDENSITY, *reinterpret_cast<u32*>(&m_Fog.density));
	}

	m_CurrentPipeline = pipeline;
}

void VideoDriverD3D9::Set3DMaterial(const Material& material)
{
	m_3DMaterial = material;
	if(m_3DMaterial.GetRenderer() == nullptr)
		m_3DMaterial.SetRenderer(m_SolidRenderer);
}

const Material& VideoDriverD3D9::Get3DMaterial() const
{
	return m_3DMaterial;
}

void VideoDriverD3D9::Set2DMaterial(const Material& material)
{
	m_2DMaterial = material;

	if(m_2DMaterial.GetRenderer() == nullptr)
		m_2DMaterial.SetRenderer(m_SolidRenderer);
}

const Material& VideoDriverD3D9::Get2DMaterial() const
{
	return m_2DMaterial;
}

void VideoDriverD3D9::SetRenderstates3DMode(bool useMaterial)
{
	if(m_CurrentRendermode != ERM_3D || (m_2DTransformChanged && m_Use2DTransform)) {
		// Matrizen wiederherstellen
		m_D3DDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)(&m_Transforms[ETS_WORLD]));
		m_D3DDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)(&m_Transforms[ETS_VIEW]));
		m_D3DDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)(&m_Transforms[ETS_PROJECTION]));

		ResetAllRenderstates();
	}

	/*
	TODO: Hat hier eigentlich nichts zu suchen hab aber keine anderer Wahl
	*/
	const Colorf LastAmbient = m_CurrentMaterial.ambient * m_AmbientColor;
	const Colorf NewAmbient = m_3DMaterial.ambient * m_AmbientColor;
	if(m_CurrentMaterial.diffuse != m_3DMaterial.diffuse ||
		m_CurrentMaterial.shininess != m_3DMaterial.shininess ||
		LastAmbient != NewAmbient ||
		m_CurrentMaterial.specular != m_3DMaterial.specular ||
		m_CurrentMaterial.emissive != m_3DMaterial.emissive ||
		m_ResetRenderstates) {
		// Direct3D-material erstellen und einsetzen
		D3DMATERIAL9 D3DMaterial = {SColorToD3DColor(m_3DMaterial.diffuse),
			SColorToD3DColor(NewAmbient),
			SColorToD3DColor(m_3DMaterial.specular),
			SColorToD3DColor(m_3DMaterial.emissive),
			m_3DMaterial.shininess};
		m_D3DDevice->SetMaterial(&D3DMaterial);

		// Glanz nur wenn Beleuchtung aktiviert ist
		if(m_Pipeline.Lighting && !math::IsZero(m_3DMaterial.shininess))
			m_D3DDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
		else
			m_D3DDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	}

	if((m_ResetRenderstates || useMaterial) || m_CurrentMaterial != m_3DMaterial) {
		// Altes material deaktivieren
		if(m_CurrentMaterial.GetRenderer())
			m_CurrentMaterial.GetRenderer()->OnUnsetMaterial();

		// Neues material aktivieren
		m_3DMaterial.GetRenderer()->OnSetMaterial(m_3DMaterial, m_CurrentMaterial, m_ResetRenderstates);
		m_CurrentMaterial = m_3DMaterial;
	}

	if(m_CurrentMaterial.GetRenderer()->GetShader())
		m_CurrentMaterial.GetRenderer()->GetShader()->LoadSceneValues();

	m_CurrentRendermode = ERM_3D;
	m_ResetRenderstates = false;
}

void VideoDriverD3D9::SetRenderstates2DMode(bool useMaterial, bool Alpha, bool AlphaChannel)
{
	if(m_CurrentRendermode != ERM_2D || m_3DTransformsChanged || m_2DTransformChanged) {
		// Matrizen setzen
		math::matrix4 m;
		if(m_Use2DTransform)
			m_D3DDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)(m_2DTranform.DataRowMajor()));
		else
			m_D3DDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)(m.DataRowMajor()));
		/*
				m.SetTranslation(math::vector3f(-0.5f, -0.5f, 0.0f));
				m_D3DDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)(m.DataRowMajor()));
				m = math::matrix4(2.0f / m_CurrentRendertarget.GetSize().width, 0.0f, 0.0f, 0.0f,
					0.0f, -2.0f / m_CurrentRendertarget.GetSize().height, 0.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 0.0f,
					-1.0f, 1.0f, 0.0f, 1.0f);
					*/
		m = math::matrix4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			-(s32)m_CurrentRendertarget.GetSize().width / 2 - 0.5f, (s32)m_CurrentRendertarget.GetSize().height / 2 - 0.5f, 0.0f, 1.0f);
		m_D3DDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)(m.DataRowMajor()));
		m = math::matrix4(
			2.0f / m_CurrentRendertarget.GetSize().width, 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f / m_CurrentRendertarget.GetSize().height, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		m_D3DDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)(m.DataRowMajor()));

		m_3DTransformsChanged = false;
		m_2DTransformChanged = false;
	}

	if(m_CurrentMaterial.GetRenderer())
		m_CurrentMaterial.GetRenderer()->OnUnsetMaterial();

	if(useMaterial) {
		if(m_2DMaterial.GetRenderer())
			m_2DMaterial.GetRenderer()->OnSetMaterial(m_2DMaterial, m_CurrentMaterial);

		m_CurrentMaterial = m_2DMaterial;

		if(m_CurrentMaterial.GetRenderer()->GetShader())
			m_CurrentMaterial.GetRenderer()->GetShader()->LoadSceneValues();
	} else {
		m_D3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_D3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
		if(Alpha || AlphaChannel) {
			m_D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			m_D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_D3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		} else {
			m_D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}


		m_D3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

		if(AlphaChannel) {
			m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			if(Alpha) {
				m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			} else {
				m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			}
		} else {
			if(Alpha) {
				m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			}
		}
	}

	m_CurrentRendermode = ERM_2D;
	m_ResetRenderstates = false;
}

void VideoDriverD3D9::ResetAllRenderstates()
{
	m_ResetRenderstates = true;
}

IDirect3DVertexDeclaration9* VideoDriverD3D9::CreateVertexFormat(const VertexFormat& format)
{
	if(!format.IsValid()) {
		log::Error("Vertexformat \"~s\" is invalid.", format.GetName());
		return nullptr;
	}

	if(format.GetElement(0, VertexElement::EUsage::Position).sematic != VertexElement::EUsage::Position &&
		format.GetElement(0, VertexElement::EUsage::PositionNT).sematic != VertexElement::EUsage::PositionNT) {
		log::Error("Vertexformat \"~s\" is missing position usage.", format.GetName());
		return nullptr;
	}

	D3DVERTEXELEMENT9* d3dElements = LUX_NEW_ARRAY(D3DVERTEXELEMENT9, format.GetElemCount() + 1);

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

			if(error) {
				LUX_FREE_ARRAY(d3dElements);

				log::Error("Vertexformat \"~s\" has unknown vertex element in declaration.", format.GetName());
				return nullptr;
			}
		}
	}

	const u32 size = format.GetElemCount();
	d3dElements[size].Stream = 0xFF;
	d3dElements[size].Offset = 0;
	d3dElements[size].Type = D3DDECLTYPE_UNUSED;
	d3dElements[size].Method = 0;
	d3dElements[size].Usage = 0;
	d3dElements[size].UsageIndex = 0;

	// Deklaration erstellen
	IDirect3DVertexDeclaration9* d3dDecl = NULL;
	m_D3DDevice->CreateVertexDeclaration(d3dElements, &d3dDecl);
	LUX_FREE_ARRAY(d3dElements);

	if(!d3dDecl) {
		log::Error("Vertexformat \"~s\" can't be created.", format.GetName());
		return nullptr;
	}

	return d3dDecl;
}

}
}


#endif // LUX_COMPILE_WITH_D3D9
