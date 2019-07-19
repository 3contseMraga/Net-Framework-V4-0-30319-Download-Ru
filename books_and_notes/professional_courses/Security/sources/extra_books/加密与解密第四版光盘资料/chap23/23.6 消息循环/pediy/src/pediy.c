/*-----------------------------------------------------------------------
��23��  ����Ķ��ο���
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

#include <windows.h>
#include "resource.h"

#define ID_EDIT     1
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;

TCHAR szAppName[] = TEXT ("pediy"); 

/*-------------------------------------------------------------*/
/* �Ż�                                                        */
/*-------------------------------------------------------------*/

//���ʹ�� VS.NET�������������ã��ڡ������������������ѡ����Ŀ���һ���ѡ�����ԡ�->����������->���߼���->����ڵ㡱 
#pragma comment(linker, "/ENTRY:EntryPoint") 



//////////////////////////////////////////////////////////////////////////////////////////////////// 
//��ں��� 
//ʹ�������Լ�����ں�����������������Ĭ���ṩ��һ��ѳ����ʼ�������Ĵ��� 
//Ϊ����һ����ͨ�� Win32SDK ��������ʹ�����ַ���������ĺ��������� WinMain() ��������������Ӧ�Ĳ��� 
void EntryPoint() 
{ 
      ExitProcess(WinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOWNORMAL)); 
}
//////////////////////////////////////////////////////////////////////////////////////////////////// 


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
     HWND     hwnd ;
     MSG      msg ;
     WNDCLASS wndclass ;
     
     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
     wndclass.lpfnWndProc   = WndProc ;
     wndclass.cbClsExtra    = 0 ;
     wndclass.cbWndExtra    = 0 ;
     wndclass.hInstance     = hInstance ;
     wndclass.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_ICON));
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
     wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
     wndclass.lpszMenuName  = szAppName ;
     wndclass.lpszClassName = szAppName ;
     
     if (!RegisterClass (&wndclass))                                // ��Unicode��������ʱ,����������Windows 9xϵͳ
     {
          MessageBox (NULL, TEXT ("This program requires Windows NT!"),
                      szAppName, MB_ICONERROR) ;
          return 0 ;
     }
     
     hwnd = CreateWindow (szAppName, TEXT ("�����������"),
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          400, 300,
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
	 static HWND hwndEdit ;
     
     switch (message)
     {
   case WM_CREATE :
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

	 case WM_COMMAND:
 
          
          switch (LOWORD (wParam))
          {
         
               
        //  case IDM_APP_EXIT:
             //  SendMessage (hwnd, WM_CLOSE, 0, 0) ;
             //  return 0 ;
               
         
          case IDM_APP_ABOUT:
               MessageBox (hwnd, TEXT ("�����������\n")
                                 TEXT ("(c) �θ�, 2002"),
                           szAppName, MB_ICONINFORMATION | MB_OK) ;
               return 0 ;
          }
          break ;

               
     case WM_DESTROY:
          PostQuitMessage (0) ;
          return 0 ;
     }
     return DefWindowProc (hwnd, message, wParam, lParam) ;
}
