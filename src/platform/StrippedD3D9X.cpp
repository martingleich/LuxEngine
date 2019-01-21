#include "platform/StrippedD3D9X.h"
#include "core/Logger.h"

static D3DXLibraryLoader g_Loader;
D3DXLibraryLoader& D3DXLibraryLoader::Instance()
{
	return g_Loader;
}

D3DXLibraryLoader::~D3DXLibraryLoader()
{
}

#define LX_LOAD_DLL_FUNCTION(name) \
	m_##name = (name##T)GetProcAddress(m_Module, #name); \
	if(!m_##name) \
		goto FUNCTION_LOAD_FAILED;

bool D3DXLibraryLoader::Load()
{
	if(m_IsLoaded)
		return true;

	if(m_LoadFailed)
		return false;

	const char dllPath[] = "d3dx9_"  LX_STRINGIFY(D3DX_SDK_VERSION) ".dll";
	m_Module  = LoadLibraryA(dllPath);
	if(!m_Module) {
		m_LoadFailed = true;
		lux::log::Error("Failed to load {}.", dllPath);
		lux::log::Error("Is the DirectX-SDK installed?");
		return false;
	}

	LX_LOAD_DLL_FUNCTION(D3DXCompileShader);
	LX_LOAD_DLL_FUNCTION(D3DXFilterTexture);
	LX_LOAD_DLL_FUNCTION(D3DXGetImageInfoFromFileInMemory);
	LX_LOAD_DLL_FUNCTION(D3DXCreateTextureFromFileInMemoryEx);
	LX_LOAD_DLL_FUNCTION(D3DXCreateCubeTextureFromFileInMemoryEx);
	LX_LOAD_DLL_FUNCTION(D3DXCreateVolumeTextureFromFileInMemoryEx);

	return true;
FUNCTION_LOAD_FAILED:
	m_LoadFailed = true;
	lux::log::Error("Failed to load functions from {}.", dllPath);
	lux::log::Error("Is the DirectX-SDK installed?");
	return false;
}
#undef LX_LOAD_DLL_FUNCTION
