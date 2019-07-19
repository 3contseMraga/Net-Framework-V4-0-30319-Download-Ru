;******************************************
;coded by Hume,2K+
;����������ܣ����İ棩��
;(c)  ��ѩѧԺ www.kanxue.com 2000-2018
;******************************************
;����4.��ʾ�쳣����Ķ�ջչ��
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
        Assume esi:ptr EXCEPTION_RECORD,edi:ptr CONTEXT        
        mov     esi,[pExcept]
        mov     edi,pContext
        test    [esi].ExceptionFlags,1
        jnz     Exit2SearchAddr
        test    [esi].ExceptionFlags,2
        jnz     @_Unwind
        cmp     [esi].ExceptionCode,ExceptType
        jnz     Exit2SearchAddr
        ;;below should follow the real processing codes
ENDM 
;~~~~~~~~~~~~~~~~~~~~~~~
	.DATA
szTit           db "SEH����-Ƕ�׼�չ����Hume,2k+",0
FixDivSuc       db "Fix Div0 Error Suc!",0
FixWriSuc       db "Fix Write Acess Error Suc!",0
FixInt3Suc      db "Fix Int3 BreakPoint Suc!",0
DATABUF         dd 0
        .DATA?
OLDHANDLER      dd ?
;;-----------------------------------------
	.CODE
;��0���쳣������
;-----------------------------------------

;mesAddrӦ����ָ������ʾ��Ϣ�ĵ�ַ
MsgBox          proc   mesAddr
        invoke	MessageBox,0,mesAddr,offset szTit,MB_ICONINFORMATION
        ret
MsgBox          endp
;-----------------------------------------

Div_handler0    proc  C  pExcept,pFrame,pContext,pDispatch
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
@_Unwind:
        invoke	MsgBox,CTEXT("Div Handler unwinds")
        jmp     @ContiSearch
Div_handler0    endp    

;��д��ͻ�ڴ��쳣������
Wri_handler1    proc  C  pExcept,pFrame,pContext,pDispatch
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
@_Unwind:
        invoke	MsgBox,CTEXT("Write Acess Handler unwinds")
        jmp     @ContiSearch

Wri_handler1    endp    

;�ϵ��ж��쳣������
Int3_handler2   proc C   pExcept,pFrame,pContext,pDispatch
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
@_Unwind:
        invoke	MsgBox,CTEXT("Int3 Handler unwinds")
        jmp     @ContiSearch
        
Int3_handler2   endp    

SELF_UNWIND     proc C   pExcept,pFrame,pContext,pDispatch
        PUSHAD 
        SEHhandlerProcessOrNot  STATUS_ILLEGAL_INSTRUCTION,@ContiSearch      ;�Ƿ�����Чָ�����
        invoke	MessageBox,0,CTEXT("Unwind by SELF(Y) or Let SYSTEM do(N)?"),addr szTit,MB_YESNO
        cmp     eax,IDYES
        jnz     @ContiSearch
        mov     [edi].regEip,offset unwind_return

        invoke	RtlUnwind,[pFrame],0,[pExcept],0        
        POPAD
        mov     eax,ExceptionContinueExecution
        ret
@ContiSearch:
        POPAD
        mov     eax,ExceptionContinueSearch 
        ret    
@_Unwind:
        invoke	MsgBox,CTEXT("The LAST Handler unwinds")
        jmp     @ContiSearch

SELF_UNWIND     endp
;-----------------------------------------

_StArT:
        Assume fs:nothing
        invoke	SetErrorMode,0
        mov     [OLDHANDLER],eax
        
        InstSEHframe    SELF_UNWIND
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

        nop
        invoke	MsgBox,offset FixInt3Suc      ;�������ϵ�int 3�ɹ�
        
       
        invoke	MessageBox,0,CTEXT("Test Illegal INSTR with Handler&Unwind ability or Not(Y/N)?"),offset szTit,MB_YESNO
        cmp     eax,IDYES
        jnz     no_test
        db      0Fh,0bh         ;Ϊ�Ƿ�ָ�����
        nop
        nop

unwind_return:                               
        invoke	MsgBox,CTEXT("SELF UNWIND SUC NOW EXIT!")
        ;xor     eax,eax        ;uncomment this to test   
        ;mov     [eax],eax      ;
no_test:
        UnInstSEHframe          ;ж�����еĻص�����
        UnInstSEHframe
        UnInstSEHframe
        UnInstSEHframe
	invoke ExitProcess,0
END	_StArT