/*-----------------------------------------------------------------------
��12��  ע�뼼��
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// UserApcInjectDll.cpp : Defines the entry point for the console application.
// Tested On:WinXP SP3/Win7 x86/Win7 x64

#include "stdafx.h"
#include <windows.h>
#include <TLHELP32.H>

DWORD ProcesstoPid(char *Processname) ;
BOOL InjectModuleToProcessById(DWORD dwPid, char *szDllFullPath);

#ifdef _WIN64
char g_szDllPath[MAX_PATH] = "F:\\Program2016\\DllInjection\\MsgDll64.dll" ;
char g_szProcName[MAX_PATH] = "HostProc64.exe";
#else
char g_szDllPath[MAX_PATH] = "F:\\Program2016\\DllInjection\\MsgDll.dll" ;
char g_szProcName[MAX_PATH] = "HostProc.exe";
#endif

int main(int argc, char* argv[])
{
	DWORD dwPid = ProcesstoPid(g_szProcName);
	if (dwPid == 0)
	{
		printf("δ�ҵ�Ŀ�����!\n");
		return 0;
	}
	printf("Target Pid = %d\n",dwPid);
	BOOL bResult = InjectModuleToProcessById(dwPid,g_szDllPath);
	printf("Result = %d\n",bResult);
	return 0;
}

BOOL InjectModuleToProcessById(DWORD dwPid, char *szDllFullPath)  
{  
    SIZE_T   stSize = 0 ;  
    BOOL    bStatus = FALSE ;  
    LPVOID  lpData = NULL ;  
    SIZE_T  uLen = lstrlen(szDllFullPath) + 1;  
	
    // ��Ŀ�����  
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid) ;  
    if (hProcess)  
    {  
        // ����ռ�  
        lpData = VirtualAllocEx(hProcess, NULL, uLen, MEM_COMMIT, PAGE_READWRITE);  
        DWORD dwErr = GetLastError();  
        if (lpData)  
        {  
            // д����Ҫע���ģ��·��ȫ��  
            bStatus = WriteProcessMemory(hProcess, lpData, szDllFullPath, uLen, &stSize) ;  
        }  
		
        CloseHandle(hProcess) ;  
    }  
	
	//���ϲ����봴��Զ���̵߳�׼��������ͬ

    if (bStatus == FALSE)  
        return FALSE ;  
	
    // �����߳̿���  
    THREADENTRY32 te32 = {sizeof(THREADENTRY32)} ;  
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0) ;  
    if (hThreadSnap == INVALID_HANDLE_VALUE)   
        return FALSE ;   
	
    bStatus = FALSE ;  
    // ö�������߳�  
    if (Thread32First(hThreadSnap, &te32))  
    {  
        do{  
            // �ж��Ƿ�Ŀ������е��߳�  
            if (te32.th32OwnerProcessID == dwPid)  
            {  
                // ���߳�  
                HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID) ;  
                if (hThread)  
                {  
                    // ��ָ���߳����APC  
                    DWORD dwRet = QueueUserAPC ((PAPCFUNC)LoadLibraryA, hThread, (ULONG_PTR)lpData) ;  
                    if (dwRet > 0)  
                        bStatus = TRUE ;  
                    CloseHandle (hThread) ;  
                }  
            }   
			
        }while (Thread32Next ( hThreadSnap, &te32 )) ;  
    }  
	
    CloseHandle (hThreadSnap) ;  
    return bStatus;  
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