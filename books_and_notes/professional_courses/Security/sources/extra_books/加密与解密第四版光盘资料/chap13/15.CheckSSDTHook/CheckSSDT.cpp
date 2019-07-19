/*-----------------------------------------------------------------------
��13��  Hook����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/


// CheckSSDT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "ntapi.h"
#include "PELoader.h"

#pragma comment(lib,"ntdll.lib")

#pragma pack(1)    //SSDT��Ľṹ
typedef struct _ServiceDescriptorEntry {
	DWORD *ServiceTableBase;
	DWORD *ServiceCounterTableBase; //Used only in checked build
	DWORD NumberOfServices;
	BYTE *ParamTableBase;
} ServiceDescriptorTableEntry, *PServiceDescriptorTableEntry;
#pragma pack()

typedef struct _SSDTINFO 
{
	DWORD ServiceIndex;//��������
	DWORD CurrentAddr;//��ǰSSDT���еĵ�ַ
	DWORD OriginalAddr;//ԭʼ��ַ
	BOOL bHooked;//�Ƿ�Hook��
	char FuncName[124];//��������
}SSDTINFO,*PSSDTINFO;


//ZwAccessCheckByTypeResultListAndAuditAlarmByHandle

void PrintZwError( char *funcname, NTSTATUS status );
VOID CheckSSDT();
NTSTATUS ReadKernelMemory(LPVOID BaseAddress,LPVOID lpBuffer,DWORD nSize,LPDWORD lpNumberOfBytesRead);
void PrintZwError( char *funcname, NTSTATUS status );
BOOL GetOSKrnlInfo(
	OUT char *szKrnlPath,
	OUT DWORD *pKernelBase
	);

BOOL EnableDebugPrivilege();
BOOL BuildServiceNameTable(PELoader *pModule);
VOID ShowSSDT();

//�洢SSDT��Ϣ
SSDTINFO g_SSDTInfo[1000];
//�ܷ�����
DWORD g_dwServiceCnt = 0 ;
//��Hook�ķ�����
DWORD g_dwHookedCnt = 0 ;

int main(int argc, char* argv[])
{
	EnableDebugPrivilege();
	CheckSSDT();
	return 0;
}

VOID CheckSSDT()
{
	BOOL bResult = FALSE ;
	DWORD dwBytesReturn = 0 ;
	DWORD i = 0 ;
	//////////////////////////////////////////////////////////////////////////
	//1.����ntdll.dll
	//////////////////////////////////////////////////////////////////////////
	PELoader *ldNtdll = new PELoader;
	char szNtdllPath[MAX_PATH]={0};
	PBYTE pNewLoadedNtdll = NULL ;
	GetSystemDirectory(szNtdllPath,MAX_PATH);
	lstrcat(szNtdllPath,"\\ntdll.dll");

	pNewLoadedNtdll = ldNtdll->LoadPE(szNtdllPath,FALSE,0);
	printf("[*] ���¼��غ��ntdll  BaseAddress = 0x%08X\n",pNewLoadedNtdll);
	
	//2.����������
	BuildServiceNameTable(ldNtdll);
	printf("[*] ϵͳ���������б������.\n");
	delete ldNtdll;
	//ShowSSDT();

	//3.��ȡ��ǰ�ں��ļ���·���ͻ�ַ
	char szOskrnlPath[MAX_PATH]={0};
	DWORD TrueKernelBase = 0 ;
	bResult = GetOSKrnlInfo(szOskrnlPath,&TrueKernelBase);
	if (!bResult)
	{
		printf("[-] �޷���ȡϵͳ�ں��ļ���·���ͼ��ػ�ַ!\n");
		return ;
	}
	printf("[*] �ں�·�� : %s\n",szOskrnlPath);
	printf("[*] �ں˻�ַ : 0x%08X\n",TrueKernelBase);
	
	//4.���ص�ǰ�ں��ļ�ntoskrnl.exe
	PELoader *ldOsKrnl = new PELoader;
	PBYTE pRemapedOsKrnl = ldOsKrnl->LoadPE(szOskrnlPath,TRUE,TrueKernelBase);
	printf("[*] ���¼��غ��OSKrnl  BaseAddress = 0x%08X\n",pRemapedOsKrnl);
	
	ServiceDescriptorTableEntry *pRemapedSDT,*pOriginalSDT,OriginalSDT;
	//��ȡSSDT�ĵ�ַ
	pRemapedSDT = (ServiceDescriptorTableEntry*)ldOsKrnl->_GetProcAddress(pRemapedOsKrnl,"KeServiceDescriptorTable");
	printf("[*] SSDT In ReMaped OSKrnl : 0x%08X\n",pRemapedSDT);
	//��ȡ�����ؼ��غ��KeServiceDescriptorTable�ṹ�ǿյģ���Ҫ����ƫ�Ƽ�����ʵ��SSDT���ڴ��е�λ��

	pOriginalSDT = (ServiceDescriptorTableEntry*)((DWORD)pRemapedSDT - (DWORD)pRemapedOsKrnl + TrueKernelBase );
	printf("[*] SSDT In True    OSKrnl : 0x%08X\n",pOriginalSDT);
	
	//////////////////////////////////////////////////////////////////////////
	//3.��ȡ��ǰSSDT��ַ
	//////////////////////////////////////////////////////////////////////////
	printf("[*] ��ȡ KeServiceDescriptorTable.\n");

	ZeroMemory(&OriginalSDT,sizeof(ServiceDescriptorTableEntry));
	ReadKernelMemory(pOriginalSDT,&OriginalSDT,sizeof(ServiceDescriptorTableEntry),&dwBytesReturn);
	if (dwBytesReturn == 0)
	{
		printf("[-] ��ȡ KeServiceDescriptorTable ʧ��!\n");
		return ;
	}

	printf("[*] KiServiceTable = 0x%08X\n",OriginalSDT.ServiceTableBase);
	
	DWORD OriginalKiServiceTable[1000];
	DWORD RemapedKiServiceTable[1000];

	//��ȡKiServiceTable
	ReadKernelMemory((LPVOID)OriginalSDT.ServiceTableBase,
		OriginalKiServiceTable,
		OriginalSDT.NumberOfServices * sizeof(DWORD) , 
		&dwBytesReturn);

	if (dwBytesReturn == 0)
	{
		printf("[-] ��ȡ KiServiceTable ʧ��!\n");
		return ;
	}
	
	for (i=0;i<g_dwServiceCnt;i++)
	{
		g_SSDTInfo[i].CurrentAddr = OriginalKiServiceTable[i];
	}

	//////////////////////////////////////////////////////////////////////////
	//4.��ȡԭʼSSDT��ַ
	//////////////////////////////////////////////////////////////////////////
	DWORD dwKiServiceTableInRemapedKrnl = (DWORD)OriginalSDT.ServiceTableBase - TrueKernelBase + (DWORD)pRemapedOsKrnl;
	printf("[*] ���غ���ں��� KiServiceTable = 0x%08X\n",dwKiServiceTableInRemapedKrnl);

	memcpy(RemapedKiServiceTable,(PVOID)dwKiServiceTableInRemapedKrnl,OriginalSDT.NumberOfServices * sizeof(DWORD));
	for (i=0;i<g_dwServiceCnt;i++)
	{
		g_SSDTInfo[i].OriginalAddr = RemapedKiServiceTable[i];
	}
	//////////////////////////////////////////////////////////////////////////
	//5.�ԱȽ��
	//////////////////////////////////////////////////////////////////////////
	g_dwHookedCnt = 0 ;
	for (i=0;i<g_dwServiceCnt;i++)
	{
		if ( g_SSDTInfo[i].OriginalAddr != g_SSDTInfo[i].CurrentAddr)
		{
			g_SSDTInfo[i].bHooked = TRUE ;
			g_dwHookedCnt++;
		}
	}

	printf("[*] ��ʾ�����:\n\n");
	ShowSSDT();
	printf("[*] ������. ���з��� %d ������Hook���� %d ����\n",g_dwServiceCnt,g_dwHookedCnt);	
}

BOOL EnableDebugPrivilege() //��������������Ȩ�ޣ�������SE_DEBUG_NAME
{ 
	TOKEN_PRIVILEGES tkp; 
	HANDLE hToken; 
	if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken))     //�򿪵�ǰ����ʧ�� 
		return FALSE; 
	LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tkp.Privileges[0].Luid); //�鿴��ǰȨ��
	tkp.PrivilegeCount = 1; 
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); //����Ȩ�ޣ���������
	return TRUE; 
}

NTSTATUS ReadKernelMemory(LPVOID BaseAddress,LPVOID lpBuffer,DWORD nSize,LPDWORD lpNumberOfBytesRead)
{
	SYSDBG_VIRTUAL sysvir;
	sysvir.Address=BaseAddress;
	sysvir.Buffer=lpBuffer;
	sysvir.Request=nSize;
	return ZwSystemDebugControl(SysDbgReadVirtual,//8
		(PVOID)&sysvir,
		sizeof(SYSDBG_VIRTUAL),
		NULL,
		NULL,
		lpNumberOfBytesRead);
	
}

BOOL GetOSKrnlInfo(
	OUT char *szKrnlPath,
	OUT DWORD *pKernelBase)
{
	BOOL bResult = FALSE ;
	PSYSTEM_MODULE_INFORMATION_ENTRY pSysModuleInfo;
	NTSTATUS status=0;
	char *pBuf=NULL;
	DWORD needlen=0,truelen;
	DWORD Modcnt=0;
	
	ZwQuerySystemInformation(SystemModuleInformation,NULL,0,&needlen);
	//printf("Need length:0x%x\n",needlen);
	truelen=needlen;
	status=ZwAllocateVirtualMemory(NtCurrentProcess(),(PVOID*)&pBuf,0,&truelen,MEM_COMMIT,PAGE_READWRITE);
	if (!NT_SUCCESS(status))
	{
		PrintZwError("ZwAllocateVirtualMemory",status);
		return FALSE;
	}
	//printf("Buf:0x%08x\n",pBuf);
	status=ZwQuerySystemInformation(SystemModuleInformation,(PVOID)pBuf,truelen,&needlen);
	if (!NT_SUCCESS(status))
	{
		PrintZwError("ZwQuerySystemInformation",status);
		return FALSE;
	}
	Modcnt=*(DWORD*)pBuf;
	pSysModuleInfo=(PSYSTEM_MODULE_INFORMATION_ENTRY)(pBuf+sizeof(DWORD));
	if (strstr(
		(strlwr(pSysModuleInfo->ImageName),pSysModuleInfo->ImageName),"nt"))
	{
		*pKernelBase=(DWORD)pSysModuleInfo->Base;
		GetSystemDirectory(szKrnlPath,MAX_PATH);
		lstrcat(szKrnlPath,strrchr(pSysModuleInfo->ImageName,'\\'));
	}
	status=ZwFreeVirtualMemory(NtCurrentProcess(),(PVOID*)&pBuf,&truelen,MEM_RELEASE);
	
	return TRUE;
}

BOOL BuildServiceNameTable(PELoader *pModule)
{
	DWORD i =0 ;
	DWORD nFunCnt = 0 ;
	char *szFunName = NULL;
	DWORD *namerav,*funrav;
	PBYTE ModuleBase = pModule->m_hModule ;
	DWORD dwServiceIndex = 0 ;
	PBYTE pFunAddr = NULL ;
	PIMAGE_EXPORT_DIRECTORY pExportDir = pModule->m_pExportDir;
	namerav=(DWORD*)(ModuleBase+pExportDir->AddressOfNames);
	funrav=(DWORD*)(ModuleBase+pExportDir->AddressOfFunctions);

	g_dwServiceCnt = 0 ;
	for (i=0;i<pExportDir->NumberOfNames;i++)
	{
		szFunName = (char*)ModuleBase + namerav[i] ;
		//ʹ��Zw����ʹ��Nt��ԭ������ЩNt��ͷ�Ĳ���ϵͳ������
		if (memcmp(szFunName,"Zw",2) == 0)
		{
			pFunAddr = ModuleBase + funrav[i];
			dwServiceIndex = *(DWORD*)(pFunAddr + 1);
			//������Ϣ�������
			g_SSDTInfo[dwServiceIndex].ServiceIndex = dwServiceIndex;
			lstrcpy(g_SSDTInfo[dwServiceIndex].FuncName,szFunName);
			memcpy(g_SSDTInfo[dwServiceIndex].FuncName,"Nt",2);//��һ�º�������
			g_dwServiceCnt++ ;
		}
		
	}
	return TRUE;
}

VOID ShowSSDT()
{
	DWORD i = 0 ;
	printf(" ���� ԭʼ��ַ  ��ǰ��ַ Hook ��������\n");
	printf("=========================================================\n");
	for (i=0;i<g_dwServiceCnt;i++)
	{
		printf("[%3d] %08X  %08X %s   %s\n",
			g_SSDTInfo[i].ServiceIndex,
			g_SSDTInfo[i].OriginalAddr,
			g_SSDTInfo[i].CurrentAddr,
			g_SSDTInfo[i].bHooked ? "��":"  ",
			g_SSDTInfo[i].FuncName);
	}
	printf("=========================================================\n");
}

void PrintZwError( char *funcname, NTSTATUS status )
{
    char *errMsg;
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		RtlNtStatusToDosError(status),
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		( LPTSTR )&errMsg, 0, NULL );
    printf("%s ErrCode=0x%08x:%s", funcname,status, errMsg);
    LocalFree( errMsg );
    return;
}