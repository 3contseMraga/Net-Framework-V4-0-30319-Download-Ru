/*-----------------------------------------------------------------------
��12��  ע�뼼��
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

#include "StdAfx.h"
#include "Hook.h"

ULONG_PTR SkipJmpAddress(ULONG_PTR uAddress);
void makehookentry(PVOID HookPoint);


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
		|| (pfnHead[0] == 0x6A && pfnHead[2] == 0x68)
		|| (pfnHead[0] == 0x68 && pfnHead[5] == 0x68)) //push sehhandler
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
