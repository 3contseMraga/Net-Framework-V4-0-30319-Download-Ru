/*-------------------------------------------------------
/*  ����������ܡ� ��6�� �����㷨   
/*   MD5������Դ��
/*  (c) www.PEDIY.com  by �θ� 2002.9
/ *  Modified by cnbragon 2006
-------------------------------------------------------*/

////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <windows.h>
#include "resource.h"

#include "miracl.h"
#include "mirdef.h"

#if _DEBUG
  #pragma comment(linker,"/NODEFAULTLIB:LIBC")
#endif

#pragma comment(lib,"ms32.lib")

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
	TCHAR szWaterMark[]="Modified by cnbragon belong to iNsidE PaNdoRa's BoX and Reverse Code Team";
	miracl* mip=mirsys(500,16);
	TCHAR szName[MAXINPUTLEN]={0};
	TCHAR szSerial[MAXINPUTLEN]={0};
	TCHAR szBuffer[MAXINPUTLEN]={0};
    long dtLength,lsLength;
	int i;
	big n,e,c,m;

	dtLength=GetDlgItemText(hWnd, IDC_Name, szName, sizeof(szName)/sizeof(TCHAR)+1);  
	if (dtLength==0)
	{
		SetDlgItemText(hWnd, IDC_Serial, "Wrong Serial!");
		return FALSE;
	}
	lsLength=GetDlgItemText(hWnd,IDC_Serial,szSerial,sizeof(szSerial)/sizeof(TCHAR)+1);
	if (lsLength==0)
	{
		SetDlgItemText(hWnd,IDC_Serial,"Wrong Serial!");
		return FALSE;
	}
	
	for(i=0;szSerial[i]!=0;i++)      // �����������к��Ƿ�Ϊ16��������Ϊcinstr(M,szSerial)ʹ����׼��
	{
		if(isxdigit(szSerial[i])==0)
		{
			SetDlgItemText(hWnd,IDC_Serial,"Wrong Serial!");
			return FALSE;
		}
	}
    
	/* MIRACL������������� 
     * p=C75CB54BEDFA30AB
     * q=A554665CC62120D3
     * n=80C07AFC9D25404D6555B9ACF3567CF1
     * d=651A40B9739117EF505DBC33EB8F442D
     * e=10001
     * 128 bit
	 */
	mip->IOBASE=16;               // �趨16����ģʽ	                            
	n=mirvar(0);                  // ��ʼ������ 
	e=mirvar(0);     
	m=mirvar(0);                  // m �����ģ�ע����
	c=mirvar(0);                  // c ������

	cinstr(m,szSerial);	                           // ����������к�ת���ɴ��� ������szSerial	
	cinstr(n,"80C07AFC9D25404D6555B9ACF3567CF1");  // ��ʼ��ģ��n	 
	cinstr(e,"10001");  

	if(compare(m,n)==-1)                           // m < n ,���ܶ���Ϣm����
	{
		powmod(m,e,n,c);                               // �������� c= m ^ e mod n

		big_to_bytes(0,c,szBuffer,FALSE);                 // �� c �Ӵ���ת�����ֽ�����    
                              
		mirkill(n);
		mirkill(e);
		mirkill(m);
		mirkill(c);  
		mirexit();

		if(lstrcmp(szName, szBuffer)!=0)               // �Ƚ����������кż��ܺ����ݵ��Ƿ���ȣ�
		{
			SetDlgItemText(hWnd,IDC_Serial,"Wrong Serial!");
			return FALSE;
		}
		else
		{
			SetDlgItemText(hWnd,IDC_Serial,"Success!");
			return TRUE;
		}
	}
	else
	{
		SetDlgItemText(hWnd,IDC_Serial,"Wrong Serial!");
		return FALSE;
	}
}
