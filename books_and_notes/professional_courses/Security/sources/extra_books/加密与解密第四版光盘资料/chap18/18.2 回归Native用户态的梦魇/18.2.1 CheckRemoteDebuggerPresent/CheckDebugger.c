/*-----------------------------------------------------------------------
��18��  �����ټ���
code by forgot 
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

typedef BOOL (WINAPI *CHECK_REMOTE_DEBUGGER_PRESENT)(HANDLE, PBOOL);
BOOL 
CheckDebugger(
    VOID)
{
    HANDLE      hProcess;
    HINSTANCE   hModule;    
    BOOL        bDebuggerPresent = FALSE;
    CHECK_REMOTE_DEBUGGER_PRESENT CheckRemoteDebuggerPresent;
    hModule = GetModuleHandleA("Kernel32");
    CheckRemoteDebuggerPresent = 
        (CHECK_REMOTE_DEBUGGER_PRESENT)GetProcAddress(hModule, "CheckRemoteDebuggerPresent");
    hProcess = GetCurrentProcess();
    return CheckRemoteDebuggerPresent(
                hProcess, 
                &bDebuggerPresent) ? bDebuggerPresent : FALSE;
}
