/////////////////////////////////////////////////////////////////
//         ��13��  Hook���� ����������ܣ����İ棩��           //
//                                                             //
//         Author: achillis(���½���)                          //
//         Blog  : http://www.cnblogs.com/achillis/            //
//         QQ    : 344132161                                   //
//         Email : achillis@126.com                            //
//         ת���뱣��������Ϣ                                  //
//         (c)  ��ѩѧԺ www.kanxue.com 2000-2018              //
/////////////////////////////////////////////////////////////////
//������:Ϊuser32.dll��װEAT Hook��Ŀ�꺯����MessageBoxA

#include <windows.h>
#include <stdio.h>
#include <imagehlp.h>

#pragma comment(lib,"imagehlp.lib")

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

BOOL InstallModuleEATHook(
	HMODULE hModToHook,// IN
	char *szFuncName,// IN
	PVOID ProxyFunc,// IN
	PULONG_PTR *pAddrPointer,//OUT
	ULONG_PTR *pOriginalFuncAddr//OUT
	);

int main(int argc, char* argv[])
{
	HMODULE hUser32 = LoadLibrary("user32.dll");
	PULONG_PTR pEATPointer;
	ULONG_PTR uOldRVA;
	InstallModuleEATHook(hUser32,"MessageBoxA",My_MessageBox,&pEATPointer,&uOldRVA);
	printf("pEATPointer = 0x%X OldRVA = 0x%X\n",pEATPointer,uOldRVA);
	

	//Test
	printf("Now Test the EAT Hook.\n");
	PFN_MessageBox pMsgBox = (PFN_MessageBox)GetProcAddress(GetModuleHandle("user32.dll"),"MessageBoxA");
	printf("dwAddr = 0x%p\n",pMsgBox);
	pMsgBox(NULL,"EAT Hook","Test",MB_OK);
	

	//����һ����MessageBox��Dll������
#ifdef _WIN64
	LoadLibrary("../Dll/MsgDll64.dll");
#else
	LoadLibrary("../Dll/MsgDll.dll");
#endif
	return 0;
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
	result=MessageBoxA(hWnd,newMsg,newCation,uType);
	return result;
	
}

BOOL InstallModuleEATHook(
	HMODULE hModToHook,// IN
	char *szFuncName,// IN
	PVOID ProxyFunc,// IN
	PULONG_PTR *pAddrPointer,//OUT
	ULONG_PTR *pOriginalFuncAddr//OUT
	)
{
	PIMAGE_EXPORT_DIRECTORY	  pExportDir;
	ULONG ulSize;
	ULONG_PTR TargetFunAddr;
	BOOL result = FALSE ;
	ULONG nFuncCnt = 0 ;
	ULONG i = 0 ;
	ULONG_PTR TargetFunRVA = 0 ;
	ULONG *funrav = NULL ;
	
	TargetFunAddr = (ULONG_PTR)GetProcAddress(hModToHook,szFuncName);
	TargetFunRVA = (ULONG)(TargetFunAddr - (ULONG_PTR)hModToHook); 
	printf("[*]Address of %s:0x%p  RVA = 0x%X\n",szFuncName,TargetFunAddr,TargetFunRVA);
	printf("[*]Module To Hook at Base:0x%p\n",hModToHook);
	pExportDir = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData(hModToHook, TRUE,IMAGE_DIRECTORY_ENTRY_EXPORT, &ulSize);
	printf("[*]Find ExportTable,Address:0x%p\n",pExportDir);
	nFuncCnt = pExportDir->NumberOfFunctions ;
	funrav=(ULONG*)((BYTE*)hModToHook+pExportDir->AddressOfFunctions);
	for (i=0;i<nFuncCnt;i++)
	{
		if (funrav[i] == TargetFunRVA)
		{
			printf("[*]Find target address!\n");
			//�޸��ڴ�ҳ������
			DWORD dwOLD;
			MEMORY_BASIC_INFORMATION  mbi;
			VirtualQuery(&funrav[i],&mbi,sizeof(mbi));
			VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_EXECUTE_READWRITE,&dwOLD);
			//����ԭʼ����
			if (pAddrPointer)
			{
				*pAddrPointer = (PULONG_PTR)&funrav[i] ;
			}
			if (pOriginalFuncAddr)
			{
				*pOriginalFuncAddr = funrav[i] ;
			}
			//�޸ĵ�ַ
			funrav[i] = (ULONG)((ULONG_PTR)ProxyFunc - (ULONG_PTR)hModToHook);
			result = TRUE ;
			//�ָ��ڴ�ҳ������
			VirtualProtect(mbi.BaseAddress,mbi.RegionSize,dwOLD,0);
			printf("[*]Hook ok.\n");
			break;
		}
	}
	return result;
}