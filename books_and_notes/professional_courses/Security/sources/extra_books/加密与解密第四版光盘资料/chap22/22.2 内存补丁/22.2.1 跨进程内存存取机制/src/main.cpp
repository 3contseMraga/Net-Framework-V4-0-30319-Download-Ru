/*-----------------------------------------------------------------------
��22��  ��������
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

/*
004011F5   74 37         je      short 0040122E
���ĳɣ�
004011F5      90         nop
004011F6      90         nop
*/
#define PATCH_ADDRESS   0x4011F5				//Ŀ�����Ҫ�����ĵ�ַ
#define PATCH_SIZE      2					    //Ŀ�����Ҫ�������ֽ���
#define SZFILENAME       ".\\TraceMe_asp.exe"   //Ŀ���ļ���

#define BREAK_ENTRYPOINT 0x401000
#define BREAK_POINT1     0x40138F
#include <windows.h>

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{		
	STARTUPINFOA     	si;
	PROCESS_INFORMATION pi;
	
	BYTE      ReadBuffer[MAX_PATH]={0};
	BOOL      bContinueRun=TRUE;
	DWORD	  Oldpp;
	
	BYTE      TarGetData[]={0x74,0x37}; 			//����ǰ�Ĵ���
	BYTE      WriteData[]={0x90,0x90}; 				//������Ĵ���
	
	
	
	ZeroMemory(&si, sizeof(STARTUPINFO)) ;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION)) ;

    if( !CreateProcess(SZFILENAME, 
        NULL, 
        NULL,  
        NULL,  
        FALSE,   
        CREATE_SUSPENDED, 
        NULL,          
        NULL,          
        &si,            
        &pi )        
		) 
	{
        MessageBox(NULL, "CreateProcess Failed.", "ERROR", MB_OK); 
        return FALSE; 
    } 
	
	
	while (bContinueRun) {
		
		ResumeThread(pi.hThread); 
		Sleep(10);
		SuspendThread(pi.hThread);
		ReadProcessMemory(pi.hProcess, (LPVOID)PATCH_ADDRESS, &ReadBuffer, PATCH_SIZE, NULL);
		//�ж��ǲ�����ȫ���������
		if( !memcmp(TarGetData,ReadBuffer, PATCH_SIZE) ){	
			VirtualProtectEx(pi.hProcess, (LPVOID)PATCH_ADDRESS, PATCH_SIZE, PAGE_EXECUTE_READWRITE, &Oldpp);
			WriteProcessMemory(pi.hProcess, (LPVOID)PATCH_ADDRESS,&WriteData, PATCH_SIZE,NULL);
			ResumeThread(pi.hThread); 
			bContinueRun=FALSE;
		}		
	}
    CloseHandle(pi.hProcess); 
    CloseHandle(pi.hThread); 

    return 0; 	
}
