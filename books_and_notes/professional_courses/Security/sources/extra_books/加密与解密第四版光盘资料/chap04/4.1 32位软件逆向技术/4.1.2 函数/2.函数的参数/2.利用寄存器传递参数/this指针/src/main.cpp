/*******************************************************
*  thisָ��                                                                                                        *
*   (c)  ��ѩѧԺ www.kanxue.com 2000-2018                *
********************************************************/
#include <stdio.h>
class CSum
{
public:
	int Add(int a, int b) //ʵ��Addԭ�;���������ʽ:Add(this,int a,int b)
	{
		return (a + b);
	}
};

void main()
{     
	CSum sum;
	sum.Add(1, 2);
} 
