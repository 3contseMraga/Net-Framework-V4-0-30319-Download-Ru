/*-----------------------------------------------------------------------
��23��  ����Ķ��ο���
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

/************************************************************************/
/* ��������Ϣ�ӿ����˼·�ο���peansen�ڿ�ѩ�����ȫ��̳��һƪ���£��ڴ˱�ʾ��л��

�� ��: ���±��������ӷ���
������:peansen
ʱ ��: 2005-10-05 22:13 
ԭ������:http://bbs.pediy.com/showthread.php?t=17376                                                                   */
/************************************************************************/

#include <windows.h>
#include "peplug.h"
#include "commdlg.h"
BOOL MenuOpen(HWND) ;
BOOL MenuSave(HWND);

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
     return TRUE ;
}

void _cdecl MyWndProc (const DWORD reversed,HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	    case WM_COMMAND:  
		   switch (LOWORD (wParam))
		   {
			 case 40002://Open
			    MenuOpen(hwnd);
				break;
			 case 40003://Save
			    MenuSave(hwnd);
				break;
			 case 40005://Exit
			    SendMessage (hwnd, WM_CLOSE, 0, 0) ;
				break;				
		   }
	 break ;
	}
}


BOOL MenuOpen(HWND hWnd)  // ���ļ�����
{
	
	HANDLE hFile;	
	DWORD szTemp;
	long FileSize;
	static OPENFILENAME ofn ;
	TCHAR  *pBuffer = new TCHAR [0x10000];     // ����64K�ڴ�
	TCHAR szFileName[MAX_PATH ], szTitleName[MAX_PATH ] ;
	static TCHAR szFilter[] =TEXT ("TXT Files (*.TXT)\0*.txt\0") \
				TEXT ("All Files (*.*)\0*.*\0\0") ;

	szFileName[0] = '\0';                         
	ZeroMemory(&ofn, sizeof(ofn));                             // ��ʼ��OPENFILENAME�ṹ
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = hWnd ;
	ofn.lpstrFilter       = szFilter ;
	ofn.lpstrFile         = szFileName ;         
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrFileTitle    = szTitleName ;          
	ofn.nMaxFileTitle     = MAX_PATH ;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT ;           


	if(GetOpenFileName (&ofn))
	{
		HWND hwndEdit = GetDlgItem(hWnd, 1);
		if (hwndEdit )
		{
		     hFile = CreateFile( szFileName,	
					 GENERIC_READ ,
					 NULL,
					 NULL,
					 OPEN_EXISTING,
					 FILE_ATTRIBUTE_NORMAL ,
					 NULL);
		     if( hFile != INVALID_HANDLE_VALUE )
		     {
			     FileSize=GetFileSize(hFile,&szTemp);  // ��ȡ�ļ���С
			     if(FileSize<0x10000)                  // ���ļ�С��64K���ȡ
			     {
				     if(ReadFile(hFile, pBuffer, FileSize, &szTemp, NULL)!=NULL) // ��ȡ����
				     {

					     SetWindowText( hWnd,szFileName);       // �ڴ��ڱ�������ʾ�򿪵��ļ���
					     SetWindowText(hwndEdit,pBuffer);       // ���ڴ��е�������ʾ���ı��༭����
					     CloseHandle(hFile);
					     delete pBuffer;  // �ͷ��ڴ�
					     return TRUE;   // �˳�����
				     }
			     }
			     CloseHandle(hFile);
			    	
		     }	 
		}
		MessageBoxA (NULL, TEXT ("�������ļ�����64K������ԭ��!"), TEXT ("���ļ�ʧ�ܣ�"), 0) ;
	}


	delete pBuffer; 
	return FALSE;

}

BOOL MenuSave(HWND hWnd)  // ���湦��
{

	TCHAR szFileName[MAX_PATH ], szTitleName[MAX_PATH ] ;
	
	HANDLE hFile;
	TCHAR  *pBuffer = new TCHAR [0x10000];     // ����64�ڴ�
	DWORD dwLen;
	DWORD NumberOfBytesRW;
	static OPENFILENAME ofn ;
	static TCHAR szFilter[] =TEXT ("TXT Files (*.TXT)\0*.txt\0") \
		                 TEXT ("All Files (*.*)\0*.*\0\0") ;

	szFileName[0] = '\0';                         
	ZeroMemory(&ofn, sizeof(ofn));                             // ��ʼ��OPENFILENAME�ṹ
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = hWnd ;
	ofn.lpstrFilter       = szFilter ;
	ofn.lpstrFile         = szFileName ; 
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrFileTitle    = szTitleName ;          
	ofn.nMaxFileTitle     = MAX_PATH ;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT ;           
	ofn.lpstrDefExt       = TEXT("txt");                        // �����Ĭ����չ��


	if(GetSaveFileName (&ofn))
	{
		HWND hwndEdit = GetDlgItem(hWnd, 1);
		if (hwndEdit)
		{
			dwLen = GetWindowText(hwndEdit, pBuffer, 0x1000); 
			if (dwLen<0x10000 && dwLen>0 )
			{
				hFile = CreateFile( szFileName,	
						GENERIC_WRITE,
						NULL,
						NULL,
						CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
						NULL);
				if( hFile != INVALID_HANDLE_VALUE )
				{
					if(WriteFile (hFile,pBuffer,dwLen,&NumberOfBytesRW, NULL))
					{
						SetWindowText( hWnd,szFileName);  
						CloseHandle(hFile);
						delete pBuffer;  
						return TRUE;
					}
					
					CloseHandle(hFile);
				}
			}
		}
		MessageBoxA (NULL, TEXT ("�������ļ�̫����ļ�������!"), TEXT ("�����ļ�ʧ�ܣ�"), 0) ;
	}
	delete pBuffer; 
	return FALSE;

}
