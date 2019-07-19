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

/////////////////////////////////////////////////////////////////
//         Author: achillis(���½���)                          //
//         Blog  : http://www.cnblogs.com/achillis/            //
//         QQ    : 344132161                                   //
//         Email : achillis@126.com                            //
//              ת���뱣��������Ϣ                             //
/////////////////////////////////////////////////////////////////

//������:��user32.dll������MessageBoxA����Inline Hook
//��������д�ڴ治��ʹ�÷�����VirtualProtect������ֱ��ʹ��WriteProcessMemory

#include <windows.h>
#include <stdio.h>
#include <CONIO.H>

//�������½ṹ������һ��InlineHook����Ҫ����Ϣ
typedef struct _HOOK_DATA{
	char szApiName[128];	//��Hook��API����
	char szModuleName[64];	//��Hook��API����ģ�������
	int  HookCodeLen;		//Hook����
	BYTE oldEntry[16];		//����Hookλ�õ�ԭʼָ��
	BYTE newEntry[16];		//����Ҫд��Hookλ�õ���ָ��
	ULONG_PTR HookPoint;		//��HOOK��λ��
	ULONG_PTR JmpBackAddr;		//������ԭ�����е�λ��
	ULONG_PTR pfnTrampolineFun;	//����ԭʼ������ͨ��
	ULONG_PTR pfnDetourFun;		//HOOK���˺���
}HOOK_DATA,*PHOOK_DATA;

#define HOOKLEN (5)	//Ҫ��д��ָ��ĳ���
HOOK_DATA MsgBoxHookData;

ULONG_PTR SkipJmpAddress(ULONG_PTR uAddress);
LPVOID GetAddress(char *,char *);
void makehookentry(PVOID HookPoint);
int WINAPI My_MessageBoxA(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType);
int WINAPI OriginalMessageBox(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType);
BOOL Inline_InstallHook(void);
BOOL Inline_UnInstallHook();
BOOL InstallCodeHook(PHOOK_DATA pHookData);
BOOL UninstallCodeHook(PHOOK_DATA pHookData);

int main(int argc, char* argv[])
{
	MessageBoxA(NULL,"Before Inline Hook","Test",MB_OK);
	Inline_InstallHook();
	MessageBoxA(NULL,"After  Inline Hook","Test",MB_OK);
	Inline_UnInstallHook();
	MessageBoxA(NULL,"After  Inline Hook Unhooked","Test",MB_OK);
	return 0;
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
	MsgBoxHookData.HookCodeLen = 5;
	MsgBoxHookData.HookPoint = (ULONG_PTR)GetAddress(MsgBoxHookData.szModuleName,MsgBoxHookData.szApiName);//HOOK�ĵ�ַ
	MsgBoxHookData.pfnTrampolineFun = (ULONG_PTR)OriginalMessageBox;//����ԭʼ������ͨ��
	MsgBoxHookData.pfnDetourFun = (ULONG_PTR)My_MessageBoxA;//Fake
	
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
		jmp MsgBoxHookData.JmpBackAddr //����Hook����֮��ĵط����ƹ��Լ���װ��HOOK
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
	ʹ�õ���5�ֽڵ�Jmp
*/
void InitHookEntry(PHOOK_DATA pHookData)
{
	if (pHookData==NULL
		|| pHookData->pfnDetourFun==NULL
		|| pHookData->HookPoint==NULL)
	{
		return;
	}

	pHookData->newEntry[0] = 0xE9; //Jmp 
	//������תƫ�Ʋ�д��
	*(ULONG*)(pHookData->newEntry+1) = (ULONG)pHookData->pfnDetourFun - (ULONG)pHookData->HookPoint - 5;//0xE9 ʽjmp�ļ���
	
	
}

ULONG_PTR SkipJmpAddress(ULONG_PTR uAddress)
{
	ULONG_PTR TrueAddress = 0 ;
	PBYTE pFn = (PBYTE)uAddress;
	
	if (memcmp(pFn,"\xFF\x25",2) == 0)
	{
		TrueAddress = *(ULONG_PTR*)(pFn + 2);
		return TrueAddress;
	}
	
	if (pFn[0] == 0xE9)
	{
		TrueAddress = (ULONG_PTR)pFn + *(ULONG_PTR*)(pFn + 1) + 5 ;
		return TrueAddress;
	}
	
	if (pFn[0] == 0xEB)
	{
		TrueAddress = (ULONG_PTR)pFn + pFn[1] + 2 ;
		return TrueAddress;
	}
	
	return (ULONG_PTR)uAddress;
}

BOOL InstallCodeHook(PHOOK_DATA pHookData)
{
	DWORD dwBytesReturned=0;
	HANDLE hProcess=GetCurrentProcess();
	BOOL bResult=FALSE;
	if (pHookData==NULL
		|| pHookData->HookPoint==0
		|| pHookData->pfnDetourFun == NULL
		|| pHookData->pfnTrampolineFun ==NULL)
	{
		return FALSE;
	}
	pHookData->pfnTrampolineFun = SkipJmpAddress(pHookData->pfnTrampolineFun);
	pHookData->HookPoint = SkipJmpAddress(pHookData->HookPoint); //���������ͷ����ת����ô��������
	pHookData->JmpBackAddr = pHookData->HookPoint + pHookData->HookCodeLen ;
	LPVOID OriginalAddr = (LPVOID)pHookData->HookPoint ;
	printf("Address To HOOK=0x%08X\n",OriginalAddr);
	InitHookEntry(pHookData);//���Inline Hook����
	if(ReadProcessMemory(hProcess,OriginalAddr,pHookData->oldEntry,pHookData->HookCodeLen,&dwBytesReturned))
	{
		if (WriteProcessMemory(hProcess,OriginalAddr,pHookData->newEntry,pHookData->HookCodeLen,&dwBytesReturned))
		{
			printf("Install Hook write oK! WrittenCnt=%d\n",dwBytesReturned);
			bResult=TRUE;
		}
	}
	return bResult;
}

BOOL UninstallCodeHook(PHOOK_DATA HookData)
{
	DWORD dwBytesReturned=0;
	HANDLE hProcess=GetCurrentProcess();
	BOOL bResult=FALSE;
	LPVOID OriginalAddr;
	if (HookData==NULL
		|| HookData->HookPoint==0
		|| HookData->oldEntry[0]==0)
	{
		return FALSE;
	}
	OriginalAddr=(LPVOID)HookData->HookPoint;
	bResult = WriteProcessMemory(hProcess,OriginalAddr,HookData->oldEntry,HookData->HookCodeLen,&dwBytesReturned) ;
	return bResult;
}