#include <windows.h>
#include <winbase.h>
void main()
{
       LoadLibrary("msvcrt.dll");
       __asm {

				mov esp,ebp                 ;��ebp�����ݸ�ֵ��esp
				push ebp                    ;����ebp��esp��4
				mov ebp,esp                 ;��ebp����ֵ������Ϊ�ֲ������Ļ�ָ��
				sub esp, 0Ch			;
				mov eax, 6D6D6F63h			;
				mov dword ptr [ebp-0Ch], eax	;
				mov eax, 2E646E61h			;
				mov dword ptr [ebp-08h], eax	;
				mov eax, 226D6F63h		;
				mov dword ptr [ebp-04h], eax
				xor edx, edx
				mov byte ptr [ebp-01], dl
				push eax
				mov eax, 0x77bf8044
				call eax
       }
}

