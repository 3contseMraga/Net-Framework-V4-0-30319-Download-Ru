/*-----------------------------------------------------------------------
*��12��  ע�뼼��
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// HostProc.cpp : Defines the entry point for the console application.
// һ��������ʾ����ע�������

#include "stdafx.h"
#include <windows.h>

//�̶���ַ�������Ǳ���Ϊx64ʱ���ڱ�������а������ַ����ΪNO,�������
#pragma comment(linker,"/Base:0x20000000")

DWORD WINAPI ThreadProc(LPVOID lpParameters);

int main(int argc, char* argv[])
{
	printf("[Process]PID = %d, Host Process Working!\n",GetCurrentProcessId());
	//LoadLibrary("F:\\Program2016\\DllInjection\\SecurityMod.dll");
	CreateThread(NULL,0,ThreadProc,NULL,0,NULL);
	while (TRUE)
	{
		Sleep(20000);
	}
	
	return 0;
}

//For APC Inject
DWORD WINAPI ThreadProc(LPVOID lpParameters)
{
	while (TRUE)
	{
		SleepEx(3000,TRUE);
	}
	printf("Waked up from Sleep.\n");
	return 0 ;
}
