
/*-----------------------------------------------------------------------
��17��  �����������
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

#include <windows.h>
#include <commdlg.h>
#include "resource.h"

/*-------------------------------------------------------------*/
/* �����ӳ�����ȫ�ֱ���������                                  */
/*-------------------------------------------------------------*/
#define ID_EDIT     1
#define MAXINPUTLEN 8

LRESULT CALLBACK WndProc  (HWND, UINT, WPARAM, LPARAM) ;
BOOL    CALLBACK RegisterDlgProc (HWND, UINT, WPARAM, LPARAM) ;
BOOL    CALLBACK MenuOpen(HWND) ;
void    Decrypt (DWORD* ,DWORD,DWORD );

BOOL    bSucceed=FALSE;
TCHAR   cCode[30]={0}; 

/*-------------------------------------------------------------*/
/* WinMain �� ����WIN32�ĳ�������                            */
/*-------------------------------------------------------------*/

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
     static TCHAR szAppName[] = TEXT ("PEDIY") ;
     MSG          msg ;
     HWND         hwnd ;
     WNDCLASS     wndclass ;
     
     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
     wndclass.lpfnWndProc   = WndProc ;
     wndclass.cbClsExtra    = 0 ;
     wndclass.cbWndExtra    = 0 ;
     wndclass.hInstance     = hInstance ;
     wndclass.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_ICON)) ;
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
     wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
     wndclass.lpszMenuName  = szAppName ;
     wndclass.lpszClassName = szAppName ;
     
     if (!RegisterClass (&wndclass))
     {
          MessageBox (NULL, TEXT ("This program requires Windows NT!"),
                      szAppName, MB_ICONERROR) ;
          return 0 ;
     }
     
     hwnd = CreateWindow (szAppName, TEXT ("About Box Demo Program"),
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          NULL, NULL, hInstance, NULL) ;
     
     ShowWindow (hwnd, iCmdShow) ;
     UpdateWindow (hwnd) ; 
     
     while (GetMessage (&msg, NULL, 0, 0))
     {
          TranslateMessage (&msg) ;
          DispatchMessage (&msg) ;
     }
     return msg.wParam ;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     static HINSTANCE hInstance ;
	 static HWND hwndEdit ;
     
     switch (message)
     {
	 case WM_CREATE :
		  hInstance = ((LPCREATESTRUCT) lParam)->hInstance ;
          hwndEdit = CreateWindow (TEXT ("edit"), NULL,
                         WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
                                   WS_BORDER | ES_LEFT | ES_MULTILINE |
                                   ES_AUTOHSCROLL | ES_AUTOVSCROLL,
                         0, 0, 0, 0, hwnd, (HMENU) ID_EDIT,
                         ((LPCREATESTRUCT) lParam) -> hInstance, NULL) ;
           return 0 ;
          
     case WM_SETFOCUS :
          SetFocus (hwndEdit) ;	  
          return 0 ;
          
     case WM_SIZE : 
          MoveWindow (hwndEdit, 0, 0, LOWORD (lParam), HIWORD (lParam), TRUE) ;
          return 0 ;

     case WM_COMMAND :
          switch (LOWORD (wParam))
          {
		  case IDM_APP_OPEN:
			   MenuOpen(hwndEdit);
	           break ;
		  case IDM_APP_EXIT:
			   SendMessage (hwnd, WM_CLOSE, 0, 0) ;
               break ;
		  case IDM_APP_REGISTER:
			   DialogBox (hInstance, TEXT ("REGISTERBOX"), hwnd, RegisterDlgProc) ;
			   break ;

          case IDM_APP_ABOUT :
			   MessageBoxA (NULL, TEXT ("���������ݽ�ϼ���     (c) �θ�, 2003"), TEXT ("��������ܼ�����Ļ��"), 0) ;          
               break ;
          }
          return 0 ;
          
     case WM_DESTROY :
          PostQuitMessage (0) ;
          return 0 ;
     }
     return DefWindowProc (hwnd, message, wParam, lParam) ;
}

/*-------------------------------------------------------------*/
/* RegisterDlgProc �� ע��Ի�����                            */
/*-------------------------------------------------------------*/
BOOL CALLBACK RegisterDlgProc (HWND hDlg, UINT message, 
                            WPARAM wParam, LPARAM lParam)
{

     switch (message)
     {
     case WM_INITDIALOG :
		  //��ʼ��edit�ؼ�IDC_TXT0�ַ�����
		  SendDlgItemMessage(hDlg, IDC_TXT0, EM_LIMITTEXT, MAXINPUTLEN, 0);
          return TRUE ;
          
     case WM_COMMAND :
          switch (LOWORD (wParam))
          {
          case IDOK :
			   GetDlgItemText(hDlg,IDC_TXT0,cCode,sizeof(cCode)/sizeof(TCHAR)+1);
			   MessageBoxA (NULL, TEXT ("��л��ע�ᣬ��ע������ȷ��򿪹��ܿ���!"), 
				          TEXT ("(c) �θ�, 2002"), 0) ;    
          case IDCANCEL :
               EndDialog (hDlg, 0) ;
               return TRUE ;
          }
          break ;
     }
     return FALSE ;
}

/*-------------------------------------------------------------*/
/* MenuOpen�� �˵����ܣ����ļ�                               */
/*-------------------------------------------------------------*/
BOOL CALLBACK MenuOpen(HWND hWnd)  
{
	
	HANDLE hFile;	
	DWORD szTemp,address1,address2,Size,k;
	DWORD* ptr;
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

	
	_asm mov address1,offset begindecrypt   // ȡ�����ܴ����׵�ַ
    _asm mov address2,offset enddecrypt     // ȡ�����ܴ���ĩ��ַ

	//�������ע�������һ���ı任���õ���Կk ��k = F��ע���룩
	k=1;
	for (unsigned int i=0;i<strlen(cCode);i++) 
	{
		k = k*6 + cCode[i];
	}

	Size=address2-address1;
	ptr=(DWORD*)address1;

	if(!bSucceed)
	Decrypt (ptr,Size,k); //ִ�н��ܺ���

	//��Decrypt()����û�����ȷ�Ĵ��룬����쳣
	try
	{
	 // ��ʮ�����ƹ��������������д��붨λ���ܴ�����ʼ��
      _asm inc eax // ��ʮ�����ƹ����ж�Ӧ0x40  
      _asm dec eax // ��ʮ�����ƹ����ж�Ӧ0x48

/***************************************************************/
/* ��Ҫ���ܵĴ�����ʼ��ǩ����address1��ַ                     */
/***************************************************************/
begindecrypt: 

	if(GetOpenFileName (&ofn))
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

					     SetWindowText(hWnd,pBuffer); // ���ڴ��е�������ʾ���ı��༭����
					     CloseHandle(hFile);
					     delete pBuffer;  // �ͷ��ڴ�
						 bSucceed=true;   //���ñ�־����ζ���������ȷ���Ժ��ٵ��á��򿪡�����ʱ�������ٵ���Decrypt()����
					     return TRUE;    // �˳�����
				     }
			     }
			     CloseHandle(hFile);
		 
		}
		MessageBoxA (NULL, TEXT ("�������ļ�����64K������ԭ��!"), TEXT ("���ļ�ʧ�ܣ�"), 0) ;
	}

	delete pBuffer; 
	bSucceed=true;
/*****************************************************************/
/* ��Ҫ���ܵĴ��������ǩ ����address2��ַ                       */
/*****************************************************************/
enddecrypt: 

	 // ��ʮ�����ƹ��������������д��붨λ���ܴ��������
	  _asm inc eax // ��ʮ�����ƹ����ж�Ӧ0x40  
      _asm dec eax // ��ʮ�����ƹ����ж�Ӧ0x48
	  return TRUE;

	}
	//�����ַaddress1��address2֮�����ݽ��ܲ��ɹ�����ִ����Щ��������ʱ�ض��쳣���쳣��ͻ��������´��봦��
	catch (...)
	{
		MessageBoxA (NULL, TEXT ("��ע�ᣬ�Ի�������Ĺ��� !"), TEXT ("��ʾ"), 0) ;  
		Decrypt (ptr,Size,k);//��ע���벻��ȷ���������ٴε���Decrypt()��address1��address2֮�����ݻ�ԭ
		return FALSE;
	}

}

/*-------------------------------------------------------------*/
/* Decrypt �� �����ӳ���                                      */
/*-------------------------------------------------------------*/
// �Ե�ַaddress1��address2֮�����ݽ���
// �������� = ԭʼ���� XOR value
//
// ע�⣺�粻��VirtualQuery��VirtualProtect�����ı������ԣ��������PE���߽�.text������Ϊ����дģʽ��Ч��һ����

void Decrypt (DWORD* pData,DWORD Size,DWORD value)
{
    //����Ҫ�����Ǹı���һ�������ڴ���ڴ汣��״̬���Ա�������ɴ�ȡ����
	MEMORY_BASIC_INFORMATION mbi_thunk; 
    //��ѯҳ��Ϣ
	VirtualQuery(pData, &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION)); 
	//�ı�ҳ��������Ϊ��д��
	VirtualProtect(mbi_thunk.BaseAddress,mbi_thunk.RegionSize,PAGE_READWRITE, &mbi_thunk.Protect); 

	Size=Size/0x4; //�����ݹ���Ҫ���Ĵ���
	//����begindecrypt��enddecrypt��ǩ��������
	while(Size--)
	{
		*pData=(*pData)^value;
		pData++;
	}
	
	//�ָ�ҳ��ԭ�������ԡ�
	DWORD dwOldProtect; 
	VirtualProtect(mbi_thunk.BaseAddress,mbi_thunk.RegionSize, mbi_thunk.Protect, &dwOldProtect); 

}


