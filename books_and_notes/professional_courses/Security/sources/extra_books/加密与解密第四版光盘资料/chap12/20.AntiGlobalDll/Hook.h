#include "StdAfx.h"

//�������½ṹ������һ��InlineHook����Ҫ����Ϣ
typedef struct _HOOK_DATA{
	char szApiName[128];	//��Hook��API����
	char szModuleName[64];	//��Hook��API����ģ�������
	int  HookCodeLen;		//Hook����
	BYTE oldEntry[16];		//����Hookλ�õ�ԭʼָ��
	BYTE newEntry[16];		//����Ҫд��Hookλ�õ���ָ��
	BYTE HotPatchCode[8];
	ULONG_PTR HookPoint;		//��HOOK��λ��
	ULONG_PTR JmpBackAddr;		//������ԭ�����е�λ��
	ULONG_PTR pfnTrampolineFun;	//����ԭʼ������ͨ��
	ULONG_PTR pfnDetourFun;		//HOOK���˺���
}HOOK_DATA,*PHOOK_DATA;

LPVOID GetAddress(char *,char *);
BOOL InstallCodeHook(PHOOK_DATA pHookData);
BOOL UninstallCodeHook(PHOOK_DATA pHookData);