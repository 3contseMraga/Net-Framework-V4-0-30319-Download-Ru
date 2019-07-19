/*-----------------------------------------------------------------------
��13��  Hook����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/


// CheckInlineHook.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "PELoader.h"

//�������½ṹ������һ��InlineHook����Ҫ����Ϣ
typedef struct _HOOK_DATA{
	char szApiName[128];	//��Hook��API����
	char szModuleName[64];	//��Hook��API����ģ�������
	int  HookCodeLen;		//Hook����
	BYTE oldEntry[16];		//����Hookλ�õ�ԭʼָ��
	BYTE newEntry[16];		//����Ҫд��Hookλ�õ���ָ��
	ULONG HookPoint;		//��HOOK��λ��
	PVOID pfnOriginalFun;	//����ԭʼ������ͨ��
	PVOID pfnFakeFun;		//HOOK���˺���
}HOOK_DATA,*PHOOK_DATA;

#define HOOKLEN (5)	//Ҫ��д��ָ��ĳ���
HOOK_DATA MsgBoxHookData;

LPVOID GetAddress(char *,char *);
void makehookentry(PVOID HookPoint);
int WINAPI My_MessageBoxA(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType);
int WINAPI OriginalMessageBox(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType);
BOOL Inline_InstallHook(void);
BOOL Inline_UnInstallHook();
BOOL InstallCodeHook(PHOOK_DATA pHookData);
BOOL UninstallCodeHook(PHOOK_DATA pHookData);

VOID CheckModuleInlineHook(char *szModName);
SIZE_T CompareMemory(VOID  *Source1,VOID  *Source2,SIZE_T  Length);

int main(int argc, char* argv[])
{
	Inline_InstallHook();
	MessageBoxA(NULL,"After  Inline Hook","Test",MB_OK);
	//���user32.dllģ���Inline Hook
	CheckModuleInlineHook("user32.dll");
	return 0;
}

VOID CheckModuleInlineHook(char *szModName)
{
	PELoader Ldr;
	char szModPath[MAX_PATH] = {0};
	HMODULE hTargetMod = GetModuleHandle(szModName);

	GetModuleFileName(hTargetMod,szModPath,MAX_PATH);
	PBYTE pNewUser32 = Ldr.LoadPE(szModPath,TRUE,(DWORD)hTargetMod,TRUE);
	printf("[*] pNewUser32 = 0x%08X\n",pNewUser32);

	PIMAGE_SECTION_HEADER pCodeSec = Ldr.m_pSecHeader ;
	WORD i = 0 ;

	//����.text����ε�λ��
	for (i=0;i<Ldr.m_SectionCnt;i++)
	{
		if (strcmp((char*)pCodeSec->Name,".text") == 0)
		{
			break;
		}
	}

	printf("[*] CodeSection = 0x%08X\n",pCodeSec);
	PBYTE pOriginalCodeStart = Ldr.m_hModule + pCodeSec->VirtualAddress;
	PBYTE pCurrentCodeStart = (PBYTE)hTargetMod + pCodeSec->VirtualAddress ;
	DWORD dwCodeSize = pCodeSec->SizeOfRawData;
	printf("[*] pOriginalCode = 0x%08X  pCurCode = 0x%08X  Size = 0x%X\n",pOriginalCodeStart,pCurrentCodeStart,dwCodeSize);
	
	DWORD dwScanOffset = 0;
	DWORD dwDifCnt = 0 ;
	PBYTE pOrgCode,pCurCode;
	DWORD EqualSize = 0 ;
	BYTE  OrgCode[1024]={0};
	BYTE  CurCode[1024]={0};
	DWORD m = 0 ,n = 0;
	DWORD JmpAddress = 0 ;
	pOrgCode = pOriginalCodeStart;
	pCurCode = pCurrentCodeStart ;

	printf("[*] ��ʼ���Inline Hook.\n");
	while (dwScanOffset < dwCodeSize)
	{
		EqualSize = CompareMemory(pOrgCode,pCurCode,dwCodeSize - dwScanOffset);
		dwScanOffset += EqualSize;
		if (dwScanOffset == dwCodeSize)
		{
			break;
		}
		pOrgCode += EqualSize;
		pCurCode += EqualSize;
		printf("[%2d]Inline Hook Found at : 0x%08X\n",dwDifCnt++,pCurCode);

		//������ͬ�ĵط����������ָ��
		m = 0 ;
		while (*pOrgCode != *pCurCode)
		{
			OrgCode[m] = *pOrgCode++;
			CurCode[m] = *pCurCode++;
			m++;
		}

		//��ʾ��һ���Ĵ���
		printf("ԭʼ���� : ");
		for (n=0;n<m;n++)
		{
			printf("%02X ",OrgCode[n]);
		}
		printf("\n");
		printf("��ǰ���� : ");
		for (n=0;n<m;n++)
		{
			printf("%02X ",CurCode[n]);
		}

		if (CurCode[0] == 0xE9)
		{
			//����jmp��ַ
			JmpAddress = (DWORD)pCurCode + *(DWORD*)(CurCode + 1) + 5 ;
			printf(" => Jmp %08X",JmpAddress);
		}
		printf("\n");
		printf("====================================\n");
		
		dwScanOffset += m;
	}

	printf("[*] Inline Hook������.\n");
}


//�Ƚ������ڴ棬��������ȵ��ֽ�����������RtlCompareMemory��ͬ
SIZE_T CompareMemory(VOID  *Source1,VOID  *Source2,SIZE_T  Length)
{
	SIZE_T Offset = 0 ;
	PBYTE pS1 = (PBYTE)Source1;
	PBYTE pS2 = (PBYTE)Source2;

	while (Offset < Length
		&& *pS1++ == *pS2++)
	{
		Offset++;
	}

	return Offset;
}

//************************************
// Method:    FakeMessageBox
// FullName:  FakeMessageBox
// Purpose:   ȡ��ԭʼMessageBoxA�Ĺ���,HOOK�����ж�MessageBoxA�ĵ��ý�ʵ�ʵ��ñ�����
// Author:    achillis
// Returns:   int WINAPI
// Parameter: HWND hWnd
// Parameter: LPCTSTR lpText
// Parameter: LPCTSTR lpCaption
// Parameter: UINT uType
//************************************
//ע�⺯���Ķ����ԭʼ����һ��Ҫһ���������ǵ���Լ�������������غ󽫳���
int WINAPI My_MessageBoxA(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType)
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
	ret = OriginalMessageBox(hWnd,newText,newCaption,uType);//����ԭMessageBox�������淵��ֵ
	//����ԭ����֮�󣬿��Լ�����OUT(�����)�������и���,�������纯����recv�����Ը��淵�ص�����
	return ret;//�����㻹���Ը���ԭʼ�����ķ���ֵ
}

BOOL Inline_InstallHook()
{
	//׼��Hook
	ZeroMemory(&MsgBoxHookData,sizeof(HOOK_DATA));
	strcpy(MsgBoxHookData.szApiName , "MessageBoxA");
	strcpy(MsgBoxHookData.szModuleName , "user32.dll");
	MsgBoxHookData.HookPoint = (ULONG)GetAddress(MsgBoxHookData.szModuleName,MsgBoxHookData.szApiName);//HOOK�ĵ�ַ
	MsgBoxHookData.pfnOriginalFun = (PVOID)OriginalMessageBox;//����ԭʼ������ͨ��
	MsgBoxHookData.pfnFakeFun = (PVOID)My_MessageBoxA;//Fake
	MsgBoxHookData.HookCodeLen = 5;
	return InstallCodeHook(&MsgBoxHookData);
}


BOOL Inline_UnInstallHook()
{
	return UninstallCodeHook(&MsgBoxHookData);
}
/*
MessageBoxA�Ĵ��뿪ͷ:
77D5050B >  8BFF                   mov edi,edi
77D5050D    55                     push ebp
77D5050E    8BEC                   mov ebp,esp
77D50510    833D 1C04D777 00       cmp dword ptr ds:[gfEMIEnable],0
*/
//����Ҫ����ԭʼ��MessageBoxʱ��ֱ�ӵ��ô˺������ɣ�������ȫ��ͬ
__declspec( naked )
int WINAPI OriginalMessageBox(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType)
{
	_asm
	{
		//��������д���Jmpָ���ƻ���ԭ����ǰ3��ָ��,���������ִ��ԭ������ǰ3��ָ��
		mov edi,edi  //��һ����ʵ���Բ�Ҫ
		push ebp
		mov ebp,esp
		mov eax,MsgBoxHookData.HookPoint //ȡԭʼ��ַ
		add eax,MsgBoxHookData.HookCodeLen //���ϱ�Hook����ĳ���
		jmp eax //����Hook����֮��ĵط����ƹ��Լ���װ��HOOK
	}
}

//��ȡָ��ģ����ָ��API�ĵ�ַ
LPVOID GetAddress(char *dllname,char *funname)
{
	HMODULE hMod=0;
	if (hMod=GetModuleHandle(dllname))
	{
		return GetProcAddress(hMod,funname);
	} 
	else
	{
		hMod=LoadLibrary(dllname);
		return GetProcAddress(hMod,funname);
	}
	
}

/*	
	�������һ��Ҫ����ָ��
	����ҪHOOK�ĳ��Ȳ�ͬ���ֳ������Ĳ�ͬҪ�󣬿��Բ��ò�ͬ��HOOKָ��
	ͨ��Զ��ת����Ҫ5�ֽڣ������ǳ��õ��������:

	5�ֽ�	jmp xxxxxxxx  (0xE9) �󲿷ֺ�����ͷ��������
	77D5050B >  8BFF                   mov edi,edi
	77D5050D    55                     push ebp
	77D5050E    8BEC                   mov ebp,esp
	77D50510    833D 1C04D777 00       cmp dword ptr ds:[gfEMIEnable],0

	6�ֽ�	push xxxxxxxx/ret ʹ���������

	7�ֽ�	mov eax,xxxxxxxx/jmp eax ��������SEH�ĺ�����ͷ,��IsBadReadPtr
	7565D015 >  6A 0C                          push 0C
	7565D017    68 78D06575                    push kernel32.7565D078
	7565D01C    E8 9F45FDFF                    call kernel32.756315C0
	
	��Ҫ�ı��ָ��ȴ�������ʱ������ʹ��nop�����
*/
void InitHookEntry(PHOOK_DATA pHookData)
{
	BYTE Jmp=0xE9;
	if (pHookData==NULL
		|| pHookData->pfnFakeFun==NULL
		|| pHookData->HookPoint==NULL)
	{
		return;
	}

	pHookData->newEntry[0] = 0xE9; //Jmp 
	//������תƫ�Ʋ�д��
	*(ULONG*)(pHookData->newEntry+1) = (ULONG)pHookData->pfnFakeFun - (ULONG)pHookData->HookPoint - 5;//0xE9 ʽjmp�ļ���
	
	
}

BOOL InstallCodeHook(PHOOK_DATA pHookData)
{
	DWORD dwBytesReturned=0;
	DWORD dwOldProtect;
	HANDLE hProcess=GetCurrentProcess();
	MEMORY_BASIC_INFORMATION MBI={0};
	BOOL bResult=FALSE;
	if (pHookData==NULL
		|| pHookData->HookPoint==0
		|| pHookData->pfnFakeFun==NULL
		|| pHookData->pfnOriginalFun==NULL)
	{
		return FALSE;
	}
	LPVOID OriginalAddr=(LPVOID)pHookData->HookPoint;
	printf("Address To HOOK=0x%08X\n",OriginalAddr);
	InitHookEntry(pHookData);//���Inline Hook����
	if(ReadProcessMemory(hProcess,OriginalAddr,pHookData->oldEntry,pHookData->HookCodeLen,&dwBytesReturned))
	{
		if (VirtualQueryEx(hProcess,OriginalAddr,&MBI,sizeof(MEMORY_BASIC_INFORMATION))
			&& VirtualProtectEx(hProcess,MBI.BaseAddress,pHookData->HookCodeLen,PAGE_EXECUTE_READWRITE,&dwOldProtect))
		{
			if (WriteProcessMemory(hProcess,OriginalAddr,pHookData->newEntry,pHookData->HookCodeLen,&dwBytesReturned))
			{
				printf("Install Hook write oK! WrittenCnt=%d\n",dwBytesReturned);
				bResult=TRUE;
			}
			VirtualProtectEx(hProcess,MBI.BaseAddress,pHookData->HookCodeLen,dwOldProtect,&dwOldProtect);
		}
	}
	return bResult;
}

BOOL UninstallCodeHook(PHOOK_DATA HookData)
{
	DWORD dwBytesReturned=0;
	DWORD dwOldProtect;
	HANDLE hProcess=GetCurrentProcess();
	BOOL bResult=FALSE;
	MEMORY_BASIC_INFORMATION MBI={0};
	LPVOID OriginalAddr;
	if (HookData==NULL
		|| HookData->HookPoint==0
		|| HookData->oldEntry[0]==0)
	{
		return FALSE;
	}
	OriginalAddr=(LPVOID)HookData->HookPoint;
	if (VirtualQueryEx(hProcess,OriginalAddr,&MBI,sizeof(MEMORY_BASIC_INFORMATION))
		&& VirtualProtectEx(hProcess,MBI.BaseAddress,HookData->HookCodeLen,PAGE_EXECUTE_READWRITE,&dwOldProtect))
	{
		if (WriteProcessMemory(hProcess,OriginalAddr,HookData->oldEntry,HookData->HookCodeLen,&dwBytesReturned))
		{
			bResult=TRUE;
		}
		VirtualProtectEx(hProcess,MBI.BaseAddress,HookData->HookCodeLen,dwOldProtect,&dwOldProtect);
	}
	return bResult;
}