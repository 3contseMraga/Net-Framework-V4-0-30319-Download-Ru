/*-----------------------------------------------------------------------
��17��  �����������
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/


#include <windows.h>

BOOL IsFileModified();
DWORD CRC32(BYTE*,DWORD);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{

	if(IsFileModified() )
		MessageBox(NULL,TEXT ("CRC32 Check OK!"),TEXT ("OK"),MB_ICONEXCLAMATION);
	else
    	MessageBox(NULL,"File corrupted! !","CRC error",MB_ICONEXCLAMATION);
	return 0;
}

////////////////////////////////////////////////////////////////
// ���ļ��ж�CRC32ֵ�Ƿ���ȷ
//

BOOL IsFileModified()
{
	PIMAGE_DOS_HEADER	    pDosHeader=NULL;
    PIMAGE_NT_HEADERS       pNtHeader=NULL;
    PIMAGE_SECTION_HEADER   pSecHeader=NULL;

	DWORD fileSize,OriginalCRC32,NumberOfBytesRW;
 	TCHAR  *pBuffer ,szFileName[MAX_PATH]; 


	//����ļ���
        GetModuleFileName(NULL,szFileName,MAX_PATH);
	//���ļ�
	HANDLE hFile = CreateFile(
		szFileName,
		GENERIC_READ,
		FILE_SHARE_READ, 
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	 if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;


	//����ļ����� :
	fileSize = GetFileSize(hFile,NULL);
	if (fileSize == 0xFFFFFFFF) return FALSE;

	pBuffer = new TCHAR [fileSize];     // �����ڴ�,Ҳ����VirtualAlloc�Ⱥ��������ڴ�
	ReadFile(hFile,pBuffer, fileSize, &NumberOfBytesRW, NULL);//��ȡ�ļ�����
	CloseHandle(hFile);  //�ر��ļ�
		

	pDosHeader=(PIMAGE_DOS_HEADER)pBuffer;
	pNtHeader=(PIMAGE_NT_HEADERS32)((DWORD)pDosHeader+pDosHeader->e_lfanew);
	 //��λ��PE�ļ�ͷ�����ִ���PE\0\0������ǰ4���ֽڴ��������������������CRC-32ֵ��
	OriginalCRC32 =*((DWORD *)((DWORD)pNtHeader-4));	 	 

	fileSize=fileSize-DWORD(pDosHeader->e_lfanew);//��PE�ļ�ͷǰ�ǲ�������ȥ��

	if (CRC32((BYTE*)(pBuffer+pDosHeader->e_lfanew),fileSize) == OriginalCRC32 )
		return TRUE;
	else
		return FALSE;

}
////////////////////////////////////////////////////////////////
// �����ַ�����CRC32ֵ
// ������������CRC32ֵ�ַ������׵�ַ�ʹ�С
// ����ֵ: ����CRC32ֵ

DWORD CRC32(BYTE* ptr,DWORD Size)
{

	DWORD crcTable[256],crcTmp1;
	
	//��̬����CRC-32��
	for (int i=0; i<256; i++)
	 {
		crcTmp1 = i;
		for (int j=8; j>0; j--)
		 {
			if (crcTmp1&1) crcTmp1 = (crcTmp1 >> 1) ^ 0xEDB88320L;
			 else crcTmp1 >>= 1;
		}

		 crcTable[i] = crcTmp1;
	 }
	//����CRC32ֵ
	DWORD crcTmp2= 0xFFFFFFFF;
	while(Size--)
	{
		crcTmp2 = ((crcTmp2>>8) & 0x00FFFFFF) ^ crcTable[ (crcTmp2^(*ptr)) & 0xFF ];
		ptr++;
	}
		
	return (crcTmp2^0xFFFFFFFF);
}
