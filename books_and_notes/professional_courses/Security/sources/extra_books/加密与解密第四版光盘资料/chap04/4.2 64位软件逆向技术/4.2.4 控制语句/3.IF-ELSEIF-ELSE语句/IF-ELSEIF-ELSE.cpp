/*--------------------------------------------
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
----------------------------------------------*/

// if-elseif-else.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
int _tmain(int argc, _TCHAR* argv[]) {
	if (argc > 2)
		printf("argc > 2\r\n");
	else if (argc == 2)
		printf("argc == 2\r\n");
	else
		printf("argc <= 1\r\n");
	return 0;
}
