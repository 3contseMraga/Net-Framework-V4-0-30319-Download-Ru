/*-----------------------------------------------------------------------
��18��  �����ټ��� 
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

BOOL
CheckRemoteDebuggerPresent(
    HANDLE	hProcess, 
    PBOOL		pbDebuggerPresent)
{
    DWORD rv;
    if (hProcess & pbDebuggerPresent) {
        rv = NtQueryInformationProcess(hProcess, 7, &hProcess, 4, 0);
        if (rv < 0) {
            BaseSetLastNTError(rv); //��ʵ���������������IDA��kernel32.dll�õ���
            return FALSE;
        } else {
            pbDebuggerPresent = hProcess;
            return TRUE;
        }
    } else {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
}
