/*--------------------------------------------------------------
   int41.cpp -- ����int41 ���SoftICE
                           (c) www.pediy.com �θ�, 2003
  --------------------------------------------------------------*/

#include <windows.h>

BOOL SoftICELoaded();
#define MAX_HANDLE_LIST_BUF 0x20000

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
//����int41 ���SoftICE
//////////////////////////////////////////////////////////////////////

BOOL SoftICELoaded()
{
	try {
    _asm {
         mov eax,0x4f          // AX = 004Fh
         int 0x41              // INT 41 
         cmp ax,0x0F386        // �������������AX = F386h 
         jz label
	   }

		return FALSE;
label:
		return TRUE;
	}
	catch (...)
	{
		return FALSE;
	}

}


