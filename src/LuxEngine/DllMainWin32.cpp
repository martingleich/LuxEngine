#include "DllMainWin32.h"

static HINSTANCE g_Instance;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD dwReasonForCall,
	LPVOID pvReserved)
{
	LUX_UNUSED(dwReasonForCall);
	LUX_UNUSED(pvReserved);

	g_Instance = hModule;

	return TRUE;
}

namespace lux
{

HINSTANCE GetLuxModule()
{
	return g_Instance;
}

}
