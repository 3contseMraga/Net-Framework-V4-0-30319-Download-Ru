/*-----------------------------------------------------------------------
��8��  Windows�µ��쳣����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/
// Showx86SEH.cpp : Defines the entry point for the console application.
// Author:achillis
// ���뻷����VC6/VS2010���������汾VS���ɣ�
// ������������ʾVC���������쳣�ķ�װ���������˾��������ĸ������
// ��������е��Ա����򣬻�ʹ��IDA Pro����������ı����Ĵ���

#include "stdafx.h"
#include <windows.h>

DWORD ShowFinalNormal();
DWORD ShowFinalDirectRetn();
DWORD ShowFinalWithLeave();
DWORD ShowFinalNotCatchExp();
VOID OneException();
VOID OneUnHandleException();
VOID TwoException();
VOID ThreeException();
VOID ShowCPPException();
VOID CatchSubFunException();


int main(int argc, char* argv[])
{
	
	OneException();
	printf("================One Exception End====================\n");
	TwoException();
	printf("================Two Exception End====================\n");
	ThreeException();
	printf("===============Three Exception End===================\n");
	CatchSubFunException();
	printf("================Catch Subfun Exception End===========\n");
	ShowFinalNormal();
	printf("==================FinalNormal End====================\n");
	ShowFinalDirectRetn();
	printf("=================FinalDirectRetn End=================\n");
	ShowFinalWithLeave();
	printf("=================FinalWithLeave End==================\n");
	ShowFinalNotCatchExp();
	printf("=================FinalNotCatchExp End================\n");
	return 0;
}


//����try/except���,�����쳣
VOID OneException()
{
	int *pValue = NULL ;
	__try
	{
		printf("In Try.\n");
		*pValue = 0x55555555;
	}
	__except(printf("In Filter\n"),EXCEPTION_EXECUTE_HANDLER)
	{
		printf("In handler.\n");
	}
}

//����try/except���,�������쳣
VOID OneUnHandleException()
{
	int *pValue = NULL ;
	__try
	{
		printf("[SubFun] In Try.\n");
		*pValue = 0x55555555;
	}
	__except(printf("[SubFun] In Filter\n"),EXCEPTION_CONTINUE_SEARCH)
	{
		printf("[SubFun] In handler.\n");
	}
}

//�����Ӻ������쳣
VOID CatchSubFunException()
{
	__try
	{
		printf("In Try.\n");
		OneUnHandleException();
	}
	__except(printf("In Filter\n"),EXCEPTION_EXECUTE_HANDLER)
	{
		printf("In handler.\n");
	}
}

//����Ƕ���쳣
VOID TwoException()
{
	int *pValue = NULL ;
	__try
	{
		printf("In Try0.\n");
		__try
		{
			printf("In Try1.\n");
			*pValue = 0x55555555;
		}
		__except(printf("In Filter1\n"),EXCEPTION_CONTINUE_SEARCH)
		{
			printf("In Handler1.\n");
		}
	}
	__except(printf("In Filter0\n"),EXCEPTION_EXECUTE_HANDLER)
	{
		printf("In Handler0.\n");
	}
	
	printf("After All Trys.\n");
}

//���������
VOID ThreeException()
{
	int *pValue = NULL ;
	__try
	{
		printf("In Try0.\n");
		__try
		{
			printf("In Try1.\n");
			__try
			{
				printf("In Try2.\n");
				*pValue = 0x55555555;
			}
			__except(printf("In Filter2\n"),EXCEPTION_CONTINUE_SEARCH)
			{
				printf("In Handler2.\n");
			}
		}
		__except(printf("In Filter1\n"),EXCEPTION_CONTINUE_SEARCH)
		{
			printf("In Handler1.\n");
		}
	}
	__except(printf("In Filter0\n"),EXCEPTION_EXECUTE_HANDLER)
	{
		printf("In Handler0.\n");
	}
	
	printf("After All Trys.\n");
}

//��ͨ��try/finally
DWORD ShowFinalNormal()
{
	DWORD dwValue = 0x11111111 ;
	__try
	{
		printf("In Try.\n");
		dwValue = GetTickCount();
	}
	__finally
	{
		printf("In Finally.\n");
	}

	return dwValue;
}

//��try��ֱ��return
DWORD ShowFinalDirectRetn()
{
	DWORD dwValue = 0x11111111 ;
	__try
	{
		printf("In Try.\n");
		dwValue = GetTickCount();
		if (dwValue % 2 == 0)
		{
			return dwValue;
		}
	}
	__finally
	{
		printf("In Finally.\n");
	}
	
	return dwValue;
}

//ʹ��leave�뿪try��
DWORD ShowFinalWithLeave()
{
	DWORD dwValue = 0x11111111 ;
	__try
	{
		printf("In Try.\n");
		dwValue = GetTickCount();
		if (dwValue % 2 == 0)
		{
			printf("Begin to Leave try.\n");
			__leave;
		}
	}
	__finally
	{
		printf("In Finally.\n");
	}
	
	return dwValue;
}

//��try�������쳣,���������⽫����չ��ʱfinally�鱻ִ��
DWORD ShowFinalNotCatchExp()
{
	int *pValue = NULL ;
	__try
	{
		printf("In Try.\n");
		*pValue = 5 ;
	}
	__finally
	{
		printf("In Finally.\n");
	}
	
	return 0;
}
