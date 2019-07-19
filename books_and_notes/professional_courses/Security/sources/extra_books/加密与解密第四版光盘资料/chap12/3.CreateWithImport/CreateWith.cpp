/*-----------------------------------------------------------------------
��12��  ע�뼼��
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// CreateWith.cpp : Defines the entry point for the console application.
// Tested On:WinXP SP3/Win7 x86/Win7 x64

#include "stdafx.h"
#include "Image.h"
#include <stdlib.h>
#include <Psapi.h>
#include <tlhelp32.h>

#pragma comment(lib,"psapi.lib")

typedef BOOL 
(WINAPI *PDETOUR_CREATE_PROCESS_ROUTINE)(
	LPCTSTR lpApplicationName,// pointer to name of executable module
	LPTSTR lpCommandLine,  // pointer to command line string
	LPSECURITY_ATTRIBUTES lpProcessAttributes,  // process security attributes
	LPSECURITY_ATTRIBUTES lpThreadAttributes,   // thread security attributes
	BOOL bInheritHandles,  // handle inheritance flag
	DWORD dwCreationFlags, // creation flags
	LPVOID lpEnvironment,  // pointer to new environment block
	LPCTSTR lpCurrentDirectory,   // pointer to current directory name
	LPSTARTUPINFO lpStartupInfo,  // pointer to STARTUPINFO
	LPPROCESS_INFORMATION lpProcessInformation  // pointer to PROCESS_INFORMATION
	);

BOOL DetourCreateProcessWithDll(
	LPCTSTR lpApplicationName,
	LPTSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCTSTR lpCurrentDirectory,
	LPSTARTUPINFO lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation,
	LPCSTR lpDllName,
	PDETOUR_CREATE_PROCESS_ROUTINE pfCreateProcess
	);

DWORD ProcesstoPid(char *Processname);
ULONG_PTR FindImageBase(HANDLE hProc,LPSTR lpCommandLine);


int main(int argc, char* argv[])
{
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
	
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

#ifdef _WIN64
	DetourCreateProcessWithDll(
		NULL,
		"F:\\Program2016\\DllInjection\\HostProc64.exe",
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi,
		"MsgDll64.dll",
		NULL);
#else
	DetourCreateProcessWithDll(
		NULL,
		//"D:\\killvirus\\PCHunter_free\\PCHunter32.exe",
		"F:\\Program2016\\DllInjection\\HostProc.exe",
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi,
		"MsgDll.dll",
		NULL);
#endif
	return 0;
}



BOOL DetourCreateProcessWithDll(
	LPCTSTR lpApplicationName,
	LPTSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCTSTR lpCurrentDirectory,
	LPSTARTUPINFO lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation,
	LPCSTR szDllName,
	PDETOUR_CREATE_PROCESS_ROUTINE pfCreateProcess
	)
{
	CImage Image;
	LONG Status = 0 ;
	BOOL bResult = FALSE ;
	WORD i = 0 ;
	SIZE_T dwIoCnt = 0 ;
	char szErrMsg[1024]={0};
	PIMAGE_SECTION_HEADER pImpSecHeader,pNewSecHeader = NULL,pTargetSecHeader = NULL;
	DWORD dwOldIIDCnt = 0 ,dwNewIIDCnt = 0 ;
	DWORD dwOldIIDSize = 0, dwNewIIDSize = 0 ;
	DWORD dwVAToStoreNewIID = 0 ; //��IID����Ĵ洢λ��
	BOOL bUseOriginalSection = FALSE ; //�Ƿ�ʹ�����½�
	PROCESS_INFORMATION pi={0};
	ULONG_PTR ProcImageBase = 0 ;
	char szDllExportFunName[]= "Msg";
	HANDLE hFile  = INVALID_HANDLE_VALUE;
	PBYTE ImageBase = 0;
	DWORD dwnewThunkDataSize = 0 ; //��IID���ThunkData�Ĵ洢λ��
	DWORD dwNewThunkDataVA = 0 ;//��IID���ThunkData�Ĵ洢λ��
	DWORD dwSizeNeed = 0 ;
	DWORD dwThunkDataOffsetByIID = 0 ;
	BYTE IIDThunkBuffer[0x200]={0};
	DWORD dwOldProtect = 0 ;
	
	printf("[*] Path = %s\n",lpCommandLine);
	if (pfCreateProcess == NULL)
	{
		pfCreateProcess = CreateProcessA;
	}
	
	DWORD dwNewCreationFlags = dwCreationFlags + CREATE_SUSPENDED ;
	
	bResult = pfCreateProcess(
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwNewCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		&pi
		);
	
	//����Ƿ񱻸�Ⱦ��
	if (!bResult)
	{
		printf("[-] �������� %s ʧ��!\n",lpCommandLine);
		return FALSE;
	}
	
	
	//���̴����ɹ�����ȡImageBase����ȡPEͷ
	//���ڴ�ʱ������Ldr����û�����������޷�ʹ��EnumProcessModule
	//����������ASLR������£����̲�һ���ܹ�������PEͷ��ָ����ImageBase��
	ImageBase = (PBYTE)FindImageBase(pi.hProcess,lpCommandLine);
	if (ImageBase == NULL)
	{
		printf("[-] �޷���ȡ���̵�ImageBase!\n",lpCommandLine);
		return FALSE;
	}

	printf("[*] ImageBase = 0x%p\n",ImageBase);
	
	//��ȡĿ����̵�PEͷ
	bResult = Image.AttachToProcess(pi.hProcess,ImageBase);
	if (!bResult)
	{
		printf("[-] ��ȡĿ�����exeӳ���PEͷʧ��!\n");
		return FALSE;
	}
	printf("[*] PEͷ��ȡ�ɹ�!\n");
	printf("[*] ��ǰ�������Ϣ VA = 0x%p Size = 0x%X\n",Image.m_pImpDataDir->VirtualAddress,Image.m_pImpDataDir->Size);
	dwOldIIDSize = Image.m_pImpDataDir->Size ;
	dwOldIIDCnt = dwOldIIDSize / sizeof(IMAGE_IMPORT_DESCRIPTOR) ;
	dwNewIIDCnt = dwOldIIDCnt + 1;
	dwNewIIDSize = dwNewIIDCnt * sizeof(IMAGE_IMPORT_DESCRIPTOR) ;
	printf("[*] dwOldIIDCnt = %d  Size = 0x%X\n",dwOldIIDCnt,dwOldIIDSize);
	printf("[*] dwNewIIDCnt = %d  Size = 0x%X\n",dwNewIIDCnt,dwNewIIDSize);
	
	pImpSecHeader = Image.LocateSectionByRVA(Image.m_pImpDataDir->VirtualAddress);
	printf("[*] ��������ڽ�  %s  RawOffset = 0x%X Size = 0x%X\n",
		pImpSecHeader->Name,
		pImpSecHeader->PointerToRawData,
		pImpSecHeader->SizeOfRawData);
	
	
	dwnewThunkDataSize = sizeof(ULONG_PTR)*4 + strlen(szDllName) + 1 + sizeof(WORD) + strlen(szDllExportFunName) + 1 ;
	dwThunkDataOffsetByIID = ALIGN_SIZE_UP(dwNewIIDSize,sizeof(ULONG_PTR)) ;

	//����Ĵ�С���µ����IID�ṹ�Ĵ�С����ThunkData�Ĵ�С
	dwSizeNeed = dwThunkDataOffsetByIID + dwnewThunkDataSize ;
	//���ڶ������ڴ��в��������ؿ���ԭ���ļ��еĴ�С������ֱ�������½�
	pNewSecHeader = Image.AddNewSectionToMemory(".Patch",dwSizeNeed);
	printf("[*] �½�������!\n");
	printf("[*] NewSection VA = 0x%X VirtualSize = 0x%X RawSize = 0x%X\n",
		ImageBase + pNewSecHeader->VirtualAddress ,pNewSecHeader->Misc.VirtualSize,pNewSecHeader->SizeOfRawData);
	
	dwVAToStoreNewIID = pNewSecHeader->VirtualAddress ;
	pTargetSecHeader = pNewSecHeader;

	//����ԭ�����
	PIMAGE_IMPORT_DESCRIPTOR pOldImpDesp = Image.m_pImportDesp;
	PIMAGE_IMPORT_DESCRIPTOR pBuildNewImpDesp = (PIMAGE_IMPORT_DESCRIPTOR)malloc(dwSizeNeed);
	ZeroMemory(pBuildNewImpDesp,dwSizeNeed);

	//����ԭ���ĵ����
	bResult = ReadProcessMemory(pi.hProcess,
		ImageBase + Image.m_pImpDataDir->VirtualAddress,
		pBuildNewImpDesp,
		dwOldIIDSize,
		&dwIoCnt);

	if (!bResult)
	{
		printf("[-] ��ȡĿ�����ԭʼ�����ʧ��!\n");
		return FALSE;
	}
	//memcpy(pBuildNewImpDesp,pOldImpDesp,dwOldIIDSize);
	printf("[*] ԭ�����IID�ṹ�������.\n");

	//ָ��һ������ӵ�IID��,�Ժ����
	PIMAGE_IMPORT_DESCRIPTOR pNewImpEntry = pBuildNewImpDesp + dwOldIIDCnt - 1;
	//׼������µ�������Thunks��Ϣ
	PULONG_PTR pOriginalFirstThunk = (PULONG_PTR)IIDThunkBuffer;
	ZeroMemory(pOriginalFirstThunk,dwOldIIDSize);
	//�����������ݣ���һ���Ժ���䣬�ڶ�����0��Ϊ�������
	PULONG_PTR pFirstThunk = pOriginalFirstThunk + 2 ;
	//������������,��һ���Ժ���䣬�ڶ�����0��Ϊ������ǣ�֮����ΪDll����
	PCHAR  pDllName = (PCHAR)(pFirstThunk + 2);
	//����dll����
	strcpy(pDllName,szDllName);
	
	SIZE_T DllNameLen = strlen(szDllName);
	pDllName[DllNameLen] = 0;
	//��������Ϊһ��PIMPORT_BY_NAME�ṹ
	PIMAGE_IMPORT_BY_NAME pImpName = (PIMAGE_IMPORT_BY_NAME)(pDllName + ALIGN_SIZE_UP(DllNameLen + 1,sizeof(ULONG_PTR)));
	//�����
	pImpName->Hint = 0;
	strcpy((char*)pImpName->Name,szDllExportFunName);
	printf("[*] �µ����IID�ӽṹ�������.\n");
	
	//�������λ��
	PBYTE pEnd = (PBYTE)pImpName + sizeof(pImpName->Hint) + strlen((char*)pImpName->Name) + 1;
	//������ռ�õĿռ��С
	DWORD dwNewIIDEntrySizeUsed = (DWORD)(pEnd - (PBYTE)pOriginalFirstThunk);
	printf("[*] ��IID��Առ�õĿռ��С = 0x%X\n",dwNewIIDEntrySizeUsed);
	
	dwNewThunkDataVA = pNewSecHeader->VirtualAddress + dwThunkDataOffsetByIID ;
	//���������OriginalFirstThunk��FirstThunk
	//���ݶ��壬OriginalFirstӦָ��IMAGE_IMPORT_BY_NAME�ṹ��ƫ��
	//����ImpName����ڵ�ǰThunkBuffer��ƫ�ƣ���ThunkBuffer��λ��ʵ������ԭ���̵ĵ����IID����
	pOriginalFirstThunk[0] = (DWORD)(MEM_OFFSET(pImpName,IIDThunkBuffer) + dwNewThunkDataVA);
	pFirstThunk[0] = pOriginalFirstThunk[0];

	//�������µ�IID���������RVA��Ӧ���������ڵ�ǰBuffer���ڲ�ƫ�ƣ��ټ���ԭ����������IID��ʼƫ��
	//Ҳ����Ҫ���Thunk�ṹ�ĵط�
	
	pNewImpEntry->OriginalFirstThunk = (DWORD)(MEM_OFFSET(pOriginalFirstThunk,IIDThunkBuffer) + dwNewThunkDataVA);
	pNewImpEntry->Name = (DWORD)(MEM_OFFSET(pDllName,IIDThunkBuffer) + dwNewThunkDataVA);
	pNewImpEntry->FirstThunk = (DWORD)(MEM_OFFSET(pFirstThunk,IIDThunkBuffer) + dwNewThunkDataVA);
	printf("[*] ��IID������.\n");

	//����PEͷ�еļ���ֵ
	//�µĵ�����С
	Image.m_pImpDataDir->Size = dwNewIIDSize;
	//�µĵ����IID����ʼƫ��
	Image.m_pImpDataDir->VirtualAddress = dwVAToStoreNewIID;
	
	//������½��ǿ�д�ģ���ThunkDataҲ���½��У����Բ����ڲ���д������
	//�����Ҫ�޸�ԭ�ڵ����ݣ���Ϊ��ʱ�����Ѿ�ӳ����ϣ��������޸�PEͷ�еĽ������Ѿ����ã���Ҫֱ���޸�ӳ����ҳ����
	//pImpSecHeader->Characteristics |= IMAGE_SCN_MEM_WRITE;
	

	//��հ������,ǿ�ȼ��������¼���IAT
	Image.m_pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
	Image.m_pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;
	printf("[*] PEͷ�������.\n");

	//д���ļ�
	printf("[*] ��ʼ�����ڴ��е�PE����.\n");

	//��ʼ�����ڴ��е��޸����ݵ��ļ���
	//��д���µ�PEͷ
	PBYTE TargetAddress = 0;
	DWORD dwSizeToChange = 0;
	DWORD dwVAInMemory = 0 ;
	TargetAddress = ImageBase;
	printf("[*] ׼��д��PEͷ: StartAddress = 0x%p Size = 0x%X\n",TargetAddress,Image.m_pOptHeader->SizeOfHeaders);
	bResult = VirtualProtectEx(pi.hProcess,TargetAddress,Image.m_pOptHeader->SizeOfHeaders,PAGE_READWRITE,&dwOldProtect);
	if (!bResult)
	{
		sprintf(szErrMsg,"�޷��޸�Ŀ������е�ַ 0x%p Size = 0x%X ������ڴ�����!",TargetAddress,Image.m_pOptHeader->SizeOfHeaders);
		Image.FormatErrorMsg(szErrMsg,GetLastError());
		Image.GetErrorMsg(szErrMsg,1024);
		printf("[-] %s\n",szErrMsg);
		return FALSE;
	}
	bResult = WriteProcessMemory(pi.hProcess,ImageBase,Image.m_HeaderData,Image.m_pOptHeader->SizeOfHeaders,&dwIoCnt);
	if (!bResult)
	{
		Image.FormatErrorMsg("��Ŀ�����д��PEͷʧ��!",GetLastError());
		Image.GetErrorMsg(szErrMsg,1024);
		printf("[-] %s\n",szErrMsg);
		return FALSE;
	}
	VirtualProtectEx(pi.hProcess,TargetAddress,Image.m_pOptHeader->SizeOfHeaders,dwOldProtect,&dwOldProtect);

	printf("[*] PEͷд�����.\n");
	
	//д���µ�IID�ṹ,��Ϊ������������ڴ���(PAGE_EXECUTE_READWRITE)�����Բ���Ҫ���޸�ҳ����
	dwVAInMemory = (DWORD)Image.m_pImpDataDir->VirtualAddress;
	TargetAddress = ImageBase + dwVAInMemory ;
	bResult = WriteProcessMemory(pi.hProcess,ImageBase + dwVAInMemory,pBuildNewImpDesp,dwNewIIDSize,&dwIoCnt);
	if (!bResult)
	{
		Image.FormatErrorMsg("[-] ��Ŀ�����д��IIDʧ��!",GetLastError());
		return FALSE;
	}

	//д����IID���ӽṹ��Ϣ,λ�����µ�������֮��
	dwVAInMemory = MEM_OFFSET(pOriginalFirstThunk,IIDThunkBuffer) + dwNewThunkDataVA;
	TargetAddress = ImageBase + dwVAInMemory ;
	printf("[*] ׼��д��IID��Ϣ: StartAddress = 0x%p Size = 0x%X\n",TargetAddress,Image.m_pOptHeader->SizeOfHeaders);
	bResult = WriteProcessMemory(pi.hProcess,ImageBase + dwVAInMemory,pOriginalFirstThunk,dwNewIIDEntrySizeUsed,&dwIoCnt);
	if (!bResult)
	{
		Image.FormatErrorMsg("[-] ��Ŀ�����д��IID Thunk����ʧ��!",GetLastError());
		return FALSE;
	}
	printf("[*] ��IID����ӽṹд�����. Add = 0x%X Size = 0x%x\n",dwVAInMemory,dwIoCnt);

	
	printf("[*] �µ��������д�����. Offset = 0x%X Size = 0x%x\n",dwVAInMemory,dwIoCnt);
	printf("[*] ������Ⱦ���.\n");
	
	if (!(dwCreationFlags & CREATE_SUSPENDED))
	{
		//����ִ�����߳�
		ResumeThread(pi.hThread);
	}
	
	if (lpProcessInformation != NULL)
	{
		memcpy(lpProcessInformation,&pi,sizeof(PROCESS_INFORMATION));
	}
	
	return TRUE;
}

ULONG_PTR FindImageBase(HANDLE hProc,LPSTR lpCommandLine)
{
	ULONG_PTR uResult = 0 ;
	TCHAR szBuf[1024]={0};
	SIZE_T dwSize = 0 ;
	PBYTE pAddress = NULL ;
	
	MEMORY_BASIC_INFORMATION mbi = {0};
	BOOL bFoundMemImage = FALSE ;
	char szImageFilePath[MAX_PATH]={0};
	char *pFileNameToCheck = strrchr(lpCommandLine,'\\');

	//��ȡҳ�Ĵ�С
	SYSTEM_INFO sysinfo;
	ZeroMemory(&sysinfo,sizeof(SYSTEM_INFO));
	GetSystemInfo(&sysinfo);
	
	//Found First MEM_IMAGE Page
	pAddress = (PBYTE)sysinfo.lpMinimumApplicationAddress;
	while (pAddress < (PBYTE)sysinfo.lpMaximumApplicationAddress)
	{
		ZeroMemory(&mbi,sizeof(MEMORY_BASIC_INFORMATION));
		dwSize = VirtualQueryEx(hProc,pAddress,&mbi,sizeof(MEMORY_BASIC_INFORMATION));
		if (dwSize == 0)
		{
			pAddress += sysinfo.dwPageSize ;
			continue;
		}

		switch(mbi.State)
		{
		case MEM_FREE:
		case MEM_RESERVE:
			pAddress = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
			break;
		case MEM_COMMIT:
			if (mbi.Type == MEM_IMAGE)
			{
				if (GetMappedFileName(hProc,pAddress,szImageFilePath,MAX_PATH) != 0)
				{
					//printf("Address = 0x%p ImageFileName = %s\n",pAddress,szImageFilePath);
					char *pCompare = strrchr(szImageFilePath,'\\');
					if (stricmp(pCompare,pFileNameToCheck) == 0)
					{
						bFoundMemImage = TRUE;
						uResult = (ULONG_PTR)pAddress;
						break;
					}
				}
			}
			pAddress = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
		    break;
		default:
		    break;
		}

		if (bFoundMemImage)
		{
			break;
		}
	}
	return uResult ;
}

DWORD ProcesstoPid(char *Processname) //����ָ�����̵�PID(Process ID)
{
	HANDLE hProcessSnap=NULL;
	DWORD ProcessId=0;
	PROCESSENTRY32 pe32={0};
	hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0); //�򿪽��̿���
	if(hProcessSnap==(HANDLE)-1)
	{
		printf("\nCreateToolhelp32Snapshot() Error: %d",GetLastError());
		return 0;
	}
	pe32.dwSize=sizeof(PROCESSENTRY32);
	if(Process32First(hProcessSnap,&pe32)) //��ʼö�ٽ���
	{
		do
		{
			if(!stricmp(Processname,pe32.szExeFile)) //�ж��Ƿ���ṩ�Ľ�������ȣ��ǣ����ؽ��̵�ID
			{
				ProcessId=pe32.th32ProcessID;
				break;
			}
		}
		while(Process32Next(hProcessSnap,&pe32)); //����ö�ٽ���
	}
	else
	{
		printf("\nProcess32First() Error: %d",GetLastError());
		return 0;
	}
	CloseHandle(hProcessSnap); //�ر�ϵͳ���̿��յľ��
	return ProcessId;
}