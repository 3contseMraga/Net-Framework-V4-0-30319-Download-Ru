// SafeSEH.cpp : �������̨Ӧ�ó������ڵ㡣
// Author:achillis
// ������Ϊ��ʾSafeSEH��ȫ���ö��쳣�����Ӱ��
// ��ʹ��֧��SafeSEH���õı�������VS2003���ϣ��ڴ򿪻�ر�SafeSEH���ص�����±��뱾���򣬹۲첻ͬ��Ч��
/*-----------------------------------------------------------------------
��8��  Windows�µ��쳣����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/
#include "stdafx.h"
#include <windows.h>
#include <ImageHlp.h>

#pragma comment(lib,"imagehlp.lib")

typedef struct {
	DWORD       Size;
	DWORD       TimeDateStamp;
	WORD        MajorVersion;
	WORD        MinorVersion;
	DWORD       GlobalFlagsClear;
	DWORD       GlobalFlagsSet;
	DWORD       CriticalSectionDefaultTimeout;
	DWORD       DeCommitFreeBlockThreshold;
	DWORD       DeCommitTotalFreeThreshold;
	DWORD       LockPrefixTable;            // VA
	DWORD       MaximumAllocationSize;
	DWORD       VirtualMemoryThreshold;
	DWORD       ProcessHeapFlags;
	DWORD       ProcessAffinityMask;
	WORD        CSDVersion;
	WORD        Reserved1;
	DWORD       EditList;                   // VA
	DWORD_PTR   *SecurityCookie;
	PVOID       *SEHandlerTable;
	DWORD       SEHandlerCount;
} IMAGE_LOAD_CONFIG_DIRECTORY32_2;

VOID ShowSafeSEHHandler(HMODULE hMod);
VOID SEHSafe();
void SEHUnSafe();

DWORD g_Value = 0;

EXCEPTION_DISPOSITION __cdecl MyHandler (
	struct _EXCEPTION_RECORD *ExceptionRecord,
	void * EstablisherFrame,
	struct _CONTEXT *ContextRecord,
	void * DispatcherContext
	);

int _tmain(int argc, _TCHAR* argv[])
{
	ShowSafeSEHHandler(GetModuleHandle(NULL));
	printf("MyHandler Address = 0x%p\n",(ULONG)MyHandler);
	//����SEH�쳣��������Ƿ���Ч
	SEHSafe();
	SEHUnSafe();
	printf("[main] After SEH Function.\n");
	return 0;
}

//��������SafeSEH���ú������쳣���������Ȼ������������
VOID SEHSafe()
{
	int *pValue = NULL;
	__try
	{
		//����һ���ڴ�д���쳣
		*pValue = 5;
	}
	__except(printf("[SEHSafe] In Filter.\n"),EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[SEHSafe] In Handler.\n");
	}
}

//��������SafeSEH���ú������쳣������̽��޷���������
__declspec(naked) void SEHUnSafe()
{
	__asm
	{
		push ebp
		mov ebp,esp
		push offset MyHandler
		push dword ptr fs:[0]
		mov dword ptr fs:[0],esp
		xor eax,eax
		mov eax,[eax]	;����һ���ڴ��ȡ�쳣
		mov esp,dword ptr fs:[0]
		pop dword ptr fs:[0]
		mov esp,ebp
		pop ebp
		retn
	}
}

EXCEPTION_DISPOSITION __cdecl MyHandler (
	struct _EXCEPTION_RECORD *ExceptionRecord,
	void * EstablisherFrame,
	struct _CONTEXT *ContextRecord,
	void * DispatcherContext
	)
{
	printf("[SEHUnSafe] In Handler.\n");
	if (ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
	{
		ContextRecord->Eax = (ULONG)&g_Value;
		printf("[SEHUnSafe] Exception Fixed.\n");
		return ExceptionContinueExecution;
	}
	return ExceptionContinueSearch;
}

//��ʾSafeSEH��������
VOID ShowSafeSEHHandler(HMODULE hMod)
{
	IMAGE_LOAD_CONFIG_DIRECTORY32_2 *pLoadCfg = NULL ;
	ULONG DirectorySize = 0 ;
	DWORD i = 0 ;

	pLoadCfg = (IMAGE_LOAD_CONFIG_DIRECTORY32_2*)ImageDirectoryEntryToData(
		hMod,TRUE,IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,&DirectorySize);

	if (DirectorySize == 0)
	{
		printf("No SafeSEH Handler Table.\n");
		return ;
	}

	printf("LoadConfigDir   = 0x%p\n",pLoadCfg);
	printf("Size	        = 0x%X\n",pLoadCfg->Size);
	printf("LockPrefixTable = 0x%p\n",pLoadCfg->LockPrefixTable);
	printf("SecurityCookie  = 0x%p  Cookie = 0x%p\n",pLoadCfg->SecurityCookie,*pLoadCfg->SecurityCookie);
	printf("SEHandlerTable  = 0x%p\n",pLoadCfg->SEHandlerTable);
	printf("SEHandlerCount  = %d\n",pLoadCfg->SEHandlerCount);

	printf("SehHandler Table Details :\n");
	for (i = 0 ;i < pLoadCfg->SEHandlerCount;i++)
	{
		printf("[%d] Handler : 0x%p\n",i,(ULONG_PTR)hMod + (ULONG_PTR)pLoadCfg->SEHandlerTable[i]) ;
	}
	printf("===================Dump SafeSEH Handler Finished.======================\n");

}
