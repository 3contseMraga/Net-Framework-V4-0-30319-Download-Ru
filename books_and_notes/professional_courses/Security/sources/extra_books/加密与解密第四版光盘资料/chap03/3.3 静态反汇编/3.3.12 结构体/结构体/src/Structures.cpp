/*******************************************************
*����:Structures.cpp                                                                                      *
*��;:IDA�ṹ�巴���                                                                        *
*   ��ѩ�����ȫ��վ                                                                              *   
*    www.pediy.com, kanxue 2002.8                                                                 *
********************************************************/
#include <stdio.h> 

struct student
	{int id;
	char name[20];
	int age;
	};
struct student stu[2]={{01,"Mary",14},{02,"Angela",15}};
int main(void)
{
	struct student *p;
	for(p=stu;p<stu+2;p++)
		printf("%5d  %-20s%4d\n",p->id,p->name,p->age);
	return 0;
}