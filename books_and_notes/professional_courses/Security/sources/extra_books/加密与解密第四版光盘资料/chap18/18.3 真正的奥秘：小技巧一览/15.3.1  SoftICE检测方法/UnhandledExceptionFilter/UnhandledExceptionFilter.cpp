/*--------------------------------------------------------------
   UnhandledExceptionFilter.cpp -- ����UnhandledExceptionFilter���SoftICE
                           (c) www.pediy.com �θ�, 2003
  --------------------------------------------------------------*/

#include <windows.h>

BOOL SoftICELoaded();


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{

	if( SoftICELoaded() )
		MessageBox(NULL,TEXT ("SoftICE is active!"),TEXT ("OK"),MB_ICONEXCLAMATION);
	else
    	MessageBox(NULL,TEXT ("Can't find SoftICE with this method!"),TEXT ("Error"),MB_ICONEXCLAMATION);

	return 0;
}

//////////////////////////////////////////////////////////////////////



BOOL SoftICELoaded()
{

	FARPROC Uaddr ;
	BYTE Mark = 0;
	(FARPROC&) Uaddr =GetProcAddress ( GetModuleHandle("kernel32.dll"),"UnhandledExceptionFilter");
	Mark = *((BYTE*)Uaddr);   // ȡUnhandledExceptionFilter������һ�ֽ�
	if(Mark ==0xCC)           // ����ֽ�ΪCC����SoftICE������
		return TRUE;
	else	
		return FALSE;

}