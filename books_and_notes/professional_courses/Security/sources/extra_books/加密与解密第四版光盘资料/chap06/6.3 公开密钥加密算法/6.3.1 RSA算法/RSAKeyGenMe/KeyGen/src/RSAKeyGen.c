/*-------------------------------------------------------
/*  ����������ܡ� ��6�� �����㷨   
/*   RSA *KeyGenMe* 's KeyGen
/*  (c) www.PEDIY.com  �θ� 2002.12
 *  Modified by cnbragon 2006
-------------------------------------------------------*/

////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <windows.h>
#include "resource.h"

#include "miracl.h"
#include "mirdef.h"

#pragma comment( lib, "ms32.lib") 


#if _DEBUG
  #pragma comment(linker,"/NODEFAULTLIB:LIBC")
#endif

/*-------------------------------------------------------------*/
/* �����ӳ�����ȫ�ֱ���������                                  */
/*-------------------------------------------------------------*/
HINSTANCE	hInst;
TCHAR szConstName[]="pediy";
#define MAXINPUTLEN 200

/*-------------------------------------------------------------*/
/*  ��������                                                   */
/*-------------------------------------------------------------*/
BOOL    CALLBACK MainDlg   (HWND, UINT, WPARAM, LPARAM) ;
BOOL    CALLBACK AboutDlgProc (HWND, UINT, WPARAM, LPARAM) ;
BOOL    GenerateSerial( HWND) ;

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
/* AboutDlgProc �� ���ڴ���                                    */
/*-------------------------------------------------------------*/

BOOL CALLBACK AboutDlgProc (HWND hDlg, UINT message, 
                            WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case  WM_LBUTTONDOWN:
			PostMessage(hDlg, WM_NCLBUTTONDOWN, HTCAPTION, 0);
			return TRUE ;
        
		case WM_COMMAND :
			switch (LOWORD (wParam))  		            
			{
				case IDOK :
				case IDCANCEL :
				EndDialog (hDlg, 0) ;
				return TRUE ;
			}
			 break ;
	 }
     return FALSE ;
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
		case IDC_Name:		
			GenerateSerial(hDlg);
			break;

		case IDC_About :
			DialogBox (hInst, MAKEINTRESOURCE (IDD_ABOUT), hDlg, AboutDlgProc) ;
			break;	

		case IDC_Exit:
			PostQuitMessage(0);
		}
		break;
	case WM_INITDIALOG:
		SendMessage(hDlg,WM_SETICON,(WPARAM) 1,(LPARAM) LoadIconA(hInst,MAKEINTRESOURCE(IDI_ICON)));
		SendDlgItemMessage(hDlg,IDC_Name,EM_LIMITTEXT,16,0);
		SendDlgItemMessage(hDlg,IDC_Name,WM_SETTEXT,0,(LPARAM)szConstName);
		                                                             

		break;
	}
     return 0;
}


/*-------------------------------------------------------------*/
/* MD5Hash �� MD5����������                                    */
/*-------------------------------------------------------------*/
BOOL GenerateSerial(HWND hWnd) 
{
	miracl* mip=mirsys(500,16);
	TCHAR szName[MAXINPUTLEN]={0};
	TCHAR szSerial[MAXINPUTLEN]={0};
	TCHAR szBuffer[MAXINPUTLEN]={0};
    long dtLength;
	big n,d,c,m;

	dtLength=GetDlgItemText(hWnd, IDC_Name, szName, sizeof(szName)/sizeof(TCHAR)+1);  
	if (dtLength==0)
	{
		SetDlgItemText(hWnd, IDC_Serial, "please input name");
		return FALSE;
	}
	
	/* MIRACL������������� 
     * p=C75CB54BEDFA30AB
     * q=A554665CC62120D3
     * n=80C07AFC9D25404D6555B9ACF3567CF1
     * d=651A40B9739117EF505DBC33EB8F442D
     * e=10001
     * 128 bit
	 */
	mip->IOBASE=16;                                // 16����ģʽ
	c=mirvar(0);                                   // MIRACL�Ĵ�������
	n=mirvar(0);
	d=mirvar(0);
	m=mirvar(0);

	bytes_to_big(dtLength,szName,c);                    // ������ת���ɴ���
	cinstr(n,"80C07AFC9D25404D6555B9ACF3567CF1");  // ��ʼ��ģ��n	 
	cinstr(d,"651A40B9739117EF505DBC33EB8F442D");  // ��ʼ��˽Կd
	powmod(c,d,n,m);	         	               // ����m = (c ^ d) mod n
	cotstr(m,szSerial);                            // ��m��16���ƴ���ʾд��szSerial�У���Ϊע����
    
	SetDlgItemText(hWnd, IDC_Serial,szSerial);       
    
	mirkill(c);
	mirkill(n);
	mirkill(d);
	mirkill(m);
	mirexit();
	return TRUE;
}
