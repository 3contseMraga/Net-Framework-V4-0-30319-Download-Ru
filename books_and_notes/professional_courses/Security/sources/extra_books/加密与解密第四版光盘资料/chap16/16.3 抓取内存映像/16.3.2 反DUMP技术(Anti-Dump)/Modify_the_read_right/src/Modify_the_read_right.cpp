/*------------------------------------------------------------------------
  Save as Modify_the_read_right.cpp
                         (c) Lenus Margin 2005 10.15
-------------------------------------------------------------------------*/

#include <windows.h>
#include <iostream.h>
#include <stdio.h>

void main()
{
  DWORD dwOldProtect;  
  HMODULE ImageBase=GetModuleHandle(NULL);
  VirtualProtect(ImageBase,1000,PAGE_NOACCESS,&dwOldProtect);//��PEͷ��Ϊ���ɷ��ʵĵ�����  
  cout<<"Code by Lenus."<<endl;
  cout<<"The PE HEAD read right change to no_access!!"<<endl;
  cout<<"U can try to dump at here!"<<endl;
  getchar();
}