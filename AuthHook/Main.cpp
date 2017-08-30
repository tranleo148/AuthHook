#include "WinsockHax.h"
#include "ProudNetHax.h"
#include "WindowHax.h"
#include <stdio.h>
#include <intrin.h>

//By Rajan G

VOID WINAPI ShowConsole()
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());

	freopen("CON", "w", stdout);

	char cc[128];
	sprintf_s(cc, "Client: %i", GetCurrentProcessId());
	SetConsoleTitleA(cc);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,  DWORD fdwReason, LPVOID lpvReserved)
{
	DisableThreadLibraryCalls(hinstDLL);

	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		//ShowConsole();

		BOOL winsockRet = HaxWinsock();
		BOOL proudNetRet = HaxProudNet();
		BOOL windowRet = HaxWindow();

		if (winsockRet == FALSE)
		{
			MessageBox(0, "Internal Hooks Failed", 0, 0);
		}

		if (proudNetRet == FALSE)
		{
			MessageBox(0, "Service Hooks Failed", 0, 0);
		}

		if (windowRet == FALSE)
		{
			MessageBox(0, "Window Hooks Failed", 0, 0);
		}

		return winsockRet && proudNetRet && windowRet;
	}

	return FALSE;
}