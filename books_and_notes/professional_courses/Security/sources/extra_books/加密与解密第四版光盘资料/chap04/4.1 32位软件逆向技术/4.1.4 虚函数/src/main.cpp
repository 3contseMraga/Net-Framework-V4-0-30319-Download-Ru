/*******************************************************
����������ܡ����İ�����ʵ��
4.1.4 �麯��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
********************************************************/

#include <stdio.h>
class CSum
{
public:
	virtual int Add(int a, int b) 
	{
		return (a + b);
	}

	virtual	int	Sub(int a, int b )
	{
		return (a - b);
	}

};

void main()
{   
	CSum*	pCSum = new CSum ;  

	pCSum->Add(1,2);
	pCSum->Sub(1,2);


} 
