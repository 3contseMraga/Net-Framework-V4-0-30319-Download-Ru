/*-----------------------------------------------------------------------
��12��  ע�뼼��
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// InfectImport.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include "Image.h"

#define INFECT_SIG ('PE')

BOOL AddSectionTest();
BOOL InfectImport(
	IN char *szImageFilePath,
	IN char *szDllPath,
	IN char *szDllExportFunName
	);

int main(int argc, char* argv[])
{
#ifdef _WIN64
	InfectImport("F:\\Program2016\\DllInjection\\HostProc64.exe","Msgdll64.dll","Msg");
#else
	InfectImport("F:\\Program2016\\DllInjection\\HostProc.exe","Msgdll.dll","Msg");
#endif
	return 0;
}

BOOL InfectImport(
	IN char *szImageFilePath,
	IN char *szDllName,
	IN char *szDllExportFunName)
{
	CImage Img;
	BOOL bResult = FALSE ;
	WORD i = 0 ;
	DWORD dwIoCnt = 0 ;
	char szErrMsg[1024]={0};
	PIMAGE_SECTION_HEADER pImpSecHeader,pNewSecHeader = NULL,pTargetSecHeader = NULL;
	DWORD dwOldIIDCnt = 0 ,dwNewIIDCnt = 0 ;
	DWORD dwOldIIDSize = 0, dwNewIIDSize = 0 ;
	DWORD dwVAToStoreNewIID = 0 ; //��IID����Ĵ洢λ��
	DWORD dwnewThunkDataSize = 0 ; //��IID���ThunkData�Ĵ洢λ��
	DWORD dwNewThunkDataVA = 0 ;//��IID���ThunkData�Ĵ洢λ��
	DWORD dwSizeNeed = 0 ;
	DWORD dwThunkDataOffsetByIID = 0 ;
	BOOL bUseNewSection = FALSE ; //�Ƿ�ʹ�����½�
	BOOL bPlaceThunkDataToOldIID = TRUE ; //����ThunkData��ŵ�λ���ǲ�����ԭ����IIDλ�ã�����Ų��£�������λ��
	
	printf("[*] Path = %s\n",szImageFilePath);
	//�Զ�д��ʽ��Ŀ���ļ�
	HANDLE hFile = CreateFile(szImageFilePath,
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	//����PE�ṹ
	PBYTE pNotepad = Img.LoadImage(hFile,FALSE,0,FALSE);
	printf("[*] pImageBase = 0x%p\n",pNotepad);

	if (pNotepad == NULL)
	{
		printf("[-] ����PE�ļ�ʧ��! %s\n",Img.GetErrorMsg(szErrMsg,1024));
		return FALSE;
	}
	
	//����Ƿ񱻸�Ⱦ��
	if (Img.m_pDosHeader->e_csum == INFECT_SIG)
	{
		printf("[-] �ļ��Ѿ�����Ⱦ��!\n");
		return FALSE;
	}
	printf("[*] ��ǰ�������Ϣ VA = 0x%p Size = 0x%X\n",Img.m_pImpDataDir->VirtualAddress,Img.m_pImpDataDir->Size);
	dwOldIIDSize = Img.m_pImpDataDir->Size ;
	dwOldIIDCnt = dwOldIIDSize / sizeof(IMAGE_IMPORT_DESCRIPTOR) ;
	dwNewIIDCnt = dwOldIIDCnt + 1;
	dwNewIIDSize = dwNewIIDCnt * sizeof(IMAGE_IMPORT_DESCRIPTOR) ;
	printf("[*] dwOldIIDCnt = %d  Size = 0x%X\n",dwOldIIDCnt,dwOldIIDSize);
	printf("[*] dwNewIIDCnt = %d  Size = 0x%X\n",dwNewIIDCnt,dwNewIIDSize);
	
	dwSizeNeed = dwNewIIDSize; //����Ĵ�С���µ����IID�ṹ�Ĵ�С

	pImpSecHeader = Img.LocateSectionByRVA(Img.m_pImpDataDir->VirtualAddress);
	printf("[*] ��������ڽ�  %s  RawOffset = 0x%X Size = 0x%X\n",
		pImpSecHeader->Name,
		pImpSecHeader->PointerToRawData,
		pImpSecHeader->SizeOfRawData);
	
	DWORD dwPaddingSize = Img.GetSectionPhysialPaddingSize(pImpSecHeader);
	printf("[*] �����ڿ�϶��С = 0x%X\n",dwPaddingSize);
	
	//�������ThunkData��Ҫ�Ĵ�С����������OriginalFirstThunk��FirstThunk��IMPORT_BY_NAME���Լ�DllName
	dwnewThunkDataSize = sizeof(ULONG_PTR)*4 + strlen(szDllName) + 1 + sizeof(WORD) + strlen(szDllExportFunName) + 1 ;
	dwnewThunkDataSize = ALIGN_SIZE_UP(dwnewThunkDataSize,sizeof(ULONG)); //����
	//�ж�ԭ�����λ���ܷ�д���µ�ThunkData
	if (dwnewThunkDataSize > dwOldIIDSize)
	{
		//д����,��ô��Ѱ�ҽ�϶ʱ��Ҫ����
		//��ULONG_PTR����֮�������ThunkData����Ȼ�����������Ҳ����
		dwThunkDataOffsetByIID = ALIGN_SIZE_UP(dwNewIIDSize,sizeof(ULONG_PTR)) ;
		dwSizeNeed = dwThunkDataOffsetByIID + dwnewThunkDataSize ;
		bPlaceThunkDataToOldIID = FALSE ;
	}
	printf("[*] �����µ������������Ҫ�Ĵ�С = 0x%X\n",dwSizeNeed);
	//dwPaddingSize = 0 ;//����,ǿ������½�
	if (dwPaddingSize >= dwSizeNeed)
	{
		printf("[*] �ڿ�϶���Է����µĵ������������½�!\n");
		dwVAToStoreNewIID = pImpSecHeader->VirtualAddress + Img.GetAlignedSize(pImpSecHeader->Misc.VirtualSize,sizeof(DWORD));
		pTargetSecHeader = pImpSecHeader;
	}
	else
	{
		printf("[-] �ڿ�϶���ܷ����µĵ������Ҫ����½�!\n");
		//��������Ŀռ��С���һ���½�
		pNewSecHeader = Img.AddNewSectionToFile(".Patch",dwSizeNeed);
		printf("[*] �½�������! VA = 0x%X  RawOffset = 0x%X  RawSize = 0x%X\n",
			pNewSecHeader->VirtualAddress,pNewSecHeader->PointerToRawData,pNewSecHeader->SizeOfRawData);
		dwVAToStoreNewIID = pNewSecHeader->VirtualAddress ;
		pTargetSecHeader = pNewSecHeader;
		bUseNewSection = TRUE;
	}
	
	//����ԭ�����
	PIMAGE_IMPORT_DESCRIPTOR pOldImpDesp = Img.m_pImportDesp;
	PIMAGE_IMPORT_DESCRIPTOR pBuildNewImpDesp = (PIMAGE_IMPORT_DESCRIPTOR)malloc(dwSizeNeed);
	ZeroMemory(pBuildNewImpDesp,dwSizeNeed);
	//����ԭ���ĵ�����ֵ��µ���
	memcpy(pBuildNewImpDesp,pOldImpDesp,dwOldIIDSize);
	printf("[*] ԭ�����IID�ṹ�������.\n");
	//ָ��һ������ӵ�IID��,�Ժ����
	PIMAGE_IMPORT_DESCRIPTOR pNewImpEntry = pBuildNewImpDesp + dwOldIIDCnt - 1;


	//��Ҫע����ǣ�ThunkData��32λ��64λ�µĳ����ǲ�һ���ģ��������ﶨ��Ϊ����Ӧ��ULONG_PTR
	PULONG_PTR pOriginalFirstThunk = NULL ;
	if (bPlaceThunkDataToOldIID)
	{
		//ʹ��ԭIID��λ�ô��Thunk����
		pOriginalFirstThunk = (PULONG_PTR)(Img.m_hModule + Img.m_pImpDataDir->VirtualAddress);
		dwNewThunkDataVA = Img.m_pImpDataDir->VirtualAddress ;
	}
	else
	{
		//ԭIID��λ�ô�Ų��£�ʹ����λ�ô��
		pOriginalFirstThunk = (PULONG_PTR)((PBYTE)pBuildNewImpDesp + dwThunkDataOffsetByIID);
		dwNewThunkDataVA = dwVAToStoreNewIID + dwThunkDataOffsetByIID ; //��IID���ݺ���
	}
	ZeroMemory(pOriginalFirstThunk,dwnewThunkDataSize);
	//�����������ݣ���һ���Ժ���䣬�ڶ�����0��Ϊ�������
	PULONG_PTR pFirstThunk = pOriginalFirstThunk + 2 ;
	//������������,��һ���Ժ���䣬�ڶ�����0��Ϊ������ǣ�֮����ΪDll����
	PCHAR  pDllName = (PCHAR)(pFirstThunk + 2);
	//����dll����
	strcpy(pDllName,szDllName);
	
	SIZE_T DllNameLen = strlen(szDllName);
	pDllName[DllNameLen] = 0;
	//��������Ϊһ��PIMPORT_BY_NAME�ṹ
	PIMAGE_IMPORT_BY_NAME pImpName = (PIMAGE_IMPORT_BY_NAME)(pDllName + DllNameLen + 1);
	//�����
	pImpName->Hint = 0;
	strcpy((char*)pImpName->Name,szDllExportFunName);
	printf("[*] �µ����IID�ӽṹ�������.\n");
	
	//�������λ��
	PCHAR pEnd = (PCHAR)pImpName + sizeof(pImpName->Hint) + strlen((char*)pImpName->Name) + 1;
	//������ռ�õĿռ��С
	DWORD dwNewIIDEntrySizeUsed = (DWORD)pEnd - (DWORD)pOriginalFirstThunk;
	printf("[*] ��IID��Առ�õĿռ��С = 0x%X\n",dwNewIIDEntrySizeUsed);

	//���������OriginalFirstThunk��FirstThunk
	//���ݶ��壬OriginalFirstӦָ��IMAGE_IMPORT_BY_NAME�ṹ��ƫ��
	pOriginalFirstThunk[0] = dwNewThunkDataVA + ((PBYTE)pImpName - (PBYTE)pOriginalFirstThunk);
	pFirstThunk[0] = pOriginalFirstThunk[0];

	//�������µ�IID���������RVA
	pNewImpEntry->OriginalFirstThunk = dwNewThunkDataVA;
	pNewImpEntry->Name = dwNewThunkDataVA + sizeof(ULONG_PTR)*4;//OriginalFirstThunk + FirstThunk�Ĵ�С
	pNewImpEntry->FirstThunk = dwNewThunkDataVA + sizeof(ULONG_PTR)*2;
	printf("[*] ��IID������.\n");

	//����PEͷ�еļ���ֵ
	//�µĵ�����С
	Img.m_pImpDataDir->Size = dwNewIIDSize;
	//�µĵ����IID����ʼƫ��
	Img.m_pImpDataDir->VirtualAddress = dwVAToStoreNewIID;
	if (!bUseNewSection)
	{
		pImpSecHeader->Misc.VirtualSize += dwSizeNeed;
	}
	
	//���ThunkData������ԭIID��λ�ã���Ҫ���ý�Ϊ��д��
	pImpSecHeader->Characteristics |= IMAGE_SCN_MEM_WRITE;
	//��հ������,ǿ�ȼ��������¼���IAT
	Img.m_pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
	Img.m_pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;
	//���ø�Ⱦ���
	Img.m_pDosHeader->e_csum = INFECT_SIG;
	printf("[*] PEͷ�������.\n");

	//д���ļ�
	printf("[*] ��ʼ�����ļ�.\n");

	//��ʼ�����ڴ��е��޸����ݵ��ļ���
	//��д���µ�PEͷ
	DWORD dwFileOffset = 0;
	ULONG_PTR dwVAInMemory = 0 ;
	SetFilePointer(hFile,0,NULL,FILE_BEGIN);
	bResult = WriteFile(hFile,Img.m_hModule,Img.m_pOptHeader->SizeOfHeaders,&dwIoCnt,NULL);
	if (!bResult)
	{
		Img.FormatErrorMsg("[-] д���ļ�ʧ��!",GetLastError());
		return FALSE;
	}

	printf("[*] PEͷд�����. Offset = 0x%X Size = 0x%x\n",dwFileOffset,dwIoCnt);

	//д����IID���ӽṹ��Ϣ,λ����ԭ�����Ŀ�ʼ��
	dwVAInMemory = dwNewThunkDataVA ;
	dwFileOffset = Img.Rav2Raw(dwVAInMemory);
	SetFilePointer(hFile,dwFileOffset,NULL,FILE_BEGIN);
	bResult = WriteFile(hFile,pOriginalFirstThunk,dwNewIIDEntrySizeUsed,&dwIoCnt,NULL);
	if (!bResult)
	{
		Img.FormatErrorMsg("[-] д���ļ�ʧ��!",GetLastError());
		return FALSE;
	}
	printf("[*] ��IID����ӽṹд�����. Offset = 0x%X Size = 0x%x\n",dwFileOffset,dwIoCnt);

	//д���µ�IID�ṹ
	dwVAInMemory = (ULONG_PTR)Img.m_pImpDataDir->VirtualAddress;
	dwFileOffset = Img.Rav2Raw(dwVAInMemory);
	SetFilePointer(hFile,dwFileOffset,NULL,FILE_BEGIN);
	bResult = WriteFile(hFile,pBuildNewImpDesp,dwNewIIDSize,&dwIoCnt,NULL);
	if (!bResult)
	{
		Img.FormatErrorMsg("[-] д���ļ�ʧ��!",GetLastError());
		return FALSE;
	}
	printf("[*] �µ��������д�����. Offset = 0x%X Size = 0x%x\n",dwFileOffset,dwIoCnt);
	printf("[*] ������Ⱦ���.\n");

	return TRUE;
}