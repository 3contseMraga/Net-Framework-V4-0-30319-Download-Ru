/*-----------------------------------------------------------------------
��14��  ©����������
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// getKernel32.cpp : Defines the entry point for the console application.
//ʵ�黷����
//windows xp sp3 + VC6.0 
//����ѡ�Debug��Ĭ�����ã�


#include "stdafx.h"
#include <stdio.h>
#include <windows.h>

int main(int argc, char* argv[])
{
	DWORD hKernel32 = 0;
	__asm
	{
		//assume	 	fs:nothing
		mov   		eax, fs:[30h]
		test  		eax, eax
		js  		os_9x
os_nt:  
		mov  		eax, dword ptr[eax+0ch]
		mov  		esi, dword ptr[eax+1ch]
		lodsd  
		mov   		eax, dword ptr[eax+8]
		jmp  		k_finished
os_9x:
		mov   		eax, dword ptr[eax+34h]
		mov  		eax, dword ptr[eax+7ch]
		mov  		eax, dword ptr[eax+3ch]
k_finished:
		mov  		hKernel32, eax  ;��ȡkernel32��ַ 

	}
	printf("hKernel32 = %x\n",hKernel32);
	return 0;
}

