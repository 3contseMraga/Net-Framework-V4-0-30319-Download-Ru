/*-----------------------------------------------------------------------
��13��  Hook����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/


#include <windows.h>
#include <stdio.h>

#define STATUS_FAILED (0)

LONG WINAPI TargetFun(
	IN LONG ParameterIn,//����Ĳ���
	OUT PVOID ParameterOut
	);

typedef LONG 
(WINAPI *PFN_TargetFun)(
	IN LONG ParameterIn,//����Ĳ���
	OUT PVOID ParameterOut
	);

BOOL CheckParameters(LONG ParameterIn);
BOOL CheckResult(PVOID ParameterOut);
VOID CleanupResult(PVOID ParameterOut);

PFN_TargetFun pOriginalFun = NULL ;

int main(int argc, char* argv[])
{
	return 0 ;
}

LONG WINAPI TargetFun(
	IN LONG ParameterIn,//����Ĳ���
	OUT PVOID ParameterOut
	)
{
	return 0 ;
}

BOOL CheckParameters(LONG ParameterIn)
{
	return TRUE;
}

BOOL CheckResult(PVOID ParameterOut)
{
	return TRUE;
}

VOID CleanupResult(PVOID ParameterOut)
{
	
}

LONG WINAPI DetourFun(
	IN LONG ParameterIn,	//����Ĳ���
	OUT PVOID ParameterOut	//�����ķ��ؽ��
	)
{
	LONG result = STATUS_FAILED ;
	if (!CheckParameters(ParameterIn))
	{
		//������鲻ͨ�����ܾ��˴ε��ã�ֱ�ӷ���ʧ��
		return STATUS_FAILED;
	}
	else
	{
		//���ͨ��������ԭ����
		result = pOriginalFun(ParameterIn,ParameterOut);
		if (result == STATUS_FAILED)
		{
			//���ò��ɹ���������ᣬֱ�ӷ���ԭ���
			return result;
		}
		else
		{
			//���óɹ����Խ�����м��
			if (!CheckResult(ParameterOut))
			{
				//�����ͨ����������ô�����Ӱ�죬������ʧ��
				//��������Ҫ���ͷ��ڴ桢�رվ����
				CleanupResult(ParameterOut);
				return STATUS_FAILED;
			}
			else
			{
				//������ͨ�������в�����ԭ���
				return result;
			}
		}
	}
}