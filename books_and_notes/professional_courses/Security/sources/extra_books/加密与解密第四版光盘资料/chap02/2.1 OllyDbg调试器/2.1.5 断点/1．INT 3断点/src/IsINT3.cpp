/*-----------------------------------------------------------------------
IsINT3.cpp -- ���CC�ϵ�

����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
----------------------------------------------

#include <windows.h>

BOOL IsCC();


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{

	if( IsCC() )
		MessageBox(NULL,TEXT ("Found Debug!"),TEXT ("OK"),MB_ICONEXCLAMATION);
	else
    	MessageBox(NULL,TEXT ("No Debug!"),TEXT ("Error"),MB_ICONEXCLAMATION);

	return 0;
}

//////////////////////////////////////////////////////////////////////



BOOL IsCC()
{
	FARPROC Uaddr ;
	BYTE Mark = 0;
	(FARPROC&) Uaddr =GetProcAddress ( LoadLibrary("user32.dll"),"MessageBoxA");
	Mark = *((BYTE*)Uaddr);   // ȡMessageBoxA������һ�ֽ�
	if(Mark ==0xCC)           // ����ֽ�ΪCC������ΪMessageBoxA�������¶�
		return TRUE;
	else	
		return FALSE;

}