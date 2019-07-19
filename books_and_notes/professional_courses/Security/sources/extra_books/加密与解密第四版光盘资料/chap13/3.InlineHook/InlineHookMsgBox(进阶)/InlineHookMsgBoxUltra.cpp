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

//������:��user32.dll������MessageBoxA����Inline Hook
//��������ʵ����һ���򵥵�Hook���棬����ͨ����������ĺ궨��ʵ�ֲ�ͬ��InlineHook
//��MessageBoxA����ѡֵΪ2,5,7

#define HOOKCODELEN 7

#include <windows.h>
#include <stdio.h>
#include <CONIO.H>

//�������½ṹ������һ��InlineHook����Ҫ����Ϣ
typedef struct _HOOK_DATA{
	char szApiName[128];	//��Hook��API����
	char szModuleName[128];	//��Hook��API����ģ�������
	int  HookCodeLen;		//Hook����
	BYTE oldEntry[8];		//����Hookλ�õ�ԭʼָ��
	BYTE newEntry[8];		//����Ҫд��Hookλ�õ���ָ��
	BYTE HotPatchCode[8];	//����HotPatchʽHook
	ULONG_PTR HookPoint;		//��HOOK��λ��
	ULONG_PTR JmpBackAddr;		//������ԭ�����еĵ�ַ
	ULONG_PTR pfnTrampolineFun;	//����ԭʼ������ͨ��
	ULONG_PTR pfnDetourFun;		//HOOK���˺���
}HOOK_DATA,*PHOOK_DATA;

#define HOOKLEN (7)	//Ҫ��д��ָ��ĳ���

HOOK_DATA MsgBoxHookData;

ULONG_PTR SkipJmpAddress(ULONG_PTR uAddress);
LPVOID GetAddress(char *,char *);
void makehookentry(PVOID HookPoint);
LPVOID DetourVirtualAllocEx(
	HANDLE hProcess,  // process within which to allocate memory
	LPVOID lpAddress, // desired starting address of allocation
	DWORD dwSize,     // size, in bytes, of region to allocate
	DWORD flAllocationType,  // type of allocation
	DWORD flProtect   // type of access protection
	);

int WINAPI TrampolineMessageBox(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType);
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
	ret = TrampolineMessageBox(hWnd,newText,newCaption,uType);//����ԭMessageBox�������淵��ֵ
	//����ԭ����֮�󣬿��Լ�����OUT(�����)�������и���,�������纯����recv�����Ը��淵�ص�����
	return ret;//�����㻹���Ը���ԭʼ�����ķ���ֵ
}

BOOL Inline_InstallHook()
{
	//׼��Hook
	ZeroMemory(&MsgBoxHookData,sizeof(HOOK_DATA));
	strcpy(MsgBoxHookData.szApiName , "MessageBoxA");
	strcpy(MsgBoxHookData.szModuleName , "user32.dll");
	MsgBoxHookData.HookPoint = (ULONG_PTR)GetAddress(MsgBoxHookData.szModuleName,MsgBoxHookData.szApiName);//HOOK�ĵ�ַ
	MsgBoxHookData.pfnTrampolineFun = (ULONG_PTR)TrampolineMessageBox;//����ԭʼ������ͨ��
	MsgBoxHookData.pfnDetourFun = (ULONG_PTR)My_MessageBoxA;//DetourFun
	MsgBoxHookData.HookCodeLen = HOOKCODELEN;
	return InstallCodeHook(&MsgBoxHookData);
}


BOOL Inline_UnInstallHook()
{
	return UninstallCodeHook(&MsgBoxHookData);
}
/*
VirtualAllocEx�Ĵ��뿪ͷ:
7C809B12 >  6A 10              push 10
7C809B14    68 609B807C        push kernel32.7C809B60
7C809B19    E8 B889FFFF        call kernel32.7C8024D6
*/
//����Ҫ����ԭʼ��MessageBoxʱ��ֱ�ӵ��ô˺������ɣ�������ȫ��ͬ
__declspec( naked )
int WINAPI TrampolineMessageBox(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType)
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
	if (pHookData==NULL
		|| pHookData->pfnDetourFun==NULL
		|| pHookData->HookPoint==NULL
		|| pHookData->HookCodeLen == 0)
	{
		return;
	}
	
	switch(pHookData->HookCodeLen)
	{
	case 2://Hot Patch
		{
			/*
			77D507E5   >-/E9 66086B88      jmp InlineHo.00401050
			77D507EA > $^\EB F9            jmp short USER32.77D507E5
			*/
			pHookData->newEntry[0] = 0xEB; //Jmp -5
			pHookData->newEntry[1] = 0xF9; 
			pHookData->HotPatchCode[0] = 0xE9; //Jmp
			*(ULONG*)(pHookData->HotPatchCode+1) = (ULONG)pHookData->pfnDetourFun - ((ULONG)pHookData->HookPoint - 5)- 5;//0xE9 ʽjmp�ļ���
		}
		break;
	case 5:
		{
			pHookData->newEntry[0] = 0xE9; //Jmp 
			//������תƫ�Ʋ�д��
			*(ULONG*)(pHookData->newEntry+1) = (ULONG)pHookData->pfnDetourFun - (ULONG)pHookData->HookPoint - 5;//0xE9 ʽjmp�ļ���
		}
		break;
	case 6:
		{
			/*
			0040E9D1 >    68 44332211      push 11223344
			0040E9D6      C3               retn
			*/
			memcpy(pHookData->newEntry,"\x68\x44\x33\x22\x11\xC3",6);
			*(ULONG*)(pHookData->newEntry+1) = (ULONG)pHookData->pfnDetourFun ;
		}
		break;
	case 7:
		/*
		7C809B12 >  B8 44332211        mov eax,11223344
		7C809B17    FFE0               jmp eax
		*/
		{
			memcpy(pHookData->newEntry,"\xB8\x44\x33\x22\x11\xFF\xE0",7);
			*(ULONG*)(pHookData->newEntry+1) = (ULONG)pHookData->pfnDetourFun ;
		}
	    break;
	default:
	    break;
	}
	
	
}

BOOL InstallCodeHook(PHOOK_DATA pHookData)
{
	DWORD dwBytesReturned=0;
	DWORD dwOldProtect;
	PBYTE pfnHead = NULL;
	PBYTE pAddrToWrite = NULL ;
	BOOL bResult=FALSE;
	MEMORY_BASIC_INFORMATION MBI={0};
	
	if (pHookData==NULL
		|| pHookData->HookPoint==0
		|| pHookData->pfnDetourFun==NULL
		|| pHookData->pfnTrampolineFun==NULL)
	{
		return FALSE;
	}

	pHookData->pfnTrampolineFun = SkipJmpAddress(pHookData->pfnTrampolineFun);
	pHookData->HookPoint = SkipJmpAddress(pHookData->HookPoint); //���������ͷ����ת����ô��������
	pHookData->JmpBackAddr = pHookData->HookPoint + pHookData->HookCodeLen ;
	pfnHead = (PBYTE)pHookData->HookPoint;
	printf("Address To HOOK=0x%08X\n",pfnHead);

	//����Ƿ�Hook��
	if (memcmp(pfnHead,"\x8B\xFF\x55\x8B\xEC",5) == 0 //push ebp,mov ebp,esp
		|| (pfnHead[0] == 0x6A && pfnHead[5] == 0x68)) //push sehhandler
	{
		//δHook�����ܼ���
		InitHookEntry(pHookData);//���Inline Hook����
	}
	else
	{
		printf("Addr 0x%X already hooked.\n");
	}
	

	//��ȡԭ��������
	memcpy(pHookData->oldEntry,(void*)pHookData->HookPoint,8);
	//�������HotPatch Hook����Ҫ��ԭ������ָ���滻��Trampoline�е�nop
	if (pHookData->HookCodeLen != 2)
	{
		pAddrToWrite = (PBYTE)pHookData->pfnTrampolineFun ;
		if (VirtualQuery(pAddrToWrite,&MBI,sizeof(MEMORY_BASIC_INFORMATION))
			&& VirtualProtect(MBI.BaseAddress,pHookData->HookCodeLen,PAGE_EXECUTE_READWRITE,&dwOldProtect))
		{
			memcpy(pAddrToWrite,pHookData->oldEntry,pHookData->HookCodeLen);
			VirtualProtect(MBI.BaseAddress,pHookData->HookCodeLen,dwOldProtect,&dwOldProtect);
		}
	}

	//���濪ʼHook
	if (pHookData->HookCodeLen == 2)
	{
		pAddrToWrite = (PBYTE)pHookData->HookPoint - 5 ;
	}
	else
	{
		pAddrToWrite = (PBYTE)pHookData->HookPoint;
	}

	if (VirtualQuery(pAddrToWrite,&MBI,sizeof(MEMORY_BASIC_INFORMATION))
		&& VirtualProtect(MBI.BaseAddress,pHookData->HookCodeLen,PAGE_EXECUTE_READWRITE,&dwOldProtect))  //ʵ����������޸���ҳ������
	{
		if (pHookData->HookCodeLen == 2)
		{
			//�����jmpָ��
			memcpy(pAddrToWrite,pHookData->HotPatchCode,5);
			pAddrToWrite += 5;
		}
		memcpy(pAddrToWrite,pHookData->newEntry,pHookData->HookCodeLen);
		VirtualProtect(MBI.BaseAddress,pHookData->HookCodeLen,dwOldProtect,&dwOldProtect);
	}

	return bResult;
}

BOOL UninstallCodeHook(PHOOK_DATA pHookData)
{
	DWORD dwBytesReturned=0;
	DWORD dwOldProtect;
	BOOL bResult=FALSE;
	MEMORY_BASIC_INFORMATION MBI={0};
	PBYTE pAddrToWrite = NULL ;
	if (pHookData==NULL
		|| pHookData->HookPoint==0
		|| pHookData->oldEntry[0]==0)
	{
		return FALSE;
	}
	pAddrToWrite=(PBYTE)pHookData->HookPoint;
	if (VirtualQuery(pAddrToWrite,&MBI,sizeof(MEMORY_BASIC_INFORMATION))
		&& VirtualProtect(MBI.BaseAddress,pHookData->HookCodeLen,PAGE_EXECUTE_READWRITE,&dwOldProtect))
	{
		memcpy(pAddrToWrite,pHookData->oldEntry,pHookData->HookCodeLen);
		bResult=TRUE;
		VirtualProtect(MBI.BaseAddress,pHookData->HookCodeLen,dwOldProtect,&dwOldProtect);
	}
	return bResult;
}
