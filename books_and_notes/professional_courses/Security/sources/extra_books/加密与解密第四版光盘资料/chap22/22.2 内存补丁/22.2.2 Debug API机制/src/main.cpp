/*-----------------------------------------------------------------------
��22��  ��������
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

#include <windows.h>

#define BREAK_POINT1     0x040138D            //��Ҫ�жϵĵ�ַ
#define SZFILENAME       ".\\TraceMe.exe"  //Ŀ���ļ���

	
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
	BYTE    dwINT3code[1]={0xCC};
	BYTE    dwOldbyte[1]={0};

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
	DWORD			dwState,Oldpp;


	Regs.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;
	
	while (WhileDoFlag) {
		WaitForDebugEvent (&DBEvent, INFINITE);
		dwState = DBG_EXCEPTION_NOT_HANDLED ;
		switch (DBEvent.dwDebugEventCode)
		{
			case CREATE_PROCESS_DEBUG_EVENT:
				//������̿�ʼ���򽫶ϵ��ַ�Ĵ����ΪINT3�ж�,ͬʱ����ԭ������
				ReadProcessMemory(pi.hProcess, (LPCVOID)(BREAK_POINT1), &dwOldbyte, sizeof(dwOldbyte), NULL) ;
				VirtualProtectEx(pi.hProcess, (LPVOID)BREAK_POINT1, 1, PAGE_EXECUTE_READWRITE, &Oldpp);
				WriteProcessMemory(pi.hProcess, (LPVOID)BREAK_POINT1,&dwINT3code, 1,NULL);	//�򲹶�
				dwState = DBG_CONTINUE ;
				break;			
				
			case	EXIT_PROCESS_DEBUG_EVENT :
				WhileDoFlag=FALSE;
				break ;
			
			case	EXCEPTION_DEBUG_EVENT:
				switch (DBEvent.u.Exception.ExceptionRecord.ExceptionCode)
				{
					case	EXCEPTION_BREAKPOINT:
					{
						GetThreadContext(pi.hThread, &Regs) ;
						if(Regs.Eip==BREAK_POINT1+1){
							//�жϴ����쳣�¼����ָ�ԭ�����룬����������
							Regs.Eip--;
							WriteProcessMemory(pi.hProcess, (LPVOID)BREAK_POINT1,&dwOldbyte, 1,NULL);
							ReadProcessMemory(pi.hProcess, (LPCVOID)(Regs.Ebp), &ReadBuffer, sizeof(ReadBuffer), NULL) ;
							MessageBox (0, (char *)ReadBuffer, "�ڴ�ע�����ʾ", MB_OK);						
							SetThreadContext(pi.hThread, &Regs) ;
						}
						dwState = DBG_CONTINUE ;
						break;
					}
				}
				break;
		}
		
		ContinueDebugEvent(pi.dwProcessId, pi.dwThreadId, dwState) ;
	} //.end while
	
	CloseHandle(pi.hProcess) ;
	CloseHandle(pi.hThread)  ;
	return TRUE;
}
