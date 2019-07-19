/*-----------------------------------------------------------------------
��13��  Hook����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/


// COMHook.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "COMHook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

VOID ShellTest();
BOOL HookComInterface(IUnknown *pDispatch,ULONG FuncIndex,ULONG DetourFunAddr,ULONG *pOriginalAddr);

typedef HRESULT
( STDMETHODCALLTYPE *PFN_FindFiles )(
 IShellDispatch * This
 );

HRESULT STDMETHODCALLTYPE DetourFindFiles( IShellDispatch * This);

PFN_FindFiles g_OriginalFindFiles;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		// TODO: code your application's behavior here.
		ShellTest();
	}

	return nRetCode;
}

VOID ShellTest()
{
	HRESULT hr;
	IShellDispatch *pShellDispatch = NULL ;
	ULONG uFun = 0 ;
	
	if (CoInitialize(NULL) != S_OK)
	{
		printf("CoInitialize failed!\n");
		return ;
	}
	
	hr = CoCreateInstance(CLSID_Shell,NULL,CLSCTX_SERVER,
		IID_IShellDispatch,(LPVOID*)&pShellDispatch);
	if (hr == S_OK)
	{
		printf("pShellDispatch = 0x%X\n",pShellDispatch);
		HookComInterface(pShellDispatch,26,(ULONG)DetourFindFiles,(ULONG*)&g_OriginalFindFiles);
		pShellDispatch->FindFiles(); //7d758B42 
		pShellDispatch->Release();
	}
	else
	{
		printf("Create Instance failed!\n");
	}
	
	CoUninitialize();
}

BOOL HookComInterface(IUnknown *pDispatch,ULONG FuncIndex,ULONG DetourFunAddr,ULONG *pOriginalAddr)
{
	BOOL bResult = FALSE ;
	DWORD dwOLD;
	MEMORY_BASIC_INFORMATION  mbi;
	ULONG uAddr = 0 ;
	
	ULONG *vfTableToHook = (ULONG*)*(ULONG*)pDispatch;
	//printf("vfTable = 0x%x\n",vfTableToHook);

	//�޸��ڴ�ҳ������
	if (VirtualQuery(vfTableToHook,&mbi,sizeof(mbi))
		&& VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_EXECUTE_READWRITE,&dwOLD))
	{
		//����ԭʼ����
		if (pOriginalAddr != NULL)
		{
			*pOriginalAddr = vfTableToHook[FuncIndex];
		}
		vfTableToHook[FuncIndex] = DetourFunAddr;
		bResult = TRUE ;
	}
	//�ָ��ڴ�ҳ������
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,dwOLD,0);
	printf("COM Hook Installed.\n");
	return bResult;
}

HRESULT WINAPI DetourFindFiles( IShellDispatch * This)
{
	printf("My_FindFiles:: this = 0x%x\n",This);
	return g_OriginalFindFiles(This);
}


