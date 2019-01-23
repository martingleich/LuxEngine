#ifndef INCLUDED_LUX_STRIPPED_D3D9X_H
#define INCLUDED_LUX_STRIPPED_D3D9X_H

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <d3d9\inc\d3dx9.h>

#undef CreateFile
#undef MoveFile
#undef CopyFile
#undef DeleteFile
#undef Min
#undef Max
#undef min
#undef max
#undef interface

#include "core/HelperMacros.h"
#include "core/HelperTemplates.h"

#define LX_DLL_FUNCTION(name) \
	using name##T = decltype(name)*; \
private: \
	name##T m_##name = nullptr; \
public: \
	name##T Get##name() { return Load() ? m_##name : nullptr; } \
public:

class D3DXLibraryLoader : lux::core::Uncopyable
{
public:
	LX_DLL_FUNCTION(D3DXCompileShader);
	LX_DLL_FUNCTION(D3DXFilterTexture);
	LX_DLL_FUNCTION(D3DXGetImageInfoFromFileInMemory);
	LX_DLL_FUNCTION(D3DXCreateTextureFromFileInMemoryEx);
	LX_DLL_FUNCTION(D3DXCreateCubeTextureFromFileInMemoryEx);
	LX_DLL_FUNCTION(D3DXCreateVolumeTextureFromFileInMemoryEx);

	static D3DXLibraryLoader& Instance();
	~D3DXLibraryLoader();

private:
	bool Load();

private:
	bool m_IsLoaded = false;
	bool m_LoadFailed = false;
	HMODULE m_Module=NULL;
};

#undef LX_DLL_FUNCTION
#endif
