// NoSEH.cpp : Defines the entry point for the console application.
//
/*-----------------------------------------------------------------------
��8��  Windows�µ��쳣����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/
#include "stdafx.h"

#pragma comment(linker,"/Entry:main")
#pragma comment(linker,"/subsystem:windows")

__declspec(naked) void main(void)
{
	__asm
	{
		;mov dword ptr fs:[0],-1
		xor eax,eax
		mov [eax],5 //��0��ַд�����ݣ������ڴ�����쳣
		retn
	}
}

