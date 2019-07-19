/*-----------------------------------------------------------------------
��13��  Hook����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/


// CClassHook.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CClassHook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
Ҫ�����������
1.��λ�ȡ���Ա�����ĵ�ַ
2.���ʹ����ͨ�����滻���Ա����
*/
/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;


class base
{
public:
    virtual int Add(int a,int b);
    virtual void g(){cout<<"base::g"<<endl;};
    virtual void h(){cout<<"base::h"<<endl;};
	void novirtual(){cout<<"base::not virtual"<<endl;};
};

int base::Add(int a,int b)
{
	printf("base::Add\n");
	return a + b ;
}

class DetourClass
{
public:
    virtual int DetourFun(int a,int b);
};

class TrampolineClass
{
public:
	virtual int TrampolineFun(int a,int b){printf("TrampolineClass\n");return 0 ;};//ԭ���뱻Hook������ͬ
};

DetourClass Detour;
TrampolineClass Trampoline;

int DetourClass::DetourFun(int a,int b)
{
	//TrampolineFun();  //�����������ʵ������base,���Ե���TrampolineFun�����õڶ����麯�����൱�ڵ���pbase->g()
	TrampolineClass *pTrampoline = new TrampolineClass;
	int result = pTrampoline->TrampolineFun(a,b);
	printf("DetourClass:: OriginalFun returned %d\n",result);
	result += 10 ;
	delete pTrampoline;
	return result;
}



typedef void (*pfun)();
LPVOID GetClassVirtualFnAddress(LPVOID pthis,int Index);
VOID HookClassMemberByAnotherClassMember();


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		// TODO: code your application's behavior here.
		HookClassMemberByAnotherClassMember();
		getchar();
	}

	return nRetCode;
}


void HookClassMemberByAnotherClassMember()
{
	base b;
	base *pbase=&b;
	
	DWORD dwOLD;
	MEMORY_BASIC_INFORMATION  mbi;

	printf("pbase = 0x%X\n",pbase);
	
	ULONG_PTR *vfTableToHook = (ULONG_PTR*)*(ULONG_PTR*)pbase;
	printf("vfTable = 0x%x\n",vfTableToHook);

	ULONG_PTR *vfTableTrampoline = (ULONG_PTR*)*(ULONG_PTR*)&Trampoline;
	
	//�Ƚ�ԭ�����ĵ�ַ���浽��ǰ��ı��У���Ϊ����ԭ���������
	VirtualQuery(vfTableTrampoline,&mbi,sizeof(mbi));
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_EXECUTE_READWRITE,&dwOLD);
	//����ԭʼ����
	//ԭ����λ�ڵڼ���������ǵڼ��������뱣֤λ��һ��
	vfTableTrampoline[0] = (ULONG_PTR)GetClassVirtualFnAddress(pbase,0);
	printf("Base::Add()  %p\n",vfTableTrampoline[0]);
	TrampolineClass *p = &Trampoline;
	//�ָ��ڴ�ҳ������
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,dwOLD,0);
	//�޸��ڴ�ҳ������
	
	VirtualQuery(vfTableToHook,&mbi,sizeof(mbi));
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_EXECUTE_READWRITE,&dwOLD);
	//����ԭʼ����
	vfTableToHook[0] = (ULONG_PTR)GetClassVirtualFnAddress(&Detour,0);
	printf("Detour::Add()  %p\n",vfTableToHook[0]);
	//�ָ��ڴ�ҳ������
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,dwOLD,0);
	int result = pbase->Add(1,2);    //���õ�3���麯����ʵ�ʵ��õ���HookClass::DetourFun()
	printf("result = %d  \nafter call member fun.\n",result);
}

//����������Ա����ָ��
LPVOID GetClassVirtualFnAddress(LPVOID pthis,int Index) 
{
	ULONG_PTR *vfTable = (ULONG_PTR*)*(ULONG_PTR*)pthis;
	return (LPVOID)vfTable[Index];
}