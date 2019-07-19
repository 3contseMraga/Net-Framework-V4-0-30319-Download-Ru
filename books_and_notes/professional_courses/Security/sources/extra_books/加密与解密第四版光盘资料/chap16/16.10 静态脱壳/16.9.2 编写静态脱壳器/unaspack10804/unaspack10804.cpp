/*******************************************************
����������ܡ�����������ʵ��
15.8.2 ��д��̬�ѿ���                                    
  unaspack10804.cpp : Defines the entry point for the console application.
  code by DiKen
(c)  ��ѩ�����ȫ��վ www.pediy.com 2000-2008
********************************************************/


#include <windows.h>
#include <stdio.h>


#define halt0(msg, result)							\
	printf(msg);									\
	return result;									\

#define halt1(msg, result)							\
	CloseHandle(hFile);								\
	printf(msg);									\
	return result;									\

#define halt2(msg, result)							\
	CloseHandle(hMap);								\
	CloseHandle(hFile);								\
	printf(msg);									\
	return result;									\

#define halt3(msg, result)							\
	UnmapViewOfFile(pMap);							\
	CloseHandle(hMap);								\
	CloseHandle(hFile);								\
	printf(msg);									\
	return result;									\


#define halt4(msg, result)							\
	VirtualFree(Image, 0, MEM_RELEASE);				\
	UnmapViewOfFile(pMap);							\
	CloseHandle(hMap);								\
	CloseHandle(hFile);								\
	printf(msg);									\
	return result;									\

#define halt5(msg, result)							\
	VirtualFree(Image, 0, MEM_RELEASE);				\
	VirtualFree(pLoaderCore, 0, MEM_RELEASE);		\
	UnmapViewOfFile(pMap);							\
	CloseHandle(hMap);								\
	CloseHandle(hFile);								\
	printf(msg);									\
	return result;									\
  	
#define halt6(msg, result)							\
	CloseHandle(hFileOutput);						\
	VirtualFree(Image, 0, MEM_RELEASE);				\
	VirtualFree(pLoaderCore, 0, MEM_RELEASE);		\
	UnmapViewOfFile(pMap);							\
	CloseHandle(hMap);								\
	CloseHandle(hFile);								\
	printf(msg);									\
	return result;									\
  			
		

DWORD AlignUp(DWORD n, DWORD A)
{
	return ((n / A) +  ((n % A) ? 1 : 0)) * A;
}

int IndexOfRva(void *pMap, DWORD Rva)
{
	PIMAGE_DOS_HEADER pd = (PIMAGE_DOS_HEADER)pMap;
	PIMAGE_NT_HEADERS ph = (PIMAGE_NT_HEADERS)((BYTE *)pMap + pd->e_lfanew);
	PIMAGE_SECTION_HEADER ps = (PIMAGE_SECTION_HEADER)((BYTE *)ph + sizeof(IMAGE_NT_HEADERS));

	if (Rva > ph->OptionalHeader.SizeOfImage)
	{
		return -2;
	};


	if (ps->VirtualAddress > Rva)
	{
		return -1;
	}
	else
	{
		for (int i = 0; i < ph->FileHeader.NumberOfSections; i++)
		{
			if ((ps->VirtualAddress <= Rva) && ((ps->VirtualAddress + AlignUp(ps->Misc.VirtualSize ,ph->OptionalHeader.SectionAlignment)) > Rva))
			{
				return i;
			};
			ps++;
		}
	}
	return -2;
}

void *RvaToPointer(void *pMap, DWORD Rva)
{
	int i = IndexOfRva(pMap, Rva);
	if (i < 0)
	{
		if (i = -2) return NULL;
		return ((BYTE *)pMap + Rva);
	}
	else
	{
		PIMAGE_DOS_HEADER pd = (PIMAGE_DOS_HEADER)pMap;
		PIMAGE_NT_HEADERS ph = (PIMAGE_NT_HEADERS)((BYTE *)pMap + pd->e_lfanew);
		PIMAGE_SECTION_HEADER ps = (PIMAGE_SECTION_HEADER)((BYTE *)ph + sizeof(IMAGE_NT_HEADERS));
		ps += i;
		return ((BYTE *)pMap + ps->PointerToRawData + Rva - ps->VirtualAddress);				
	};
}


#define getbit()\
	(b = b * 2, b & 0xFF ? ((b >> 8) & 1) : (((b = ((*pSrc++ * 2) + ((b >> 8) & 1))) >> 8) & 1))


#define CopyRepeat(dst, off, len)\
	for(;len > 0; len--)\
{\
	*dst = *(dst - off);\
	dst++;\
}

int aPLibDePack(void *Src, void *Dst)
{
	BYTE *pSrc = (BYTE *)Src;
	BYTE *pDst = (BYTE *)Dst; 
	DWORD off, last_off = 0;
	DWORD len;
	DWORD b = 0x80;

	*pDst++ = *pSrc++;
	for(;;)
	{
		///////////////////////
		//���bitΪ0�򿽱�1���ַ�; ����
		if (!getbit())
		{
			*pDst++ = *pSrc++;
			continue;
		};

		//
		if (getbit())
		{
			off = 0;
			if (getbit())
			{
				//�ظ�
				off = off * 2 + getbit();
				off = off * 2 + getbit();
				off = off * 2 + getbit();
				off = off * 2 + getbit();
				if (off != 0)
				{
					*pDst = *(pDst - off);
					pDst++;
				}
				else
				{
					*pDst++ = 0;
				};
				continue;
			}//goto case 5
		}
		else
		{
			//case1
			off = 1;

			do 
			{
				off = off * 2 + getbit();
			}while(getbit());

			off = off - 2;
			if (off == 0)
			{
				len = 1;
				do
				{
					len = len * 2 + getbit();
				}while (getbit());
				CopyRepeat(pDst, last_off, len);      
			}
			else
			{
				off--;
				off = off * 256 + *pSrc++;
				last_off = off;

				len = 1;
				do 
				{
					len = len * 2 + getbit();
				}while (getbit());

				//�ظ�����
				//[0-7F]                + 2
				//(7F-0x500)            + 0
				//[0x500, 0x7D00)       + 1
				//[7D00-]               + 2
				off = off - 0x80;				
				if (off >= 0x500 - 0x80) len++;
				if (off >= 0x7D00 - 0x80) len++;      
				off = off + 0x80;
				CopyRepeat(pDst, off, len);				
			};
			continue;
		};  


		///////////////////
		//case5
		off = *pSrc++;
		len = 2 + (off & 1);
		off = off >> 1;  
		if (off == 0) break;
		last_off = off;

		CopyRepeat(pDst, off, len);
	};
	return (pDst - (BYTE *)Dst);
}



int main(int argc, char* argv[])
{

	char szFileName[MAX_PATH] = "D:\\calc.exe.bak.exe";
	//�����ļ�����
	HANDLE hFile;
	DWORD dwFileSize;
	HANDLE hMap;
	void *pMap;

	




	//////////////////////////////////////////////////////////////////////////
	//�������ļ��� ��Map���ڴ�
	hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		halt0("error open file for read!\n", -1);
	};

	dwFileSize = GetFileSize(hFile, NULL);


	//OpenMap�����ļ�
	hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY | SEC_COMMIT, 0, 0, NULL);
	if (hMap == NULL)
	{
		halt1("error open file map  for read!\n", -2);
	};
	//MapFile
	pMap = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if (pMap == NULL)
	{
		halt2("error map file for read!\n", -3);
	};

	//////////////////////////////////////////////////////////////////////////
	//��ʼ����ļ���

	//DOSͷ���
	if (dwFileSize < sizeof(IMAGE_DOS_HEADER))
	{
		halt3("file size small\n", -4);
	};

	PIMAGE_DOS_HEADER pd = (PIMAGE_DOS_HEADER)pMap;
	if (pd->e_magic != IMAGE_DOS_SIGNATURE)
	{
		halt3("file not pe -> dos magic\n", -5);
	};

	//PEͷ���
	DWORD dwProbeSize = pd->e_lfanew + sizeof(IMAGE_NT_HEADERS);
    if (dwFileSize < dwProbeSize)
	{
		halt3("file size small 2\n", -6);
	};
	PIMAGE_NT_HEADERS ph = (PIMAGE_NT_HEADERS)((BYTE *)pMap + pd->e_lfanew);
	if (ph->Signature != IMAGE_NT_SIGNATURE)
	{		
		halt3("file not pe -> pe magic\n", -7);
	};

	//����ֻ֧��PE32,��֧��PE32+��
	if (ph->OptionalHeader.Magic == 0x20B)
	{
		halt3("file not pe32\n", -8);
	};

	//PE Sectionӳ��
	dwProbeSize += ph->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER); 
	if (dwFileSize < dwProbeSize)
	{		
		halt3("file size small 3\n", -9);
	};

	PIMAGE_SECTION_HEADER psbase = (PIMAGE_SECTION_HEADER)((BYTE *)ph + sizeof(IMAGE_NT_HEADERS));//�õ������ĵ�ַ
	
	//�ļ���С���
	//�����⣬ѭ�����ûɶ����
	PIMAGE_SECTION_HEADER ps = psbase;
	dwProbeSize = 0;	
	for (int i = 0; i < ph->FileHeader.NumberOfSections; i++)
	{
		if (ps->PointerToRawData >= dwProbeSize) 
			dwProbeSize = ps->PointerToRawData + ps->SizeOfRawData;
	};	
	if (dwFileSize < dwProbeSize)
	{
		//�����ݶ�������	
		halt3("file size small 4\n", -10);
	};
	
	//Overlay����
	



	//////////////////////////////////////////////////////////////////////////
	//�Ǽ��
	//�����������汾�ĿǾ��˳���

	//��Ϊ����ֻ����ض��ǣ����Ծ�ֱ���þ���ƫ�����Ƚϣ�������Ǿ���Ϊ�����������
	//��ȻֻҪ��΢�ӵ�PE-PATCH�Ļ�Ҳ���ϲ�������
	DWORD dwEntrypoint = ph->OptionalHeader.AddressOfEntryPoint;
	BYTE *pEntrypoint = (BYTE *)RvaToPointer(pMap, dwEntrypoint);
	
	//�������ָ���ж��Ƿ��Ǹÿ�
	if ((*pEntrypoint != 0x60)
		||(*((DWORD*)(pEntrypoint + 0x49)) != 0x442914BB)//mov ebx, 442914
		||(*((DWORD*)(pEntrypoint + 0xBA)) != 0x54A)	//push 54Ah
		||(*((DWORD*)(pEntrypoint + 0x29)) != dwEntrypoint)	//dd Entrypoint??
	)
	{
		halt3("not aspack 1.08.04", -11);
	};

	//////////////////////////////////////////////////////////////////////////
	//���ڿ��Թ�����
	//ӳ�������ļ����ڴ�
	char *Image;
	Image = (char *)VirtualAlloc(NULL, AlignUp(ph->OptionalHeader.SizeOfImage, ph->OptionalHeader.SectionAlignment), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (Image == NULL)
	{
		halt3("cannot map file", -12);
	};	
	ps = psbase;
	memcpy(Image, pMap, ph->OptionalHeader.SizeOfHeaders);
	for (i = 0; i < ph->FileHeader.NumberOfSections; i++)
	{
		memcpy(Image + ps->VirtualAddress,
			    (char *)pMap + ps->PointerToRawData, AlignUp(ps->SizeOfRawData, ph->OptionalHeader.FileAlignment));
		ps++;
	};


	//1.���ܹؼ�����, �ⲿ��
	BYTE *pLoaderCore = (BYTE *) VirtualAlloc(NULL, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pLoaderCore == NULL)
	{
		halt4("alloc virtual memory fail", -13);
	};

	//010270FD - 01027000
	DWORD LoaderSize = aPLibDePack(pEntrypoint + 0xFD, pLoaderCore);
	if (LoaderSize == 0)
	{
		halt5("loader decode failed.", -14);
	};

	//+08
	DWORD offImageBase = 0x0C;
	//+10
	//+14
	DWORD offReloc	=	0x18;//+18
	DWORD offImport	=	0x1C;
	DWORD offEntrypoint	=	0x20;
	//
	DWORD offSectionInfo	=	0x2C;





	
	//Ϊ�˷�ֹ������Щ��û��ѹ��, ���, ֱ�Ӳ���Map��ʽ


	//��ȡ���ݿ�ʼ���н���
	struct stSectionInfo 
	{
		DWORD Rva;
		DWORD VSize;	
	};


	stSectionInfo *pSectionInfo = (stSectionInfo *)(pLoaderCore + offSectionInfo);
	bool isfirst = true;
	while (pSectionInfo->Rva != 0)
	{
		//void *WorkMem = VirtualAlloc(NULL, pSectionInfo->VSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		//ֱ�ӽ�ѹ��Image
		DWORD dwRealSize = aPLibDePack(RvaToPointer(pMap, pSectionInfo->Rva), Image + pSectionInfo->Rva);

		//����ǵ�һ����Ҫ�޸�E8E9
		if (isfirst)
		{
			isfirst = false;
			BYTE *p = (BYTE *)Image + pSectionInfo->Rva;
			int nSize = dwRealSize - 6;
			DWORD off = 0;
			while (nSize > 0)
			{
				if ((*p == 0xE8)||(*p == 0xE9))
				{
					*(DWORD *)(p + 1) -= off;
					off += 4;
					p += 4;
					nSize -= 4;					
				};
				off++;
				p++;
				nSize--;
			}
		};
		pSectionInfo++;
	};


	//////////////////////////////////////////////////////////////////////////
	//����Image�����PEͷ
    pd = (PIMAGE_DOS_HEADER)Image;
	ph = (PIMAGE_NT_HEADERS)((BYTE *)Image + pd->e_lfanew);
	psbase = (PIMAGE_SECTION_HEADER)((BYTE *)ph + sizeof(IMAGE_NT_HEADERS));

	//�ļ����뷽ʽ
	ph->OptionalHeader.SizeOfImage = AlignUp(ph->OptionalHeader.SizeOfImage, ph->OptionalHeader.SectionAlignment);
	ph->OptionalHeader.FileAlignment = ph->OptionalHeader.SectionAlignment;
	ph->OptionalHeader.SizeOfHeaders = psbase->VirtualAddress;

	//�ڶ���
	ps = psbase;
	for (i = 0; i < ph->FileHeader.NumberOfSections; i++)
	{		
		ps->SizeOfRawData = ps->Misc.VirtualSize = AlignUp(ps->Misc.VirtualSize, ph->OptionalHeader.SectionAlignment);		
		ps->PointerToRawData = ps->VirtualAddress;
		ps++;
	}

	//////////////////////////////////////////////////////////////////////////
	//�ض�λ����
	//���ڿ�û���ƻ�Reloc, ���, ����ֱ�ӽ�Reloc��Rvaд��PEͷ��OK��
	DWORD dwRvaReloc = *(DWORD *)(pLoaderCore + offReloc);
	if (dwRvaReloc != 0)
	{
		//����Reloc Size
		DWORD RelocSize = 0;
		BYTE *pReloc = (BYTE *)Image + dwRvaReloc;

		while (*(DWORD*)pReloc != 0)
		{
			RelocSize += *(DWORD*)(pReloc + 4);
			pReloc += *(DWORD*)(pReloc + 4);
		};
		
		ph->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = RelocSize;
		ph->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = dwRvaReloc;
	};


	//////////////////////////////////////////////////////////////////////////
	//IAT����
	//���ڿ�û���ƻ�IAT, ���Ǽ򵥵Ľ�IAT�ĵ�ַд��PEͷ
	DWORD dwImportRva = *(DWORD *)(pLoaderCore + offImport);
	if (dwImportRva != 0)
	{
		DWORD ImportSize = sizeof(IMAGE_IMPORT_DESCRIPTOR);
		PIMAGE_IMPORT_DESCRIPTOR pImport = (PIMAGE_IMPORT_DESCRIPTOR)(Image + dwImportRva);
		while (pImport->Name != 0)
		{
			ImportSize += sizeof(IMAGE_IMPORT_DESCRIPTOR);
			pImport++;
		};

		ph->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = ImportSize;
		ph->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = dwImportRva;
	}



	//////////////////////////////////////////////////////////////////////////
	//OriginalEntrypoint����
	//���ڿ�û�г�ȡ���, �������ֻ�Ǽ򵥵Ľ�OEPд��PEͷ
	DWORD dwEntrypointRva = *(DWORD *)(pLoaderCore + offEntrypoint);
	ph->OptionalHeader.AddressOfEntryPoint = dwEntrypointRva;

	

	//////////////////////////////////////////////////////////////////////////
	//
	//�����ļ�
	//
	//////////////////////////////////////////////////////////////////////////
	strcat(szFileName,"_.EXE");

	HANDLE hFileOutput = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (hFileOutput == INVALID_HANDLE_VALUE)
	{
		halt5("create output file failed.", -15);
	};

	DWORD dwWrittenBytes;
	if (!WriteFile(hFileOutput, Image, ph->OptionalHeader.SizeOfImage, &dwWrittenBytes, NULL))
	{
		halt6("write output file error 1\n", -16);
	};




	//////////////////////////////////////////////////////////////////////////
	//д��������



	//////////////////////////////////////////////////////////////////////////
	//�ѿǳɹ�,	�ر��ļ�

	halt6("unpack success\n", 0);

    
	//////////////////////////////////////////////////////////////////////////
	//
	//���м���Խ����Ż�, ����FileAlign, ��Դ�ع���
	//��Ȼ����޸��ĺù�Ȼ�Ǻã������ܻ��Ż�֮������޷�������
	//
	//Ϊʲôû���ع�, �򵥵�˵, PE�ṹ��ʵ�ܸ���, �ع���ʱ����ܻ�������.
	//
	//////////////////////////////////////////////////////////////////////////
}


