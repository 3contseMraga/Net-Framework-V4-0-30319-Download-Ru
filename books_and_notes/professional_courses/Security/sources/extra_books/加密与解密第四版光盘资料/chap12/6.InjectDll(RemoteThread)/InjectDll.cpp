/*-----------------------------------------------------------------------
��12��  ע�뼼��
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// InjectDll.cpp : Defines the entry point for the console application.
// ��һ���Ѿ������еĽ�����Զ���̵߳ķ�ʽע��dll

#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h>

BOOL WINAPI InjectDllToProcess(DWORD dwTargetPid ,LPCTSTR DllPath );
DWORD ProcesstoPid(char *Processname);
BOOL EnableDebugPrivilege();

int main(int argc, char* argv[])
{
#ifdef _WIN64
	char szProcName[MAX_PATH] = "HostProc64.exe";
	char szDllPath[MAX_PATH] = "F:\\Program2016\\DllInjection\\MsgDll64.dll";
#else
	char szProcName[MAX_PATH] = "HostProc.exe";
	char szDllPath[MAX_PATH] = "F:\\Program2016\\DllInjection\\MsgDll.dll";
#endif
	
	DWORD dwPid = ProcesstoPid(szProcName);
	EnableDebugPrivilege();
	InjectDllToProcess(dwPid,szDllPath);
	return 0;
}

//ע��������
BOOL WINAPI InjectDllToProcess(DWORD dwTargetPid ,LPCTSTR DllPath )
{
    HANDLE hProc = NULL;
	hProc=OpenProcess(PROCESS_ALL_ACCESS,//Win7��Ҫ���Ȩ�޽ϸ�
		FALSE,
		dwTargetPid
		);

    if(hProc == NULL)
    {
        printf("[-] OpenProcess Failed.\n");
        return FALSE;
    }
	
    LPTSTR psLibFileRemote = NULL;
	
    //ʹ��VirtualAllocEx������Զ�̽��̵��ڴ��ַ�ռ����DLL�ļ�������
    psLibFileRemote=(LPTSTR)VirtualAllocEx(hProc, NULL, lstrlen(DllPath)+1,
		MEM_COMMIT, PAGE_READWRITE);
	
    if(psLibFileRemote == NULL)
    {
        printf("[-] VirtualAllocEx Failed.\n");
        return FALSE;
    }
	
    //ʹ��WriteProcessMemory������DLL��·�������Ƶ�Զ�̵��ڴ�ռ�
    if(WriteProcessMemory(hProc, psLibFileRemote, (void *)DllPath, lstrlen(DllPath)+1, NULL) == 0)
    {
        printf("[-] WriteProcessMemory Failed.\n");
        return FALSE;
    }
	
    //����LoadLibraryA����ڵ�ַ
    PTHREAD_START_ROUTINE pfnStartAddr=(PTHREAD_START_ROUTINE)
        GetProcAddress(GetModuleHandle("Kernel32"),"LoadLibraryA");
	
    if(pfnStartAddr == NULL)
    {
        printf("[-] GetProcAddress Failed.\n");
        return FALSE;
    }

    //pfnStartAddr��ַ����LoadLibraryA����ڵ�ַ
    HANDLE hThread = CreateRemoteThread(hProc,
        NULL,
        0,
        pfnStartAddr,
        psLibFileRemote,
        0,
        NULL);
	
    if(hThread == NULL)
    {
        printf("[-] CreateRemoteThread Failed. ErrCode = %d\n",GetLastError());
        return FALSE;
    }

	printf("[*]Inject Succesfull.\n");
    return TRUE;
}


DWORD ProcesstoPid(char *Processname) //����ָ�����̵�PID(Process ID)
{
	HANDLE hProcessSnap=NULL;
	DWORD ProcessId=0;
	PROCESSENTRY32 pe32={0};
	hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0); //�򿪽��̿���
	if(hProcessSnap==(HANDLE)-1)
	{
		printf("\nCreateToolhelp32Snapshot() Error: %d",GetLastError());
		return 0;
	}
	pe32.dwSize=sizeof(PROCESSENTRY32);
	if(Process32First(hProcessSnap,&pe32)) //��ʼö�ٽ���
	{
		do
		{
			if(!stricmp(Processname,pe32.szExeFile)) //�ж��Ƿ���ṩ�Ľ�������ȣ��ǣ����ؽ��̵�ID
			{
				ProcessId=pe32.th32ProcessID;
				break;
			}
		}
		while(Process32Next(hProcessSnap,&pe32)); //����ö�ٽ���
	}
	else
	{
		printf("\nProcess32First() Error: %d",GetLastError());
		return 0;
	}
	CloseHandle(hProcessSnap); //�ر�ϵͳ���̿��յľ��
	return ProcessId;
}

BOOL EnableDebugPrivilege() //��������������Ȩ�ޣ�������SE_DEBUG_NAME
{ 
	TOKEN_PRIVILEGES tkp; 
	HANDLE hToken; 
	if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken))     //�򿪵�ǰ����ʧ�� 
		return FALSE; 
	LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tkp.Privileges[0].Luid); //�鿴��ǰȨ��
	tkp.PrivilegeCount = 1; 
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); //����Ȩ�ޣ���������
	return TRUE; 
}