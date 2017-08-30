#include "ProudNetHax.h"
#include "Tools.h"
#include <stdio.h>
#include <string>
#include "NMCO\NMGeneral.h"
#include "NMCO\NMFunctionObject.h"
#include "NMCO\NMSerializable.h"
#include "ThemidaSDK.h"

char* username = new char[PASSPORT_SIZE];

typedef BOOL(__cdecl* pNMCO_CallNMFunc)(int uFuncCode, BYTE* pCallingData, BYTE**ppReturnData, UINT32&	uReturnDataLen);
pNMCO_CallNMFunc NMCO_CallNMFunc;

void MessageBoxFormat(const char* format, ...)
{
	char* messageBoxText = new char[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(messageBoxText, 1023, format, args);

	MessageBoxA(NULL, messageBoxText, 0, 0);

	va_end(args);
}

BOOL NMCO_CallNMFunc_Hook(int uFuncCode, BYTE* pCallingData, BYTE**ppReturnData, UINT32&	uReturnDataLen)
{
	VM_START

	//CWvsApp::InitializeAuth
	if (uFuncCode == kNMFuncCode_SetLocale || uFuncCode == kNMFuncCode_Initialize) 
	{
		CNMSimpleStream* returnStream = new CNMSimpleStream(); // Memleaked actually. 
		CNMSetLocaleFunc* retFunc = new CNMSetLocaleFunc(); // Memleaked actually. 
		retFunc->SetReturn();
		retFunc->bSuccess = true;

		if (retFunc->Serialize(*returnStream) == false)
			MessageBoxA(NULL, "Could not Serialize?!", 0, 0);

		*ppReturnData = returnStream->GetBufferPtr();
		uReturnDataLen = returnStream->GetBufferSize();

		return TRUE;
	}
	else if (uFuncCode == kNMFuncCode_LoginAuth)
	{
		CNMSimpleStream	ssStream;
		ssStream.SetBuffer(pCallingData);

		CNMLoginAuthFunc pFunc;
		pFunc.SetCalling();
		pFunc.DeSerialize(ssStream);

		memcpy(username, pFunc.szNexonID, PASSPORT_SIZE);
		//printf("Username: %s\r\n", username);

		// Return to the client that login was successful.. NOT
		CNMSimpleStream* returnStream = new CNMSimpleStream(); // Memleaked actually. 
		CNMLoginAuthFunc* retFunc = new CNMLoginAuthFunc(); // Memleaked actually. 
		retFunc->SetReturn();
		retFunc->nErrorCode = kLoginAuth_OK;
		retFunc->bSuccess = true;

		if (retFunc->Serialize(*returnStream) == false)
			MessageBoxA(NULL, "Could not Serialize?!", 0, 0);

		*ppReturnData = returnStream->GetBufferPtr();
		uReturnDataLen = returnStream->GetBufferSize();

		return TRUE;
	}
	else if (uFuncCode == kNMFuncCode_GetNexonPassport)
	{
		CNMSimpleStream* ssStream = new CNMSimpleStream(); // Memleaked actually. 

		CNMGetNexonPassportFunc* pFunc = new CNMGetNexonPassportFunc(); // Memleaked actually. 
		pFunc->bSuccess = true;

		strcpy(pFunc->szNexonPassport, username);

		pFunc->SetReturn();

		if (pFunc->Serialize(*ssStream) == false)
			MessageBoxA(NULL, "Could not Serialize?!", 0, 0);

		*ppReturnData = ssStream->GetBufferPtr();
		uReturnDataLen = ssStream->GetBufferSize();

		return TRUE;
	}
	else if (uFuncCode == kNMFuncCode_LogoutAuth)
	{
		return TRUE;
	}


	MessageBoxFormat("Woops. Missing something: %x", uFuncCode);

	VM_END

	return NMCO_CallNMFunc(uFuncCode, pCallingData, ppReturnData, uReturnDataLen);
}

BOOL HaxProudNet()
{
	VM_START

	HMODULE module = LoadLibrary("nmcogame");

	if (!module)
		return FALSE;

	DWORD address = (DWORD)GetProcAddress(module, "NMCO_CallNMFunc");

	if (!address)
		return FALSE;

	NMCO_CallNMFunc = (pNMCO_CallNMFunc)address;

	if (!SetHook(true, (PVOID*)&NMCO_CallNMFunc, (PVOID)NMCO_CallNMFunc_Hook))
	{
		return FALSE;
	}

	VM_END

	return TRUE;
}