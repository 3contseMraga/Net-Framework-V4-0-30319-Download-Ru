/*-----------------------------------------------------------------------
��18��  �����ټ���
code by forgot
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

ULONG GetPebBase(ULONG ProcessId)
{
    HANDLE hProcess = NULL;
    PROCESS_BASIC_INFORMATION pbi = {0};
    ULONG peb = 0;        
    ULONG cnt = 0;
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessId);
    if (hProcess != NULL) {
        if (NtQueryInformationProcess(
                hProcess,
                ProcessBasicInformation,
                &pbi,
                sizeof(PROCESS_BASIC_INFORMATION),
                &cnt) == 0) {
            PebBase = (ULONG)pbi.PebBaseAddress;
        }            
        CloseHandle(hProcess);
    }
}
