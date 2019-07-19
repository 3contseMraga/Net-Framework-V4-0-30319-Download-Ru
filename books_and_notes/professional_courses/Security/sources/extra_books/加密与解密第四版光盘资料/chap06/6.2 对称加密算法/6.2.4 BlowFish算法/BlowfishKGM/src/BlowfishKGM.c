/*-------------------------------------------------------
/*  ����������ܡ� ������ ��6�� �����㷨   
/*   Blowfish KeyGenMe
/*  (c) www.PEDIY.com  by cnbragon 2007
-------------------------------------------------------*/

////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <windows.h>
#include "resource.h"
#include "blowfish.c"

/*-------------------------------------------------------------*/
/* �����ӳ�����ȫ�ֱ���������                                  */
/*-------------------------------------------------------------*/
HINSTANCE	hInst;
#define MAXINPUTLEN 200

/*-------------------------------------------------------------*/
/*  ��������                                                   */
/*-------------------------------------------------------------*/
BOOL    CALLBACK MainDlg   (HWND, UINT, WPARAM, LPARAM) ;
BOOL    CheckSerial( HWND) ;

/*-------------------------------------------------------------*/
/* WinMain �� ����WIN32�ĳ�������                            */
/*-------------------------------------------------------------*/

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	hInst=hInstance;
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAINDLG), NULL, (DLGPROC)MainDlg,0);
	return 0;
}

/*-------------------------------------------------------------*/
/*  MainDlg �� ���Ի�����                                      */
/*-------------------------------------------------------------*/
BOOL CALLBACK MainDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{	

	switch (message)
	{    
	case WM_CLOSE:
		EndDialog(hDlg,0);
		break;


	case WM_COMMAND:
		switch (LOWORD(wParam))
		{ 
		case IDC_Check:		
			CheckSerial(hDlg);
			break;	

		case IDC_Exit:
			PostQuitMessage(0);
		}
		break;
	case WM_INITDIALOG:
		SendMessage(hDlg,WM_SETICON,(WPARAM) 1,(LPARAM) LoadIconA(hInst,MAKEINTRESOURCE(IDI_ICON)));  
		                                                             

		break;
	}
     return 0;
}


BOOL CheckSerial(HWND hWnd) 
{
	TCHAR szWaterMark[]="coded by cnbragon belong to iNsidE PaNdoRa's BoX and Reverse Code Team";
	BLOWFISH_CTX context;
	long lsLength;
	int i;
	TCHAR szCipher[MAXINPUTLEN]={0};
	TCHAR szBuffer[MAXINPUTLEN]={0};
	TCHAR szSerial[MAXINPUTLEN]={0};
	BYTE bf_key[MAXINPUTLEN]={0};
    DWORD xL,xR,dwVolumeSn,dwXor;

	xL=xR=dwVolumeSn=dwXor=0;
	    
	lsLength=GetDlgItemText(hWnd,IDC_Serial,szSerial,sizeof(szSerial)/sizeof(TCHAR)+1);
	if (lsLength!=32)
	{
		SetDlgItemText(hWnd,IDC_Serial,"Wrong Serial!");
		return FALSE;
	}
	
	for (i=0;i<16;i++)
	{
		sscanf((szSerial+i*2),"%02X",(szBuffer+i));
	}
	
	memcpy(bf_key,szBuffer,8);
	Blowfish_Init(&context,bf_key,8);
	xL=*(DWORD*)(&szBuffer[8]);
	xR=*(DWORD*)(&szBuffer[12]);
	Blowfish_Decrypt(&context,&xL,&xR);
	dwXor=xL^xR;
	memset(&context,0,sizeof(BLOWFISH_CTX));
	    
	GetVolumeInformation("C:\\",NULL,0,&dwVolumeSn,NULL,NULL,NULL,0);
		
	if (dwXor==dwVolumeSn)
	{
		SetDlgItemText(hWnd,IDC_Serial,"Success!");
		return TRUE;
	} 
	else
	{
		SetDlgItemText(hWnd,IDC_Serial,"Wrong Serial!");
		return FALSE;
	}

	return TRUE;
}
