/*-----------------------------------------------------------------------
��18��  �����ټ���
code by forgot 
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/



#include <windows.h>
#include <stdio.h>

#define SystemKernelDebuggerInformation 35

#pragma pack(4)

typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION 
{ 
    BOOLEAN DebuggerEnabled; 
    BOOLEAN DebuggerNotPresent; 
} SYSTEM_KERNEL_DEBUGGER_INFORMATION, *PSYSTEM_KERNEL_DEBUGGER_INFORMATION;

typedef DWORD (WINAPI *ZW_QUERY_SYSTEM_INFORMATION)(DWORD, PVOID, ULONG, PULONG);

BOOL
CheckKernelDbgr(
	VOID)
{
	HINSTANCE hModule = GetModuleHandleA("Ntdll");
	ZW_QUERY_SYSTEM_INFORMATION ZwQuerySystemInformation = 
		(ZW_QUERY_SYSTEM_INFORMATION)GetProcAddress(hModule, "ZwQuerySystemInformation");
	SYSTEM_KERNEL_DEBUGGER_INFORMATION Info = {0};

	ZwQuerySystemInformation(
		SystemKernelDebuggerInformation,
		&Info, 
		sizeof(Info),
		NULL);

	return (Info.DebuggerEnabled && !Info.DebuggerNotPresent);
}
