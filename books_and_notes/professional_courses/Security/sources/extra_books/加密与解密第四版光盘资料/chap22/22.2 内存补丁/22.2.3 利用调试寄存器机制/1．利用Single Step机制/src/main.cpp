/*******************************************************
/*����������ܣ����İ棩��
/* ��22��  ��������
/*  Code by ��  �� 
/*  Thanks Zeal 
/*  
/*(c)  ��ѩѧԺ www.kanxue.com 2000-2018
********************************************************//*******************************************************
/*����������ܣ����İ棩��
/* ��22��  ��������
/*  Code by ��  �� 
/*  Thanks Zeal 
/*  
/*(c)  ��ѩѧԺ www.kanxue.com 2000-2018
********************************************************/

#include <windows.h>

#define BREAK_POINT1     0x040138D          //��Ҫ�жϵĵ�ַ
#define SZFILENAME       ".\\TraceMe_asp.exe"  //Ŀ���ļ���

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{		
	
	STARTUPINFO				si ;
	PROCESS_INFORMATION		pi ;
	ZeroMemory(&si, sizeof(STARTUPINFO)) ;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION)) ;
	si.cb = sizeof(STARTUPINFO) ;
	
	BOOL	WhileDoFlag=TRUE;
	BYTE    ReadBuffer[MAX_PATH]={0};
	
	
	if( !CreateProcess(SZFILENAME, 
        NULL, 
        NULL,  
        NULL,  
        FALSE,   
        DEBUG_PROCESS|DEBUG_ONLY_THIS_PROCESS, 
        NULL,          
        NULL,          
        &si,            
        &pi )        
		) 
	{
        MessageBox(NULL, "CreateProcess Failed.", "ERROR", MB_OK); 
        return FALSE; 
    } 
	
	DEBUG_EVENT		DBEvent ;
	CONTEXT			Regs ;
	DWORD			 dwSSCnt ;
	
	dwSSCnt = 0 ;

	Regs.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;

	//ʹ������Single Stepģʽ������,ÿִ��һ��ָ��ͻ�����Խ��̷���EXCEPTION_SINGLE_STEP
	//��Ҫע����ǣ��յ������Ϣ�������������ó���Single Step��ȥ������Ҫ��������һ��SFλ��
	GetThreadContext(pi.hThread,&Regs);
	Regs.EFlags|=0x100;
	SetThreadContext(pi.hThread,&Regs);

	ResumeThread(pi.hThread);

	
	while (WhileDoFlag) {
		WaitForDebugEvent (&DBEvent, INFINITE);
		switch (DBEvent.dwDebugEventCode)
		{
		case	EXCEPTION_DEBUG_EVENT:

			switch (DBEvent.u.Exception.ExceptionRecord.ExceptionCode)
			{
			case	EXCEPTION_SINGLE_STEP :
				{
					++dwSSCnt ;
					if (dwSSCnt == 1)
					{   
						//���յ���һ��EXCEPTION_SINGLE_STEP�쳣�źţ���ʾ�ж��ڳ���ĵ�һ��ָ�����ڵ�
						//��Dr0���óɳ������ڵ�ַ
						
						GetThreadContext(pi.hThread,&Regs);
						
						Regs.Dr0=Regs.Eax;
						Regs.Dr7=0x101;
						
						SetThreadContext(pi.hThread,&Regs);
						
					}
					else if (dwSSCnt == 2)
					{
						//�ڶ����ж����������õ���ڵ㣬��BREAK_POINT1������Ӳ���ϵ�

						GetThreadContext(pi.hThread, &Regs) ;
						Regs.Dr0 = BREAK_POINT1;
						Regs.Dr7 = 0x101 ;
						SetThreadContext(pi.hThread, &Regs) ;
					}
					else if (dwSSCnt == 3)
					{
						//�������жϣ�����ָ���ĵ�ַ����ȡEBP�Ĵ���ָ����ڴ�����
						GetThreadContext(pi.hThread, &Regs) ;
						Regs.Dr0 = Regs.Dr7 = 0 ;
					
						//WriteProcessMemory(pi.hProcess, (LPVOID)Regs.Eip,&WriteData, 2,NULL);	//�򲹶�
						ReadProcessMemory(pi.hProcess, (LPCVOID)(Regs.Ebp), &ReadBuffer, sizeof(ReadBuffer), NULL) ;
						MessageBox (0, (char *)ReadBuffer, "test", MB_OK);						
						SetThreadContext(pi.hThread, &Regs) ;
					}
					break ;
				}				 
			}
			break ;

		case	EXIT_PROCESS_DEBUG_EVENT :
				WhileDoFlag=FALSE;
				break ;
		}
		
		ContinueDebugEvent(pi.dwProcessId, pi.dwThreadId, DBG_CONTINUE) ;
	} //.end while
	
	CloseHandle(pi.hProcess) ;
	CloseHandle(pi.hThread)  ;
	return TRUE;
}
