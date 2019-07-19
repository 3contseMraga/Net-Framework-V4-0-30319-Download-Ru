/*-----------------------------------------------------------------------
��8��  Windows�µ��쳣����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// VEHTest.cpp : �������̨Ӧ�ó������ڵ㡣
// Author: achillis
// ���뻷������Ҫ��Vista�Ժ��SDK�����±���
// ��������ƽ̨: Vista����
// ������Ϊ��ʾ�������쳣����VCH


#include "stdafx.h"
#include <windows.h>

LONG CALLBACK VectoredExceptionHandler(
	PEXCEPTION_POINTERS ExceptionInfo
	);

LONG CALLBACK VectoredContinueHandler(
	PEXCEPTION_POINTERS ExceptionInfo
	);

DWORD g_dwValue = 0 ;
int _tmain(int argc, _TCHAR* argv[])
{
	PVOID pVehHandle = AddVectoredExceptionHandler(1,VectoredExceptionHandler);
	PVOID pVchHandle = AddVectoredContinueHandler(1,VectoredContinueHandler);

	__try{
		__asm
		{
			;mov dword ptr fs:[0],-1  //���������һ�䣬�ƻ�SEH�ṹ����������SEHֱ��ִ��VCH
			xor eax,eax
			mov [eax],5	//�����ڴ�����쳣
		}
	}
	__except(printf("[SEH] In SEH filter.\n"),EXCEPTION_CONTINUE_EXECUTION) //�������쳣������Continue,��VCHִ�еĻ��ᣬ���߿��Գ����޸��������ֵ�Թ۲첻ͬ��Ч��
	{
		printf("[SEH] In SEH Handler.\n");
	}
	printf("After SEH.\n");
	return 0;
}

LONG CALLBACK VectoredExceptionHandler(
	PEXCEPTION_POINTERS ExceptionInfo
	)
{
	printf("[VEH] In VectoredExceptionHandler.\n");
	return EXCEPTION_CONTINUE_SEARCH; //VEH�в������쳣��ʹ�쳣����SEH��
}

LONG CALLBACK VectoredContinueHandler(
	PEXCEPTION_POINTERS ExceptionInfo
	)
{
	printf("[VCH] In VectoredContinueHandler.\n");
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
	{
		ExceptionInfo->ContextRecord->Eax = (DWORD)&g_dwValue;
		printf("[VCH] Exception Fixed.\n");
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

