/*--------------------------------------------------------------
   int1.cpp -- ����int 1���SoftICE
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
// �˷���Windows 2000/XPϵͳ��Ч��ʵ�ʲ���ʱ������������жϲ���ϵͳ�Ĵ���
//////////////////////////////////////////////////////////////////////

BOOL SoftICELoaded()
{
	int mark=0;
	_asm {
		

			push offset handler
			push dword ptr fs:[0]        // ����SEH��
			mov  dword ptr fs:[0],esp
			xor  eax,eax
			int 1              // ���SICE������,�Ҵ���3��������,�򲻷����쳣,�������쳣
			inc  eax
			inc  eax
			pop  dword ptr fs:[0] 
			add esp,4
			or   eax,eax
			jz   found
			cmp mark, 0
            jnz   found
			jmp  Nofound

	handler:   	// �쳣�ص�������		
	    	mov ebx,[esp+0ch]
			add dword ptr [ebx+0b8h],02h  // CONTEXT.EIP+2
		    mov ebx,[esp+4] 
			cmp [ebx], 80000004h  // 0C0000005����,80000004������
			jz Table
			inc mark          
     Table:			
			xor eax,eax
			ret                           //��ʾ�Ѿ��޸��������ڵ�CONTEXT.EIP��ִ��
	
			}
Nofound:
		return FALSE;	
		
found:
		return TRUE;

}