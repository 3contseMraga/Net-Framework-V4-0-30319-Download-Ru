/*-----------------------------------------------------------------------
��8��  Windows�µ��쳣����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/
// VEHExample.cpp : Defines the entry point for the application.
// Author: achillis
// ��������ƽ̨: WinXP����
// ������Ϊ��ʾ�������쳣����

#include "stdafx.h"

#define _WIN32_WINNT 0x502
#include <windows.h>

char szTit[]="design : achillis XP+";
DWORD validADDR;

LONG WINAPI vh0(PEXCEPTION_POINTERS ExceptionInfo)
{
    PCONTEXT pContext=ExceptionInfo->ContextRecord;
    pContext->Eax=(DWORD)&validADDR;
    return  EXCEPTION_CONTINUE_EXECUTION;
}


int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR CmdLine,int nCmdShow)
{
    PVOID handle = AddVectoredExceptionHandler(TRUE,vh0);
	
	__asm
	{
		xor eax,eax
		mov [eax],5 //��0��ַд�����ݣ������ڴ�����쳣
	}
    
    MessageBox(0,"We SUC recovering from Write Acess!",szTit,MB_ICONINFORMATION);
    RemoveVectoredExceptionHandler(handle);
    return 0;
}