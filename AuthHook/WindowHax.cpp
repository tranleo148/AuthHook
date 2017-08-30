#include "WindowHax.h"
#include "Tools.h"
#include <stdio.h>

#define WINDOWNAME "VaronMS"

typedef HWND(WINAPI* pCreateWindowEx)(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
pCreateWindowEx _CreateWindowEx;

HWND WINAPI CreateWindowEx_detour(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	if (!strcmp(lpClassName, "StartUpDlgClass") || !strcmp(lpClassName, "NexonADBallon"))
	{
		return NULL;
	}
	else if (!strcmp(lpClassName, "MapleStoryClass"))
	{
		return _CreateWindowEx(dwExStyle, lpClassName, WINDOWNAME, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}

	return _CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

BOOL HaxWindow()
{
	_CreateWindowEx = &CreateWindowExA;

	if (!SetHook(true, (PVOID*)&_CreateWindowEx, (PVOID)CreateWindowEx_detour))
	{
		return FALSE;
	}

	return TRUE;
}