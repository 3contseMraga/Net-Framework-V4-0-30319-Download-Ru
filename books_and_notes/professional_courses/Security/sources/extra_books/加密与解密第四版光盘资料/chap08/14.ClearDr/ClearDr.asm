;******************************************
;coded by Hume,2K+
;����������ܣ����İ棩��
;(c)  ��ѩѧԺ www.kanxue.com 2000-2018
;******************************************
;����1.��ʾ��SEH�ص����������Dr�Ĵ����Դﵽ�����Ե�Ŀ��
;******************************************
include ..\asminc.h
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	.DATA
Text           db "SEH����û������!",0
TextSEH        db "Hello,SEH!",0
Caption        db "SEH",0
	.DATA?
	
.code
_start:
assume fs:nothing
;------------------------------------------------
; ��ջ�й���һ�� EXCEPTION_REGISTRATION �ṹ
        push	offset _except_handler
        push	fs:[0]       
        mov	fs:[0],esp  
;---------------------------------------------------
; �����쳣��ָ��          
	    mov	esi,0
	    mov	eax,[esi]		; ��0��ַ���ڴ��쳣
 WouldBeOmit:                                  
   	    invoke  MessageBox,0,addr Text,addr Caption,MB_OK  ; ��һ����Զ�޷���ִ��
;---------------------------------------------------
; �쳣������Ϻ󣬴����￪ʼִ��
ExecuteHere:                           
   	    invoke   MessageBox,0,addr TextSEH,addr Caption,MB_OK
;--------------------------------------------------
; �ָ�ԭ���� SEH ��
    	    pop     fs:[0]                      
    	    add     esp,4 
    	    invoke  ExitProcess,NULL        
;-------------------------------------------------
; �쳣�ص�������
_except_handler proc uses ebx pExcept:DWORD,pFrame:DWORD,pContext:DWORD,pDispatch:DWORD
        mov  	  eax,pContext 
        Assume  eax:ptr CONTEXT
        lea      ebx, ExecuteHere  	; �쳣��׼����ExecuteHere��ʼִ��
        mov     [eax].regEip,ebx  	; �޸�CONTEXT.EIP��׼���ı��������·��
        xor      ebx,ebx
        mov     [eax].iDr0,ebx    	; ��Drx���ԼĴ������㣬ʹ�ϵ�ʧЧ�������٣�
        mov     [eax].iDr1,ebx
        mov     [eax].iDr2,ebx
        mov     [eax].iDr3,ebx
        mov     [eax].iDr7,341     
        mov     eax,0            	; ����ֵ=ExceptionContinueExecution����ʾ�Ѿ��޸�
        ret                     
_except_handler endp
end _start
