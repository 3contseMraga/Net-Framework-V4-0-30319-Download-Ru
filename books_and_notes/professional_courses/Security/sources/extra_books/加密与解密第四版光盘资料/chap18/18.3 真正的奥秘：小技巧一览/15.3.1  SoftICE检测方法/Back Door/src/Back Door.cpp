/*--------------------------------------------------------------
   Back Door.cpp -- ����SoftICE����ָ����SoftICE
                           www.pediy.com (c) �θ�, 2003
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
//
// ����SoftICE����ָ����SoftICE����SoftICE�����ڽ�����ϵ��쳣��
//////////////////////////////////////////////////////////////////////

BOOL SoftICELoaded()
{
	
	try
	{
		_asm {
			mov si, 'FG'  // ħ��ֵ"FG"
			mov di, 'JM'  // ħ��ֵ"JM"		
			int 3
			nop
			jmp		label
			}
label:
		return TRUE;
	}
    catch (...)
	{
		return FALSE;
	}

}

