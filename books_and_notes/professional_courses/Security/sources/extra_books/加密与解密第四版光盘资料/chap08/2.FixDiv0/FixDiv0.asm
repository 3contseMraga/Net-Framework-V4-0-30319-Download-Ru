;******************************************
;coded by Hume,2K+
;******************************************
;����2.��ʾ�߳��쳣����
;******************************************
include ../asminc.h

	.DATA
szTit   db "SEH����-�߳��쳣����Hume,2k+",0
mesSUC  db "WE SUCEED IN FIX DIV0 ERROR.",0	
	.DATA?
hInstance	dd ?
;;-----------------------------------------
	.CODE
SEHandler      proc C uses ebx esi edi pExcept,pFrame,pContext,pDispatch

        Assume  esi:ptr EXCEPTION_RECORD
        Assume  edi:ptr CONTEXT

        mov     esi,pExcept
        mov     edi,pContext
        test    [esi].ExceptionFlags,3
        jne     _continue_search
        cmp     [esi].ExceptionCode,STATUS_INTEGER_DIVIDE_BY_ZERO         ;�ǳ�0��
        jne     _continue_search

        mov     [edi].regEcx,10                         ;����������Ϊ��0ֵ��������ִ��
                                                        ;��ο��Եõ���ȷ�����10

        mov     eax,ExceptionContinueExecution          ;�޸���ϣ�����ִ��
        ret
_continue_search:
        mov     eax,ExceptionContinueSearch             ;�����쳣���޷�������������seh�ص������б�
        ret
SEHandler      endp
_StArT:
        assume fs:nothing
      
        push    offset SEHandler
        push    fs:[0]
        mov     fs:[0],esp                      ;����EXCEPTION_REGISTRATION_RECORD�ṹ����
                                                ;TIBƫ��0��Ϊ�ýṹ��ַ
        
        xor     ecx,ecx                         ;ECX=0
        mov     eax,100                         ;EAX=100
        xor     edx,edx                         ;EDX=0

        div     ecx                             ;������0��
        invoke	MessageBox,0,addr mesSUC,addr szTit,0

        pop     fs:[0]                          ;�ָ�ԭ�쳣�ص�����
        add     esp,4                           ;ƽ���ջ

	invoke ExitProcess,0
END	_StArT