/*-----------------------------------------------------------------------
��12��  ע�뼼��
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// MsgDll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>


VOID Msg();

DWORD WINAPI ThreadShow(LPVOID lpParameter);
void WriteLog(char *info,char *logfilename);

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		CreateThread(NULL,0,ThreadShow,NULL,0,NULL);
	}
    return TRUE;
}

DWORD WINAPI ThreadShow(LPVOID lpParameter)
{
	char szPath[MAX_PATH]={0};
	char szBuf[1024]={0};
	
	GetModuleFileName(NULL,szPath,MAX_PATH);
	sprintf(szBuf,"Dll��ע�뵽���� %s [Pid = %d]\n",szPath,GetCurrentProcessId());
	//�����ַ�ʽ��ʾ�Լ��Ĵ���
	//1.����̨
	//fprintf((FILE*)GetStdHandle(STD_ERROR_HANDLE),"%s",szBuf);
	printf("%s",szBuf);
	//2.������
	OutputDebugString(szBuf);
	//3.Msgbox
	Sleep(800);
	MessageBox(NULL,szBuf,"Dll Inject",MB_OK);
	return 0 ;
}

void WriteLog(char *info,char *logfilename)
{
	HANDLE	hFile;
	static char	tmp[100];
	DWORD	dwwritten=0;
	static SYSTEMTIME systime;
	GetLocalTime(&systime);
	sprintf(tmp,"%04dd-%02d-%02d %02d:%02d:%02d  ",systime.wYear,systime.wMonth,systime.wDay,systime.wHour,systime.wMinute,systime.wSecond);
	hFile=CreateFile(logfilename,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE!=hFile) 
	{
		SetFilePointer(hFile,0,0,SEEK_END);
		WriteFile(hFile,tmp,lstrlen(tmp),&dwwritten,NULL);
		WriteFile(hFile,info,lstrlen(info),&dwwritten,NULL);
		WriteFile(hFile,"\r\n",2,&dwwritten,NULL);
		CloseHandle(hFile);
	}
	
}
//��������
VOID Msg()
{

}