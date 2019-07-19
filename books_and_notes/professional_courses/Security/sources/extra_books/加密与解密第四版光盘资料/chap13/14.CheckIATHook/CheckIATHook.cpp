/*-----------------------------------------------------------------------
��13��  Hook����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/


// CheckIATHook.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <imagehlp.h>
#include "PELoader.h"
#pragma comment(lib,"imagehlp.lib")



//��MessageBoxA��ԭ�Ͷ���һ������ָ������
typedef int 
(WINAPI *PFN_MessageBoxA)(
	HWND hWnd,          // handle of owner window
	LPCTSTR lpText,     // address of text in message box
	LPCTSTR lpCaption,  // address of title of message box
	UINT uType          // style of message box
	);


int WINAPI My_MessageBoxA(
	HWND hWnd,          // handle of owner window
	LPCTSTR lpText,     // address of text in message box
	LPCTSTR lpCaption,  // address of title of message box
	UINT uType          // style of message box
	);

//�������¹�ϵ
//*(*pThunkPointer) == *pOriginalFuncAddr ;
BOOL InstallModuleIATHook(
	HMODULE hModToHook,
	char *szModuleName, 
	char *szFuncName,
	PVOID ProxyFunc,
	PULONG *pThunkPointer,
	ULONG *pOriginalFuncAddr
	);

VOID ShowMsgBox(char *szMsg);
BOOL IAT_InstallHook();
VOID IAT_UnInstallHook();

//����ԭʼMessageBoxA�ĵ�ַ
PFN_MessageBoxA OldMessageBox=NULL;
//ָ��IAT��pThunk�ĵ�ַ
PULONG g_PointerToIATThunk = NULL;

VOID CheckModuleIATHook(HMODULE hMod);

int main(int argc, char *argv[ ])
{
	IAT_InstallHook();
	printf("[*] IAT Hook��װ���. \n");
	ShowMsgBox("After  IAT Hook");
	
	CheckModuleIATHook(GetModuleHandle(NULL));
	return 0;
}

VOID CheckModuleIATHook(HMODULE hMod)
{
	PELoader Ldr;
	char szFilePath[MAX_PATH]={0};
	char *szImpModeName = NULL ;
	char *szFunName = NULL;

	printf("[*] ׼�����IAT Hook.\n");
	GetModuleFileName(hMod,szFilePath,MAX_PATH);
	PBYTE pNewMod = Ldr.LoadPE(szFilePath,FALSE,0,TRUE);//�ض�λ����Ҫ����
	if (pNewMod == NULL)
	{
		printf("[*] �ؼ���PEʧ��!\n");
		return ;
	}

	printf("[*] �ؼ��غ��Base = 0x%08X\n",pNewMod);
	
	PIMAGE_IMPORT_DESCRIPTOR  pOrgImportDescriptor = Ldr.m_pImportDesp;
	PIMAGE_IMPORT_DESCRIPTOR  pCurImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)Ldr.m_pImportDesp - (DWORD)Ldr.m_hModule + (DWORD)hMod );
	PIMAGE_THUNK_DATA pOrgThunk,pCurThunk,pNameThunk;
	PIMAGE_IMPORT_BY_NAME pImpName = NULL ;
	
	printf("[*] ��ʼ���IAT Hook.\n");
	//ͬ���Ƚ�
	while (pOrgImportDescriptor->FirstThunk)
	{
		szImpModeName = (char*)pNewMod + pOrgImportDescriptor->Name;
		printf(" ->���ڼ�⵼��ģ�� : %s\n",szImpModeName);
		pOrgThunk = (PIMAGE_THUNK_DATA)((BYTE *)pNewMod + pOrgImportDescriptor->FirstThunk);
		pNameThunk = (PIMAGE_THUNK_DATA)((BYTE *)pNewMod + pOrgImportDescriptor->OriginalFirstThunk);
		pCurThunk = (PIMAGE_THUNK_DATA)((BYTE *)hMod + pOrgImportDescriptor->FirstThunk);
		while (pOrgThunk->u1.Function)
		{
			if (pOrgThunk->u1.Function != pCurThunk->u1.Function)
			{
				//����û���жϰ���ŵ���������ֻ�����˰����Ƶ���
				pImpName = (PIMAGE_IMPORT_BY_NAME)((char*)pNewMod + pNameThunk->u1.AddressOfData) ;
				szFunName = (char*)pImpName->Name ;
				printf("   Found IAT Hook. ԭʼ��ַ= 0x%08X  ��ǰ��ַ = 0x%08X ������:%s\n",
					pOrgThunk->u1.Function,
					pCurThunk->u1.Function,
					szFunName);
			}
			pNameThunk++;
			pOrgThunk++;
			pCurThunk++;
		}
		pOrgImportDescriptor++;
	}

	printf("[*] IAT Hook������!\n");


	
}

VOID ShowMsgBox(char *szMsg)
{
	MessageBoxA(NULL,szMsg,"Test",MB_OK);
}


//֮���԰�������õ�������һ�������У�����ΪReleaseģʽ�¶Ե��ý������Ż�,�ڶ��ε���ʱֱ�Ӳ����˼Ĵ���Ѱַ�����ǵ����
//��ˣ���������һ�������п��Ա�����������

int WINAPI My_MessageBoxA(
	HWND hWnd,          // handle of owner window
	LPCTSTR lpText,     // address of text in message box
	LPCTSTR lpCaption,  // address of title of message box
	UINT uType          // style of message box
	)
{	
	//���������Զ�ԭʼ���������������
	int ret;
	char newText[1024]={0};
	char newCaption[256]="pediy.com";
	printf("���˵���MessageBox!\n");
	//�ڵ���ԭ����֮ǰ�����Զ�IN(������)�������и���
	lstrcpy(newText,lpText);//Ϊ��ֹԭ�����ṩ�Ļ��������������︴�Ƶ������Լ���һ�����������ٽ��в���
	lstrcat(newText,"\n\tMessageBox Hacked by pediy.com!");//�۸���Ϣ������
	uType|=MB_ICONERROR;//����һ������ͼ��
	ret = OldMessageBox(hWnd,newText,newCaption,uType);//����ԭMessageBox�������淵��ֵ
	//����ԭ����֮�󣬿��Լ�����OUT(�����)�������и���,�������纯����recv�����Ը��淵�ص�����
	return ret;//�����㻹���Ը���ԭʼ�����ķ���ֵ
	
}

BOOL IAT_InstallHook()
{
	BOOL bResult = FALSE ;
	HMODULE hCurExe = GetModuleHandle(NULL);
	PULONG pt ;
	ULONG OrginalAddr;
	bResult = InstallModuleIATHook(hCurExe,"user32.dll","MessageBoxA",(PVOID)My_MessageBoxA,&pt,&OrginalAddr);
	if (bResult)
	{
		printf("[*]Hook��װ���! pThunk=0x%08X  OriginalAddr = 0x%08X\n",pt,OrginalAddr);
		g_PointerToIATThunk = pt ;
		OldMessageBox = (PFN_MessageBoxA)OrginalAddr ;
	}
	return bResult;
	
}

VOID IAT_UnInstallHook()
{
	
	DWORD dwOLD;
	MEMORY_BASIC_INFORMATION  mbi;
	if (g_PointerToIATThunk)
	{
		//��ѯ���޸��ڴ�ҳ������
		VirtualQuery(g_PointerToIATThunk,&mbi,sizeof(mbi));
		VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_EXECUTE_READWRITE,&dwOLD);
		//��ԭʼ��MessageBoxA��ַ����IAT��
		*g_PointerToIATThunk = (ULONG)OldMessageBox;
		//�ָ��ڴ�ҳ������
		VirtualProtect(mbi.BaseAddress,mbi.RegionSize,dwOLD,0);
	}

}

//************************************
// FullName:    InstallModuleIATHook
// Description: Ϊָ��ģ�鰲װIAT Hook
// Returns:     BOOL
// Parameter:   HMODULE hModToHook , ��Hook��ģ���ַ
// Parameter:   char * szModuleName , Ŀ�꺯������ģ�������
// Parameter:   char * szFuncName , Ŀ�꺯��������
// Parameter:   PVOID DetourFunc , Detour������ַ
// Parameter:   PULONG * pThunkPointer , ���Խ���ָ���޸ĵ�λ�õ�ָ��
// Parameter:   ULONG * pOriginalFuncAddr , ���Խ���ԭʼ������ַ
//************************************
BOOL InstallModuleIATHook(
	HMODULE hModToHook,// IN
	char *szModuleName,// IN
	char *szFuncName,// IN
	PVOID DetourFunc,// IN
	PULONG *pThunkPointer,//OUT
	ULONG *pOriginalFuncAddr//OUT
	)
{
	PIMAGE_IMPORT_DESCRIPTOR  pImportDescriptor;
	PIMAGE_THUNK_DATA         pThunkData;
	ULONG ulSize;
	HMODULE hModule=0;
	ULONG TargetFunAddr;
	PULONG lpAddr;
	char *szModName;
	BOOL result = FALSE ;
	BOOL bRetn = FALSE;

	hModule = LoadLibrary(szModuleName);
	if (hModule == NULL)
	{
		return FALSE;
	}
	TargetFunAddr = (ULONG)GetProcAddress(hModule,szFuncName);
	if (TargetFunAddr == 0)
	{
		return FALSE;
	}
	//printf("[*]Address of %s:0x%08X\n",szFuncName,TargetFunAddr);
	//printf("[*]Module To Hook at Base:0x%08X\n",hModToHook);
	pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(hModToHook, TRUE,IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);
	if (pImportDescriptor == NULL)
	{
		return FALSE;
	}
	//printf("[*]Find ImportTable,offset:0x%08X\n",pImportDescriptor);
	//PE���������ɺ�OriginalFirstThunk��������Ч�ģ�ֻ��ʹ��FirstThunk
	while (pImportDescriptor->FirstThunk)
	{
		szModName = (char*)((PBYTE)hModToHook+pImportDescriptor->Name) ;
		//printf("[*]Cur Module Name:%s\n",szModName);
		if (stricmp(szModName,szModuleName) != 0)
		{
			//printf("[*]Module Name does not match, search next...\n");
			pImportDescriptor++;
			continue;
		}
		//pThunkDataָ���ڴ��е�IAT��ַ��
		pThunkData = (PIMAGE_THUNK_DATA)((BYTE *)hModToHook + pImportDescriptor->FirstThunk);
		while(pThunkData->u1.Function)
		{
			lpAddr = (ULONG*)pThunkData;
			//�ҵ��˵�ַ
			if((*lpAddr) == TargetFunAddr)
			{
				//printf("[*]Find target address!\n");
				//ͨ������µ���������ڴ�ҳ����ֻ���ģ������Ҫ���޸��ڴ�ҳ������Ϊ��д
				DWORD dwOldProtect;
				MEMORY_BASIC_INFORMATION  mbi;
				VirtualQuery(lpAddr,&mbi,sizeof(mbi));
				bRetn = VirtualProtect(mbi.BaseAddress,mbi.RegionSize,
					PAGE_EXECUTE_READWRITE,&dwOldProtect);
				if (bRetn)
				{
					//�ڴ�ҳ�����޸ĳɹ�,������һ������,�ȱ���ԭʼ����
					if (pThunkPointer != NULL)
					{
						*pThunkPointer = lpAddr ;
					}
					if (pOriginalFuncAddr != NULL)
					{
						*pOriginalFuncAddr = *lpAddr ;
					}
					//�޸ĵ�ַ
					*lpAddr = (ULONG)DetourFunc;
					result = TRUE ;
					//�ָ��ڴ�ҳ������
					VirtualProtect(mbi.BaseAddress,mbi.RegionSize,dwOldProtect,0);
					//printf("[*]Hook ok.\n");
				}
				
				break;	
			}
			//---------
			pThunkData++;
		}
		pImportDescriptor++;
	}
	
	FreeLibrary(hModule);
	return result;
}
