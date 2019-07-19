/*-----------------------------------------------------------------------
��17��  �����������
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

#include <windows.h> 
#include "resource.h"
#include<stdlib.h>

/*-------------------------------------------------------------*/
/* �����ӳ�����ȫ�ֱ���������                                  */
/*-------------------------------------------------------------*/

#define MAXINPUTLEN 8

BOOL    CALLBACK MainDlg   (HWND, UINT, WPARAM, LPARAM) ;
BOOL    PopFileOpenDlg(void);
int     encrypt(HWND);
TCHAR   szFileName[MAX_PATH ] ;
HINSTANCE hInst;

/*-------------------------------------------------------------*/
/* WinMain �� ����WIN32�ĳ�������                            */
/*-------------------------------------------------------------*/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
	hInst = hInstance;
	DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_MAINDLG), NULL, MainDlg, NULL);
	return 0;	
}


/*-------------------------------------------------------------*/
/*  MainDlg �� ���Ի�����                                      */
/*-------------------------------------------------------------*/

BOOL CALLBACK MainDlg (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{			

	switch (message) 
	{
	case WM_INITDIALOG:
		int DlgHeight,DlgWidth,x,y;
		RECT DlgRect;
		RECT DesktopRect;
		HWND hwindow;
		SendMessage(hDlg,WM_SETICON,ICON_BIG,LPARAM(LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON)))); //����ͼ��
		SendDlgItemMessage(hDlg, IDC_TXT0, EM_LIMITTEXT, MAXINPUTLEN, 0);      //��ʼ��edit�ؼ�IDC_TXT0�ַ�����
		EnableWindow(GetDlgItem(hDlg,IDC_ENCRYPT),FALSE);


		// �����ڶ�λ��Ļ����
		GetWindowRect(hDlg,&DlgRect); 
		hwindow = GetDesktopWindow(); 
		GetWindowRect(hwindow,&DesktopRect);

		DlgHeight = DlgRect.bottom - DlgRect.top;
		DlgWidth = DlgRect.right - DlgRect.left;
		x=(DesktopRect.right+DesktopRect.left-DlgWidth)/2;
		y =(DesktopRect.bottom+DesktopRect.top-DlgHeight)/2;
		MoveWindow(hDlg,x,y,DlgWidth,DlgHeight,FALSE);	 
		break;  
		
	case WM_CLOSE:
		DestroyWindow(hDlg); 		                       
		break;

	case WM_COMMAND: 
		switch (LOWORD (wParam))
		{
		case IDC_OPENFILE:
			if(PopFileOpenDlg())
			{
			SendMessage(GetDlgItem(hDlg,IDC_TXTFILE), WM_SETTEXT, 0, (LPARAM)szFileName);
			EnableWindow(GetDlgItem(hDlg,IDC_ENCRYPT),TRUE);
			}
			break;
			
		case IDC_ENCRYPT:
			encrypt(hDlg);
			break;

		case IDC_EXIT1: 
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}

		return TRUE;
		break;
	}
	return FALSE;
}
/*-------------------------------------------------------------*/
/*  PopFileOpenDlg �� ���ļ�                                 */
/*-------------------------------------------------------------*/
BOOL PopFileOpenDlg()
{

	static OPENFILENAME ofn ;
	static TCHAR szFilter[] =TEXT ("EXE Files (*.exe)\0*.exe\0") \
	                         TEXT ("All Files (*.*)\0*.*\0\0") ;

	szFileName[0] = '\0';                         
	ZeroMemory(&ofn, sizeof(ofn));                             // ��ʼ��OPENFILENAME�ṹ
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.lpstrFilter       = szFilter ;
	ofn.lpstrFile         = szFileName ;         
	ofn.nMaxFile          = MAX_PATH ;   
	ofn.nMaxFileTitle     = MAX_PATH ;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT ; 
	return GetOpenFileName (&ofn) ;
}

/*-------------------------------------------------------------*/
/*  encrypt �� ��address1��address2֮��Ĵ�����м��ܴ���      */
/*-------------------------------------------------------------*/
int encrypt(HWND hWnd)
{

	TCHAR szBuffer[30]={0}; 
    DWORD address1,address2,offset,Size,k,nbWritten,szTemp;;
	HANDLE hFile;	
	DWORD* ptr;
	         
	hFile = CreateFile(
			szFileName,
			GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ, 
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	
	if ( hFile == INVALID_HANDLE_VALUE )
	 {
		MessageBox(NULL,"I can't access this file !","Open error",MB_ICONEXCLAMATION);
		return 1;
	}


	GetDlgItemText(hWnd,IDC_TXT1,szBuffer,sizeof(szBuffer)/sizeof(TCHAR)+1);
	 // ���������Ƿ�Ϊ16������
	for(DWORD i=0;szBuffer[i]!=0;i++)                  
	{
		if(isxdigit(szBuffer[i])==0)
		{
			MessageBox(NULL,"������ʮ��������!","aborted",MB_ICONEXCLAMATION);
			return 0;
		}
	}
	address1 = (DWORD)strtoul( szBuffer, NULL, 16 ); 

	GetDlgItemText(hWnd,IDC_TXT2,szBuffer,sizeof(szBuffer)/sizeof(TCHAR)+1);
	 // ���������Ƿ�Ϊ16������
	for(i=0;szBuffer[i]!=0;i++)                 
	{
		if(isxdigit(szBuffer[i])==0)
		{
			MessageBox(NULL,"������ʮ��������!","aborted",MB_ICONEXCLAMATION);
			return 0;
		}
	}
	address2 = (DWORD)strtoul( szBuffer, NULL, 16 ); 

	//�������ע�������һ���ı任���õ���Կk ��k = F��ע���룩
	k=1;
	GetDlgItemText(hWnd,IDC_TXT0,szBuffer,sizeof(szBuffer)/sizeof(TCHAR)+1);
	for ( i=0;i<strlen(szBuffer);i++) 
	{
		k = k*6 + szBuffer[i];
	}

	Size=address2-address1;
	Size=Size/0x4; 	//����ʱ��ÿ����� DWORD���ݣ�Size��Ϊ������Ҫ���Ĵ���
	offset=address1;
 	for (i=0;i<Size;i++)
	{
		SetFilePointer(hFile,offset,NULL,FILE_BEGIN); 
		ReadFile(hFile,szBuffer, 4, &szTemp, NULL);//��ȡDWORD�ֽڵ��ļ�����
		ptr=(DWORD*)szBuffer;
		*ptr=(*ptr)^k;
		SetFilePointer(hFile,offset,NULL,FILE_BEGIN); 

		if(!WriteFile(hFile,ptr,4,&nbWritten,NULL))// д���ļ�
		{
			MessageBox(NULL,"Error while patching !","Patch aborted",MB_ICONEXCLAMATION);
			CloseHandle(hFile);
		
			return 1;
		}
		offset=offset+4;
	}

	CloseHandle(hFile);
	MessageBox(NULL,"Patch successfull !","Patch",MB_ICONINFORMATION);
	return 1;

}
