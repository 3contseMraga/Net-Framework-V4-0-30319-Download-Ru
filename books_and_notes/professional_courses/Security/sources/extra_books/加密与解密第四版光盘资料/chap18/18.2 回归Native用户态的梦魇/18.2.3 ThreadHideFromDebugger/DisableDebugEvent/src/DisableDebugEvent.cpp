/*-----------------------------------------------------------------------
��18��  �����ټ���
code by forgot 
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/


#include <stdio.h> 

#include <windows.h>
#include <tchar.h>

typedef DWORD (WINAPI *ZW_SET_INFORMATION_THREAD)(HANDLE, DWORD, PVOID, ULONG);
#define ThreadHideFromDebugger 17
VOID DisableDebugEvent(VOID)
{
	HINSTANCE hModule;
	ZW_SET_INFORMATION_THREAD ZwSetInformationThread;

	hModule = GetModuleHandleA("Ntdll");
	ZwSetInformationThread = 
		(ZW_SET_INFORMATION_THREAD)GetProcAddress(hModule, "ZwSetInformationThread");
	ZwSetInformationThread(GetCurrentThread(), ThreadHideFromDebugger, NULL, NULL);
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
DisableDebugEvent();
     return 0 ;
}

