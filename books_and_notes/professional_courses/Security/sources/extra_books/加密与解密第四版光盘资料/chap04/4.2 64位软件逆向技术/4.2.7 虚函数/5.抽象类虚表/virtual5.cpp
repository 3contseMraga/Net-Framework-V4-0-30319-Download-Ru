/*--------------------------------------------
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
----------------------------------------------*/

// Example4-5.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"


class IBase {
public:
	IBase() {
		m_nMember = 1;
		printf("IBase()\r\n");
	}
	virtual void fun1() = 0;
	virtual void fun2() = 0;
private:
	int m_nMember;
};

class CDerived :public IBase
{
public:
	CDerived()
	{
		printf("CDerived()\r\n");
	}
	virtual void fun1(){};
	virtual void fun2() {};
};

int _tmain(int argc, _TCHAR* argv[]) {
	IBase *pBase = new CDerived();
	delete pBase;
	return 0;
}
