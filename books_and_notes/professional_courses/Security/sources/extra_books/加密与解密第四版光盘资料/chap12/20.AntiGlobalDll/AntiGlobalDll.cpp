// AntiGlobalDll.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AntiGlobalDll.h"
#include "AntiGlobalDllDlg.h"
#include "Hook.h"

#pragma comment(lib, "version.lib") 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


typedef LONG NTSTATUS;
#define STATUS_SUCCESS	(0x00000000L)

typedef struct _UNICODE_STRING { // UNICODE_STRING structure  
    USHORT Length;  
    USHORT MaximumLength;  
    PWSTR  Buffer;  
} UNICODE_STRING;  
typedef UNICODE_STRING *PUNICODE_STRING;  


typedef NTSTATUS
(WINAPI *fLdrLoadDll) //LdrLoadDll function prototype  
(  
	IN PWCHAR PathToFile OPTIONAL,  
	IN PULONG Flags OPTIONAL,  
	IN PUNICODE_STRING ModuleFileName,  
	OUT PHANDLE ModuleHandle  
	);  

NTSTATUS WINAPI DetourLdrLoadDll(
	IN PWCHAR PathToFile ,  
	IN PULONG Flags ,  
	IN PUNICODE_STRING ModuleFileName,  
	OUT PHANDLE ModuleHandle 
	);

NTSTATUS WINAPI TrampolineLdrLoadDll(
	IN PWCHAR PathToFile ,  
	IN PULONG Flags ,  
	IN PUNICODE_STRING ModuleFileName,  
	OUT PHANDLE ModuleHandle 
	);

BOOL Inline_InstallHook(void);
BOOL Inline_UnInstallHook();

HOOK_DATA LdrHookData;

/////////////////////////////////////////////////////////////////////////////
// CAntiGlobalDllApp

BEGIN_MESSAGE_MAP(CAntiGlobalDllApp, CWinApp)
	//{{AFX_MSG_MAP(CAntiGlobalDllApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAntiGlobalDllApp construction

CAntiGlobalDllApp::CAntiGlobalDllApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CAntiGlobalDllApp object

CAntiGlobalDllApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CAntiGlobalDllApp initialization

BOOL CAntiGlobalDllApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
	
	Inline_InstallHook();

	CAntiGlobalDllDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}



BOOL Inline_InstallHook()
{
	//׼��Hook
	ZeroMemory(&LdrHookData,sizeof(HOOK_DATA));
	strcpy(LdrHookData.szApiName , "LdrLoadDll");
	strcpy(LdrHookData.szModuleName , "ntdll.dll");
	LdrHookData.HookCodeLen = 5;
	LdrHookData.HookPoint = (ULONG_PTR)GetAddress(LdrHookData.szModuleName,LdrHookData.szApiName);//HOOK�ĵ�ַ
	LdrHookData.pfnTrampolineFun = (ULONG_PTR)TrampolineLdrLoadDll;//����ԭʼ������ͨ��
	LdrHookData.pfnDetourFun = (ULONG_PTR)DetourLdrLoadDll;//Fake
	
	return InstallCodeHook(&LdrHookData);
}

BOOL Inline_UnInstallHook()
{
	return UninstallCodeHook(&LdrHookData);
}



__declspec( naked )
NTSTATUS WINAPI TrampolineLdrLoadDll(
	IN PWCHAR PathToFile ,  
	IN PULONG Flags ,  
	IN PUNICODE_STRING ModuleFileName,  
	OUT PHANDLE ModuleHandle 
	)
{
	_asm
	{
		//��������д���Hookָ���ƻ���ԭ������ͷ�ļ���ָ����Խ�ԭ����ָ��Ƶ����������ִ��ԭ������ǰ����ָ��
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		jmp LdrHookData.JmpBackAddr //����Hook����֮��ĵط����ƹ��Լ���װ��HOOK
	}
}



NTSTATUS WINAPI DetourLdrLoadDll(
	IN PWCHAR PathToFile ,  
	IN PULONG Flags ,  
	IN PUNICODE_STRING ModuleFileName,  
	OUT PHANDLE ModuleHandle 
	)
{
	NTSTATUS status  = STATUS_SUCCESS ;
	WCHAR *pFileName = NULL ;
	WCHAR Buffer[1024]={0};


	pFileName = wcsrchr(ModuleFileName->Buffer,'\\');
	pFileName = (pFileName == NULL) ? ModuleFileName->Buffer : (pFileName +1) ;
	
	//������Զ�ϵͳ�����dll���з��У�����ȫ���ܾ�������������޷����ش�����ʽ��
	if (   wcsicmp(pFileName,L"uxtheme.dll") == 0
		|| wcsicmp(pFileName,L"COMCTL32.DLL") == 0
		|| wcsicmp(pFileName,L"COMCTL32.DLL") == 0
		|| wcsicmp(pFileName,L"imm32.dll") == 0
		|| wcsicmp(pFileName,L"version.dll") == 0
		|| wcsicmp(pFileName,L"msctf.dll") == 0
		|| wcsicmp(pFileName,L"imm32.dll") == 0
		|| wcsicmp(pFileName,L"ole32.dll") == 0
		|| wcsicmp(pFileName,L"msctfime.ime") == 0)
	{
		status = TrampolineLdrLoadDll(PathToFile,Flags,ModuleFileName,ModuleHandle);
	}
	else
	{
		swprintf(Buffer,L"Flags = 0x%X Dll Injected : %ws\n",*Flags,ModuleFileName->Buffer);
		OutputDebugStringW(Buffer);
	}
	return status;
}
