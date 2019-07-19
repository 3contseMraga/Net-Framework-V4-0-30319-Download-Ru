/*--------------------------------------------------------------
   MeltICE.cpp -- MeltICE�����ͼ��SoftICE
                           (c) www.pediy.com  �θ�, 2003
  --------------------------------------------------------------*/

#include <windows.h>


BOOL IsSoftIceNTLoaded();
BOOL IsSoftIce9xLoaded();
BOOL SoftICELoadedbySIWDEBUG();


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{

	if( IsSoftIce9xLoaded() || SoftICELoadedbySIWDEBUG())
		MessageBox(NULL,TEXT ("SoftICE for Windows 9x is active!"),TEXT ("OK"),MB_ICONEXCLAMATION);
	else if( IsSoftIceNTLoaded() )
		MessageBox(NULL,TEXT ("SoftICE 4.05 for Win2K  is Running!\n"),TEXT ("OK"),MB_ICONEXCLAMATION);
	else
    	MessageBox(NULL,TEXT ("Can't find SoftICE with this method!"),TEXT ("Error"),MB_ICONEXCLAMATION);

	return 0;
}

//////////////////////////////////////////////////////////////////////
//
// ���Windows 9x�ϵ�SoftICE
//////////////////////////////////////////////////////////////////////
BOOL IsSoftIce9xLoaded()
{
	HANDLE hFile;  
 
	hFile = CreateFile( "\\\\.\\SICE",	
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);

	if( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle(hFile);
		return TRUE;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////
//
// ����\.\\SIWDEBUG���Windows 9x�ϵ�SoftICE
//////////////////////////////////////////////////////////////////////

BOOL SoftICELoadedbySIWDEBUG()
{
	
	HANDLE hFile;  
	hFile = CreateFile( "\\\\.\\SIWDEBUG",	
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);

	if( GetLastError()==0x32 )  // ���SoftICE����GetLastError()����32h; ���򷵻�02
		return TRUE;
	else 
		return FALSE;
}

//////////////////////////////////////////////////////////////////////
//
// ���Windows 2000ϵͳ�ϵ�SofICE 4.05
//////////////////////////////////////////////////////////////////////
BOOL IsSoftIceNTLoaded()
{
	HANDLE hFile;  
 
	hFile = CreateFile( "\\\\.\\NTICE",
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);

	if( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle(hFile);
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////