/*-----------------------------------------------------------------------
��13��  Hook����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/


// HookPromt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

void PrintChar(char *pch);

int main(int argc, char* argv[])
{
	char ch = 'A';
	char ch2 = 'B';
	char *pChar ;
	
	//��������
	pChar = &ch;
	PrintChar(pChar);
	//Address Hook,�޸Ĵ��ݵĵ�ַ
	pChar = &ch2;
	PrintChar(&ch2);
	//�ָ�ԭֵ
	pChar = &ch;
	//Inline  Hook,��ַ����,�޸Ĵ��ݵ�����
	*pChar = 'B';
	PrintChar(pChar);
	return 0 ;

}

void PrintChar(char *pch)
{
	printf("Address = 0x%x Char = %c\n",pch,*pch);
}