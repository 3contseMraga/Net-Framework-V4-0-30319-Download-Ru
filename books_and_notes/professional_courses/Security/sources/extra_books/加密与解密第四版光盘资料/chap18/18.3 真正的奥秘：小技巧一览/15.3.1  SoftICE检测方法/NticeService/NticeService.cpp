/*--------------------------------------------------------------
   NticeService.cpp -- ����NticeService���SoftICE
                           (c) www.pediy.com  �θ�, 2003
  --------------------------------------------------------------*/

#include <windows.h>
#include <wchar.h>

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


BOOL SoftICELoaded()
{

	SERVICE_STATUS ssStatus; 
	// �򿪷�����ƹ�����
	SC_HANDLE shServiceManager = OpenSCManager( NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);
	if(shServiceManager==NULL) 
		return FALSE;
	// ��NTice����
	SC_HANDLE shMyService =OpenService(shServiceManager,"NTice",SERVICE_ALL_ACCESS);

	if ( (QueryServiceStatus( shMyService, &ssStatus)) == 0 ) //��ѯ�����ص�ǰService��״̬
	{
		CloseServiceHandle(shMyService);
		return FALSE;
	}

	if (ssStatus.dwCurrentState == SERVICE_RUNNING ) //NTice�����������У�
	{
		CloseServiceHandle(shMyService);		
			return TRUE;
	}

	return FALSE;

}

