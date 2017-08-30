#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include "Tools.h"
#include <stdio.h>
#include <WS2spi.h>
#include "WinsockHax.h"
#include "ThemidaSDK.h"

#define REPLACEPATTERN "8.31.99.14"
#define HOSTNAME "dfnwqi12.oa.to"

typedef int (WINAPI * pWSPStartup)(WORD wVersionRequested, LPWSPDATA lpWSPData, LPWSAPROTOCOL_INFO lpProtocolInfo, WSPUPCALLTABLE UpcallTable, LPWSPPROC_TABLE lpProcTable);

DWORD nexonServer;
DWORD userServer;

pWSPStartup _WSPStartup;
WSPPROC_TABLE procTable;

SOCKET workingSocket = INVALID_SOCKET;


int WINAPI WSPGetPeerName_detour(SOCKET s, struct sockaddr *name, LPINT namelen, LPINT lpErrno)
{
	int ret = procTable.lpWSPGetPeerName(s, name, namelen, lpErrno);

	sockaddr_in* service = (sockaddr_in*)name;

	DWORD tempServer = 0;
	memcpy(&tempServer, &service->sin_addr, sizeof(DWORD));

	if (tempServer == userServer)
	{
		memcpy(&service->sin_addr, &nexonServer, sizeof(DWORD));
	}

	return  ret;
}
int WINAPI WSPConnect_detour(SOCKET s, const struct sockaddr *name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, LPINT lpErrno)
{
	if (userServer == 0)
	{
		//Domain
		hostent *he = gethostbyname(HOSTNAME);

		if (he == NULL)
		{
			MessageBox(0, "Error resolving hostname", 0, 0);
			ExitProcess(0);
			return FALSE;
		}
		else
		{
			memcpy(&userServer, he->h_addr_list[0], he->h_length);
		}
	}

	char buf[50];
	DWORD len = 50;
	WSAAddressToString((sockaddr*)name, namelen, NULL, buf, &len);

	if (strstr(buf, REPLACEPATTERN))
	{
		sockaddr_in* service = (sockaddr_in*)name;

		//workingSocket = s;

		memcpy(&nexonServer, &service->sin_addr, sizeof(DWORD)); //sin_adder -> nexonServer
		memcpy(&service->sin_addr, &userServer, sizeof(DWORD)); //userServer -> sin_adder
	}

	return procTable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
}
int WINAPI WSPStartup_detour(WORD wVersionRequested, LPWSPDATA lpWSPData, LPWSAPROTOCOL_INFO lpProtocolInfo, WSPUPCALLTABLE UpcallTable, LPWSPPROC_TABLE lpProcTable)
{
	int ret = _WSPStartup(wVersionRequested, lpWSPData, lpProtocolInfo, UpcallTable, lpProcTable);
	procTable = *lpProcTable;

	lpProcTable->lpWSPConnect = WSPConnect_detour;
	lpProcTable->lpWSPGetPeerName = WSPGetPeerName_detour;

	return ret;
}

BOOL HaxWinsock()
{
	VM_START

	HMODULE module = LoadLibrary("MSWSOCK");

	if (!module)
		return FALSE;

	DWORD address = (DWORD)GetProcAddress(module, "WSPStartup");

	if (!address)
		return FALSE;

	_WSPStartup = (pWSPStartup)address;

	if (!SetHook(true, (PVOID*)&_WSPStartup, (PVOID)WSPStartup_detour))
	{
		return FALSE;
	}

	VM_END

		return TRUE;
}