/*-----------------------------------------------------------------------
��8��  Windows�µ��쳣����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// ClearDrC.cpp : Defines the entry point for the console application.
// Author:achillis
// ������Ϊ��ʾ���쳣��������н��з����Բ��������dr�Ĵ�����ʹӲ���ϵ�ʧЧ

#include "stdafx.h"
#include <windows.h>

int main(int argc, char* argv[])
{
	PCONTEXT pContext = NULL ;
	__try
	{
		*(ULONG*)NULL = 5 ;
	}
	__except(pContext = (GetExceptionInformation())->ContextRecord,EXCEPTION_EXECUTE_HANDLER)
	{
		printf("Now Clear Dr Registers.\n");
		pContext->Dr0 = 0 ;
		pContext->Dr1 = 0 ;
		pContext->Dr2 = 0 ;
		pContext->Dr3 = 0 ;
		pContext->Dr7 = 341 ;
	}
	return 0;
}

