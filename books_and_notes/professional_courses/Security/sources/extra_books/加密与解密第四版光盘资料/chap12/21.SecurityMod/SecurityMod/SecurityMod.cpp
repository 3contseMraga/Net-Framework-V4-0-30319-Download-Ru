/*-----------------------------------------------------------------------
��12��  ע�뼼��
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// SecurityMod.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "SecurityMod.h"
#include <stdio.h>
#include <vector>
#include <algorithm>

using namespace std;

typedef vector<DWORD> ThreadVector;

extern "C" SECURITYMOD_API BOOL RegisterThread(DWORD dwThreadId);
VOID UnRegisterThread(DWORD dwThreadId);
BOOL IsTrustedThread(DWORD dwThreadId);

#define MAX_THREAD_CNT (1024)
DWORD g_ThreadIs[MAX_THREAD_CNT]={0};
DWORD g_ThreadCnt = 0 ;
BOOL  g_EnableProtect = FALSE;
CRITICAL_SECTION g_cs;
ThreadVector g_ThreadVector;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	DWORD dwCurThreadId = GetCurrentThreadId();
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				InitializeCriticalSection(&g_cs);
				printf("[SecurityMod] Loading.\n");
				break;
			}
		case DLL_THREAD_ATTACH:
			{
				if (!IsTrustedThread(dwCurThreadId))
				{
					printf("ThreadId = %d . �Ƿ��̣߳��˳�.\n",dwCurThreadId);
					ExitThread(0);
				}
				else
				{
					printf("ThreadId = %d . �����̣߳����С�\n",dwCurThreadId);
				}
				
				break;
			}
		case DLL_THREAD_DETACH:
			{
				if (IsTrustedThread(dwCurThreadId))
				{
					printf("�������߳� %d �����˳�.\n",dwCurThreadId);
					UnRegisterThread(dwCurThreadId);
				}
				break;
			}
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

extern "C" SECURITYMOD_API BOOL RegisterThread(DWORD dwThreadId)
{
	BOOL bResult = FALSE ;
	EnterCriticalSection(&g_cs);
	g_ThreadVector.push_back(dwThreadId);
	g_ThreadCnt++;
	LeaveCriticalSection(&g_cs);
	return TRUE;
}

VOID UnRegisterThread(DWORD dwThreadId)
{
	ThreadVector::iterator theIterator;
	EnterCriticalSection(&g_cs);
	theIterator = find(g_ThreadVector.begin(),g_ThreadVector.end(),dwThreadId);
	if (theIterator != g_ThreadVector.end())
	{
		//Found
		g_ThreadVector.erase(theIterator);
		g_ThreadCnt--;
	}
	LeaveCriticalSection(&g_cs);
}

BOOL IsTrustedThread(DWORD dwThreadId)
{
	BOOL bResult = FALSE ;
	DWORD i = 0 ;

	if (g_ThreadVector.size() == 0)
	{
		//��δע��ʱ���������̷߳���
		return TRUE;
	}

	ThreadVector::iterator theIterator;
	EnterCriticalSection(&g_cs);
	theIterator = find(g_ThreadVector.begin(),g_ThreadVector.end(),dwThreadId);
	if (theIterator != g_ThreadVector.end())
	{
		//Found
		bResult = TRUE;
	}
	LeaveCriticalSection(&g_cs);
	return bResult;
}
