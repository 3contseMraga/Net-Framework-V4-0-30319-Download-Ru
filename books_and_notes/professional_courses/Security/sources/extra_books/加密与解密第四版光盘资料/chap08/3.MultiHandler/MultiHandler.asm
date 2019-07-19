;******************************************
;coded by Hume,2K+
;����������ܣ����İ棩��
;(c)  ��ѩѧԺ www.kanxue.com 2000-2018
;******************************************
;����3.��ʾ�߳��쳣�����Ƕ��
;******************************************
include ../asminc.h
;~~~~~~~~~~~~~~~~MACROs
;ע��ص�����
InstSEHframe    MACRO CallbackFucAddr
        push    offset CallbackFucAddr
        push    fs:[0]
        mov     fs:[0],esp
ENDM
;ж�ػص�����
UnInstSEHframe    MACRO
        pop     fs:[0]
        add     esp,4
ENDM
;�ú���ظ����룬��Ӧ��handler���жϲ���
SEHhandlerProcessOrNot MACRO ExceptType,Exit2SearchAddr
        Assume esi:ptr EXCEPTION_RECORD
        Assume edi:ptr CONTEXT

        
        mov     esi,[pExcept]
        mov     edi,pContext
        test    [esi].ExceptionFlags,7
        jnz     Exit2SearchAddr
        cmp     [esi].ExceptionCode,ExceptType
        jnz     Exit2SearchAddr
        ;;below should follow the real processing codes
ENDM 
;~~~~~~~~~~~~~~~~~~~~~~~
	.DATA
szTit           db "SEH����-Per_ThreadǶ�ף�Hume,2k+",0
FixDivSuc       db "Fix Div0 Error Suc!",0
FixWriSuc       db "Fix Write Acess Error Suc!",0
FixInt3Suc      db "Fix Int3 BreakPoint Suc!",0
DATABUF         dd 0
;;-----------------------------------------
	.CODE
;��0���쳣������
Div_handler0    proc  C  uses ebx esi edi pExcept,pFrame,pContext,pDispatch
        PUSHAD
        SEHhandlerProcessOrNot  STATUS_INTEGER_DIVIDE_BY_ZERO,@ContiSearch      ;�Ƿ���������0��
        mov     [edi].regEcx,10                 ;����������

        POPAD
        mov     eax,ExceptionContinueExecution  ;���ؼ���ִ��       
        ret
@ContiSearch:
        POPAD
        mov     eax,ExceptionContinueSearch         
        ret
Div_handler0    endp    

;��д��ͻ�ڴ��쳣������
Wri_handler1    proc  C  uses ebx esi edi pExcept,pFrame,pContext,pDispatch
        PUSHAD
        SEHhandlerProcessOrNot  STATUS_ACCESS_VIOLATION,@ContiSearch      ;�Ƿ��Ƕ�д�ڴ��ͻ

        mov     [edi].regEip,offset safePlace1  ;�ı䷵�غ�ָ���ִ�е�ַ
        ;mov     [edi].regEdx,offset DATABUF    ;��д��ַת��Ϊ��Чֵ
        POPAD
        mov     eax,ExceptionContinueExecution
        ret
@ContiSearch:
        POPAD
        mov     eax,ExceptionContinueSearch 
        ret        

Wri_handler1    endp    

;�ϵ��ж��쳣������
Int3_handler2   proc C   uses ebx esi edi pExcept,pFrame,pContext,pDispatch
        PUSHAD
        SEHhandlerProcessOrNot  STATUS_BREAKPOINT,@ContiSearch      ;�Ƿ��Ƕϵ�
        INC     [edi].regEip                            ;�������غ�ָ���ִ�е�ַ��Խ���ϵ����ִ��  
                                                        ;ע����9X��INT3�쳣������ָ���ַΪ
        POPAD
        mov     eax,ExceptionContinueExecution
        ret
@ContiSearch:
        POPAD
        mov     eax,ExceptionContinueSearch 
        ret       
Int3_handler2   endp    
;mesAddrӦ����ָ������ʾ��Ϣ�ĵ�ַ
MsgBox          proc   mesAddr
        invoke	MessageBox,0,mesAddr,offset szTit,MB_ICONINFORMATION
        ret
MsgBox          endp
;-----------------------------------------

_StArT:
        Assume fs:nothing
        invoke	SetErrorMode,0
        InstSEHframe    Div_handler0 
        InstSEHframe    Wri_handler1
        InstSEHframe    Int3_handler2
        
        mov     eax,100
        cdq                     ;eax=100 edx=0
        xor     ecx,ecx         ;ecx��0
        div     ecx             ;��0�쳣��
        invoke	MsgBox,offset FixDivSuc       ;��������0��ɹ�

        xor     edx,edx
        mov     [edx],eax       ;���ַ0��д�룬����д�쳣��

safePlace1:
        invoke	MsgBox,offset FixWriSuc       ;�������д�����ڴ�ɹ�

        int     3
        nop
        invoke	MsgBox,offset FixInt3Suc      ;�������ϵ�int 3�ɹ�
        
       
        invoke	MessageBox,0,CTEXT("Test Illegal INSTR without Handler or Not(Y/N)?"),offset szTit,MB_YESNO
        cmp     eax,IDYES
        jnz     no_test
        db      0Fh,17h      ;Ϊ�Ƿ�ָ�����

        invoke	MsgBox,CTEXT("here,will Exit")
no_test:
        UnInstSEHframe          ;ж�����еĻص�����
        UnInstSEHframe
        UnInstSEHframe
	invoke ExitProcess,0
END	_StArT