/*--------------------------------------------
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
----------------------------------------------*/

// Example4-1.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"


class CVirtual {
public:
	CVirtual() {
		m_nMember1 = 1;
		m_nMember2 = 2;
		printf("CVirtual()\r\n");
	}
	virtual ~CVirtual() {
		printf("~CVirtual()\r\n");
	}
	virtual void fun1() {
		printf("fun1()\r\n");
	}
	virtual void fun2() {
		printf("fun2()\r\n");
	}
private:
	int m_nMember1;
	int m_nMember2;
};

int main(int argc, char* argv[]) {
	CVirtual object;
	object.fun1();
	object.fun2();
	return 0;
}


