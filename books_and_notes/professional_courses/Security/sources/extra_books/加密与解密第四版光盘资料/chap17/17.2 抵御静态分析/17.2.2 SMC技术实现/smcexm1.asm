;-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
; ˵����	�������״���---SMC������ʾ
;
; �����ܣ��˳���������ʾSMC�����Ļ������á�
;
; ʹ��˵���������Դ�������г�����Կ���һ��Ϣ��ʾ��Ȼ�󵥲����ٳ��򣬿��Կ��������������ܵĲ���
;			������н��ܡ�
;
; ������ԣ�MASM 6.0
;
; ���뷽����ML /c /coff smcexm1.asm
; ���ӷ�����LINK /SECTION:.text,ERW /SUBSYSTEM:WINDOWS smcexm1.obj
;
; ע�����ע�����ʹ�����ĳ������ο�д������������н����ֵ�ַ����д����
;			�����ӷ����б�����/SECTION����ѡ���м���ERW���á�
;-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

.586P
.MODEL FLAT,STDCALL
OPTION CASEMAP:NONE

;-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; ����ͷ�ļ�
include		windows.inc
include		kernel32.inc
include		user32.inc

includelib	kernel32.lib
includelib	user32.lib

.RADIX 16

.DATA
szUser32Dll			BYTE	'USER32.DLL',0
szMessageBoxA		BYTE	'MessageBoxA',0
.CODE
;-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; ���ܺ���
DecryptFunc	proc uses ebx ecx edx esi edi lpBuffer:LPVOID,nBuffSize:DWORD
			mov		esi,lpBuffer
			mov		edi,esi
			mov		ecx,nBuffSize
loc_loop:	lodsb
			dec		al
			stosb
			loop	loc_loop
			ret
DecryptFunc	endp


;-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; �������
Main:
			;-=-=-=-�õ�API������ַ��ջ����-=-=-=-=-
			invoke	LoadLibraryA,addr szUser32Dll
			test	eax,eax
			jz		lm_exit
			invoke	GetProcAddress,eax,addr szMessageBoxA
			test	eax,eax
			jz		lm_exit
			push	eax
			;-=-=-=-��������-=-=-=-=-
			push	DataLen
			push	offset EnData
			call	DecryptFunc

EnData		BYTE	0ECh,01Bh,054h,076h,064h,064h,066h,074h,074h,001h
			BYTE	04Ah,021h,06Dh,070h,077h,066h,021h,075h,069h,06Ah
			BYTE	074h,021h,068h,062h,06Eh,066h,022h,001h,060h,0E9h
			BYTE	001h,001h,001h,001h,05Eh,082h,0EEh,023h,011h,041h
			BYTE	001h,06Bh,001h,08Eh,086h,003h,011h,041h,001h,051h
			BYTE	08Eh,086h,00Bh,011h,041h,001h,051h,06Bh,001h,000h
			BYTE	0D8h,08Eh,0BEh,001h,011h,041h,001h,0BAh,04Eh,001h
			BYTE	001h,001h,034h,0C1h,0FDh,0F4h,0ABh

DataLen		EQU		$ - offset EnData		;$

lm_exit:	invoke	ExitProcess,0

end Main