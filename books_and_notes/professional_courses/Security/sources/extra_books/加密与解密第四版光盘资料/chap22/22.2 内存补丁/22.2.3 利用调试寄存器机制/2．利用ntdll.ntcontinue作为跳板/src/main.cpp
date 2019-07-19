/*-----------------------------------------------------------------------
��22��  ��������
����������ܣ����İ棩��
Code by ��  ��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

#include <windows.h>

#define BREAK_ENTRYPOINT 0x401000          //������ڵ�
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
	DWORD			dwState, dwBpCnt, dwSSCnt, dwAddrProc ;
	
	dwBpCnt = dwSSCnt = 0 ;

	Regs.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;
	
	while (WhileDoFlag) {
		WaitForDebugEvent (&DBEvent, INFINITE);
		dwState = DBG_EXCEPTION_NOT_HANDLED ;
		switch (DBEvent.dwDebugEventCode)
		{
		case	EXCEPTION_DEBUG_EVENT:
			switch (DBEvent.u.Exception.ExceptionRecord.ExceptionCode)
			{
			case	EXCEPTION_BREAKPOINT:
				{
					++dwBpCnt ;
					if (dwBpCnt == 1)
					{
						//ͨ��dwBPCnt�������жϳ����յ����ǵ�һ��������Ϣ��
						//�õ�ntdll.ntContinue�ĵ�ַ����������Dr0 ��ֵ���ڸõ�ַ
						GetThreadContext(pi.hThread, &Regs) ;
						Regs.Dr0 = (DWORD)(GetProcAddress(GetModuleHandle("ntdll.dll"), "NtContinue") );
						Regs.Dr7 = 0x101 ;
						SetThreadContext(pi.hThread, &Regs) ;
						dwState = DBG_CONTINUE ;
					}
					break ;
				}
			case	EXCEPTION_SINGLE_STEP :
				{
					++dwSSCnt ;
					if (dwSSCnt == 1)
					{
						//���յ���һ��EXCEPTION_SINGLE_STEP�쳣�źţ���ʾ�ж���ntdll.ntContinue����
						//��Dr0���óɳ������ڵ�ַ
						
						GetThreadContext(pi.hThread, &Regs) ;
						Regs.Dr0 = Regs.Dr7 = 0 ;
						SetThreadContext(pi.hThread, &Regs) ;
						
						ReadProcessMemory(pi.hProcess, (LPCVOID)(Regs.Esp+4), &dwAddrProc, sizeof(DWORD), NULL) ;
						ReadProcessMemory(pi.hProcess, (LPCVOID)dwAddrProc, &Regs, sizeof(CONTEXT), NULL) ;
						
						Regs.Dr0 = BREAK_ENTRYPOINT ;
						Regs.Dr7 = 0x101 ;
						
						WriteProcessMemory(pi.hProcess, (LPVOID)dwAddrProc, &Regs, sizeof(CONTEXT), NULL) ;
						
						dwState = DBG_CONTINUE ;
					}
					else if (dwSSCnt == 2)
					{   //�ڶ����ж����������õ���ڵ㣬��BREAK_POINT1������Ӳ���ϵ�
						
						GetThreadContext(pi.hThread, &Regs) ;
						Regs.Dr0 = BREAK_POINT1;
						Regs.Dr7 = 0x101 ;
						SetThreadContext(pi.hThread, &Regs) ;
						dwState = DBG_CONTINUE ;
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
						dwState = DBG_CONTINUE ;
					}
					break ;
				}				 
			}
			break ;

		case	EXIT_PROCESS_DEBUG_EVENT :
				WhileDoFlag=FALSE;
				break ;
		}
		
		ContinueDebugEvent(pi.dwProcessId, pi.dwThreadId, dwState) ;
	} //.end while
	
	CloseHandle(pi.hProcess) ;
	CloseHandle(pi.hThread)  ;
	return TRUE;
}
