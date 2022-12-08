#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "ItemFilter.h"
#include "Utils.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	switch (ul_reason_for_call) {

	case DLL_PROCESS_ATTACH:
/*
#ifdef _DEBUG
	    AllocConsole();
	    AttachConsole(GetCurrentProcessId());
	    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
	    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
	    freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);
#endif
*/
	    if (GameVersion() == D2Version::ERROR) {
		auto m = L"Could not determine the game version.";
		MessageBox(nullptr, m, L"Error", MB_OK | MB_ICONSTOP);
		exit(1);
	    }

	    // InitGame();
	    ItemFilter::Setup();
	    return TRUE;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	    break;

	case DLL_PROCESS_DETACH:
	    ItemFilter::RestoreHook();
	    break;
	}
	return TRUE;
}

