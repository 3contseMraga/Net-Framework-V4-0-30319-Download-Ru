/*-----------------------------------------------------------------------
��13��  Hook����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/


// VEHHook.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

BOOL SetBreakPoint(PVOID pFuncAddr);
BOOL ClearBreakPoint(PVOID pFuncAddr);
BOOL InstallVEHHook(PVECTORED_EXCEPTION_HANDLER Handler);
VOID UnInstallVEHHook();

typedef int 
(WINAPI *PFN_MessageBox)(
	HWND hWnd,          // handle of owner window
	LPCTSTR lpText,     // address of text in message box
	LPCTSTR lpCaption,  // address of title of message box
	UINT uType          // style of message box
	);

int WINAPI My_MessageBox(
	HWND hWnd,          // handle of owner window
	LPCTSTR lpText,     // address of text in message box
	LPCTSTR lpCaption,  // address of title of message box
	UINT uType          // style of message box
	);

LONG WINAPI VectoredHandler1(struct _EXCEPTION_POINTERS *ExceptionInfo);
LONG WINAPI VectoredHandler2(struct _EXCEPTION_POINTERS *ExceptionInfo);
LONG WINAPI VectoredHandler3(struct _EXCEPTION_POINTERS *ExceptionInfo);
VOID ShowMsgBox(LPCTSTR lpMsg);
ULONG_PTR InitTrampolineFun();

PFN_MessageBox g_OriginalMessageBoxA;
PVOID g_AddrofMessageBoxA = 0 ;
PVOID g_hVector;
BYTE  g_OldCode[16]={0};

int main(int argc, char* argv[])
{
	HMODULE hUser32 = LoadLibrary("user32.dll");
	g_AddrofMessageBoxA =  (PVOID)GetProcAddress(hUser32,"MessageBoxA");
	printf("Address of MessageBoxA = 0x%p\n",g_AddrofMessageBoxA);
	g_OriginalMessageBoxA = (PFN_MessageBox)InitTrampolineFun();  //������ͷ��Hook
	
	printf("Addr of VectoredHandler1 = 0x%p\n",VectoredHandler1);
	printf("Addr of VectoredHandler2 = 0x%p\n",VectoredHandler2);
	printf("Addr of VectoredHandler3 = 0x%p\n",VectoredHandler3);

	//ѡ��װһ�����в���
	InstallVEHHook(VectoredHandler3);

	//���öϵ�
	SetBreakPoint(g_AddrofMessageBoxA);

	//call
	ShowMsgBox("VEH Hook Test.");

	printf("All Finished!\n");
	ClearBreakPoint(g_AddrofMessageBoxA);
	UnInstallVEHHook();
	

	ShowMsgBox("Hook Cleared");
	return 0;
}

VOID ShowMsgBox(LPCTSTR lpMsg)
{
	MessageBoxA(NULL,lpMsg,"Test",MB_OK);
}

ULONG_PTR InitTrampolineFun()
{
	ULONG_PTR uResult = 0 ;
	PBYTE pFun = NULL;

#ifdef _WIN64
	//x64��Ҫ����shellcode
	/*
	USER32!MessageBoxA:
	00000000`779412b8 4883ec38        sub     rsp,38h
	00000000`779412bc 4533db          xor     r11d,r11d
	00000000`779412bf 44391d760e0200  cmp     dword ptr [USER32!gapfnScSendMessage+0x927c (00000000`7796213c)],r11d
	*/
	pFun = (PBYTE)VirtualAlloc(NULL,128,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	uResult = (ULONG_PTR)pFun;
	
	memset(pFun,0,128);
	memcpy(pFun,(PVOID)g_AddrofMessageBoxA,4); //������һ��ָ�4�ֽ�,�Ƽ�ʹ�÷����������ʵ�ʼ���
	pFun += 4 ; //��һ��ָ���Ϊjmp [xxxxxx]
	pFun[0] = 0xFF;
	pFun[1] = 0x25;
	*(ULONG_PTR*)(pFun + 6) = (ULONG_PTR)g_AddrofMessageBoxA + 4 ; //���ص�ԭ������4�ĵط�
#else
	//x86,��һ��ָ����mov edi,edi,ֱ����������
	uResult = (ULONG_PTR)g_AddrofMessageBoxA + 2;
#endif

	return uResult;
}


//����ʽ���޸Ĳ���������ԭ��������ִ��
LONG WINAPI
VectoredHandler1(
	struct _EXCEPTION_POINTERS *ExceptionInfo
	)
{
	char *szNewMsg =  "[VectoredHandler1] Hacked by pediy.com";
	LONG lResult = EXCEPTION_CONTINUE_SEARCH ;
	PEXCEPTION_RECORD pExceptionRecord;  
	PCONTEXT pContextRecord;
	int ret = 0 ;
	pExceptionRecord = ExceptionInfo->ExceptionRecord ;
	pContextRecord = ExceptionInfo->ContextRecord ;
	ULONG_PTR* uESP = 0 ;
	printf("Exception Address = %p\n",pExceptionRecord->ExceptionAddress);
	if (pExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT
		&& pExceptionRecord->ExceptionAddress == g_AddrofMessageBoxA)
	{
#ifdef _WIN64
		//x64��ǰ�ĸ���������ΪRCX,RDX,R8,R9
		//�޸ĵڶ�����������LpMsg
		printf("lpText = 0x%p   %s\n",pContextRecord->Rdx,(char*)pContextRecord->Rdx);
		pContextRecord->Rdx = (ULONG_PTR)szNewMsg;
		pContextRecord->Rip = (ULONG_PTR)g_OriginalMessageBoxA ; //����Trampoline����ִ��
#else
		/*
		0012FF70   0040105A   /CALL �� MessageBoxA ���� VEHHook.00401054
		0012FF74   00000000   |hOwner = NULL
		0012FF78   00407030   |Text = "VEH Hook"
		0012FF7C   0040703C   |Title = "Test"
		0012FF80   00000000   \Style = MB_OK|MB_APPLMODAL
		0012FF84   00401225   ���ص� VEHHook.<ModuleEntryPoint>+0B4 ���� VEHHook.00401000
		*/
		printf("ESP = 0x%p\n",pContextRecord->Esp) ;
		uESP = (ULONG_PTR*)pContextRecord->Esp ; //ȡ�ж�ʱ��ESP
		uESP[2] = (ULONG_PTR)szNewMsg; //�޸�ջ�еĲ���
		pContextRecord->Eip = (ULONG_PTR)g_OriginalMessageBoxA ; //����������ͷ
#endif
		
		lResult = EXCEPTION_CONTINUE_EXECUTION ;
	}
    return lResult;
}

//����ʽ��ֱ�ӵ���ԭ��������ԭ��������
LONG WINAPI
VectoredHandler2(
	struct _EXCEPTION_POINTERS *ExceptionInfo
	)
{
	char *szNewMsg =  "[VectoredHandler2] Hacked by pediy.com";
	LONG lResult = EXCEPTION_CONTINUE_SEARCH ;
	PEXCEPTION_RECORD pExceptionRecord;  
	PCONTEXT pContextRecord;
	int ret = 0 ;
	pExceptionRecord = ExceptionInfo->ExceptionRecord ;
	pContextRecord = ExceptionInfo->ContextRecord ;
	ULONG_PTR* uESP = 0 ;
	if (pExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT
		&& pExceptionRecord->ExceptionAddress == g_AddrofMessageBoxA)
	{
		
#ifdef _WIN64
		//x64��ǰ�ĸ���������ΪRCX,RDX,R8,R9
		printf("RSP = 0x%p\n",pContextRecord->Rsp) ;
		uESP = (ULONG_PTR*)pContextRecord->Rsp ;
		printf("Return Address = 0x%p\n",uESP[0]);
		ret = g_OriginalMessageBoxA((HWND)pContextRecord->Rcx,szNewMsg,(LPCTSTR)pContextRecord->R8,(int)pContextRecord->R9);
		printf("ret = %d\n",ret);
		//����RSP
		pContextRecord->Rsp += sizeof(ULONG_PTR);//�����ڼĴ����У�ջ���޲����������������ص�ַ
		//ֱ�ӷ��ص������ߴ�
		pContextRecord->Rip = uESP[0] ;//����EIPΪ���ص�ַ
#else
		/*
		0012FF70   0040105A   /CALL �� MessageBoxA ���� VEHHook.00401054
		0012FF74   00000000   |hOwner = NULL
		0012FF78   00407030   |Text = "VEH Hook"
		0012FF7C   0040703C   |Title = "Test"
		0012FF80   00000000   \Style = MB_OK|MB_APPLMODAL
		0012FF84   00401225   ���ص� VEHHook.<ModuleEntryPoint>+0B4 ���� VEHHook.00401000
		*/
		printf("ESP = 0x%p\n",pContextRecord->Esp) ;
		uESP = (ULONG_PTR*)pContextRecord->Esp ;
		ret = g_OriginalMessageBoxA((HWND)uESP[1],szNewMsg,(LPCTSTR)uESP[3],(int)uESP[4]);
		printf("ret = %d\n",ret);

		//ֱ�ӷ��ص������ߴ�
		pContextRecord->Eip = uESP[0] ;//����EIPΪ���ص�ַ
		pContextRecord->Esp += (4 + 1)*sizeof(ULONG_PTR); //4Ϊ��������,1Ϊ���ص�ַ
#endif
		
		lResult = EXCEPTION_CONTINUE_EXECUTION ;
	}
    return lResult;
}

//����ʽ��ֱ�ӷ���,�൱�ڹ��˵�
LONG WINAPI
VectoredHandler3(
	struct _EXCEPTION_POINTERS *ExceptionInfo
	)
{
	LONG lResult = EXCEPTION_CONTINUE_SEARCH ;
	PEXCEPTION_RECORD pExceptionRecord = ExceptionInfo->ExceptionRecord ;
	PCONTEXT pContextRecord = ExceptionInfo->ContextRecord ;
	ULONG_PTR* uESP = 0 ;
	if (pExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT
		&& pExceptionRecord->ExceptionAddress == g_AddrofMessageBoxA)
	{
		
		/*
		0012FF70   0040105A   /CALL �� MessageBoxA ���� VEHHook.00401054
		0012FF74   00000000   |hOwner = NULL
		0012FF78   00407030   |Text = "VEH Hook"
		0012FF7C   0040703C   |Title = "Test"
		0012FF80   00000000   \Style = MB_OK|MB_APPLMODAL
		0012FF84   00401225   ���ص� VEHHook.<ModuleEntryPoint>+0B4 ���� VEHHook.00401000
		*/
		

		//ֱ�ӷ��ص������ߴ�
#ifdef _WIN64
		printf("RSP = 0x%p\n",pContextRecord->Rsp) ;
		uESP = (ULONG_PTR*)pContextRecord->Rsp ;
		pContextRecord->Rip = uESP[0] ;//����EIPΪ���ص�ַ
		pContextRecord->Rsp += sizeof(ULONG_PTR); //��ѹ��ջ�ڵĲ����ͷ��ص�ַ�����,4Ϊ��������,1Ϊ���ص�ַ
#else
		printf("ESP = 0x%X\n",pContextRecord->Esp) ;
		uESP = (ULONG_PTR*)pContextRecord->Esp ;
		pContextRecord->Eip = uESP[0] ;//����EIPΪ���ص�ַ
		pContextRecord->Esp += (4 + 1)*sizeof(ULONG_PTR); //��ѹ��ջ�ڵĲ����ͷ��ص�ַ�����,4Ϊ��������,1Ϊ���ص�ַ
#endif
		
		lResult = EXCEPTION_CONTINUE_EXECUTION ;
	}
    return lResult;
}

BOOL InstallVEHHook(PVECTORED_EXCEPTION_HANDLER Handler)
{
	printf("Current Handler Address = 0x%p\n",Handler);
	g_hVector = AddVectoredExceptionHandler(1,Handler);
	return g_hVector != NULL ;
}

VOID UnInstallVEHHook()
{
	RemoveVectoredExceptionHandler(g_hVector);
}

/*
0:000> u user32!messageboxA
USER32!MessageBoxA:
77d507ea 8bff            mov     edi,edi
77d507ec 55              push    ebp
77d507ed 8bec            mov     ebp,esp
*/
BOOL SetBreakPoint(PVOID pFuncAddr)
{
	DWORD dwCnt = 0 ;
	BYTE *pTarget = (BYTE*)pFuncAddr;
	
	
	g_OldCode[0] = *pTarget;
	printf("Original Fun Head Code = 0x%02X\n",g_OldCode[0]);
	//�޸��ڴ�ҳ������
	DWORD dwOLD;
	MEMORY_BASIC_INFORMATION  mbi;
	VirtualQuery(pTarget,&mbi,sizeof(mbi));
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_EXECUTE_READWRITE,&dwOLD);
	
	//д��int3
	*pTarget  = 0xCC ;

	//�ָ��ڴ�ҳ������
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,dwOLD,0);
	return TRUE;
}

BOOL ClearBreakPoint(PVOID pFuncAddr)
{
	BYTE *pTarget = (BYTE*)pFuncAddr;
	//�޸��ڴ�ҳ������
	DWORD dwOLD;
	MEMORY_BASIC_INFORMATION  mbi;
	VirtualQuery(pTarget,&mbi,sizeof(mbi));
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_EXECUTE_READWRITE,&dwOLD);
	
	*pTarget  = g_OldCode[0] ;
	
	//�ָ��ڴ�ҳ������
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,dwOLD,0);
	return TRUE;
}

int WINAPI My_MessageBox(
	HWND hWnd,          // handle of owner window
	LPCTSTR lpText,     // address of text in message box
	LPCTSTR lpCaption,  // address of title of message box
	UINT uType          // style of message box
	)
{	
	char newMsg[400];
	char newCation[]="���ⱻ�Ҹ���!";
	int result;
	if (lpText)
	{
		ZeroMemory(newMsg,400);
		lstrcpy(newMsg,lpText);
		lstrcat(newMsg,"\n\tMessage Box hacked by pediy.com");
	}
	printf("���˵���MessageBox...\n");
	result = g_OriginalMessageBoxA(hWnd,newMsg,newCation,uType);
	return result;
	
}
