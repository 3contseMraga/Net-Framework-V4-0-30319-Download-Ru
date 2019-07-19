;-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
; ˵����	�������״���---SMC������ʾ
;
; �����ܣ��˳��������γ�smcexm1.asm�е�CodeBegin��ʼ��������
;
; ʹ��˵���������Դ�������г��򽫲���smcexm1.bin�ļ������ļ��е����ݼ�Ϊsmcexm1.asm����Ҫ������
;			ʹ��HexWorkShop��smcexm1.bin�ļ��е�����ת����ʮ������������ʽ�����Ƶ�smcexm1.asm���ɡ�
;
; ������ԣ�MASM 6.0
;
; ���뷽����ML /c /coff trexm1.asm
; ���ӷ�����LINK /SECTION:.text,ERW /SUBSYSTEM:WINDOWS trexm1.obj
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

;-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; ��������
EncryptFunc			proto :LPVOID,:DWORD
WriteToFile			proto :LPVOID,:LPVOID,:DWORD

;-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; ���ݶ���
.DATA
szErrTitle		BYTE	"Error Information",0
szErrMsg1		BYTE	"Error: Create File for Write Data Failed!",0
szErrMsg2		BYTE	"Error: Write Data to File Failed!",0
szErrMsg3		BYTE	"Can't Write Data to File!",0
szGoodTitle		BYTE	"Success",0
szGoodMsg		BYTE	"Ok! Write Success!",0
lpszFileName	BYTE	"smcexm1.bin",0

.CODE
;-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; �������ܵĴ���
.RADIX 16
ALIGN	1
CodeBegin:	jmp		loc_begin

szTitle		BYTE	'Success',0
szMsg		BYTE	'I love this game!',0

loc_begin:
			;-=-=-=-�Ӷ�ջ�еõ�MessageBoxA�����ĵ�ַ-=-=-=-=-
			pop		edi
			;-=-=-=-��������ʱ�ĵ�ַ������ַ֮��-=-=-=-=-
			call	loc_next
loc_next:	pop		ebp
			sub		ebp,offset loc_next
			;-=-=-=-����MessageBoxA������ʾ��Ϣ-=-=-=-=-
			push	MB_OK
			lea		eax,[ebp + szTitle]
			push	eax
			lea		eax,[ebp + szMsg]
			push	eax
			push	NULL
			call	edi
			;-=-=-=-������Ĵ�������-=-=-=-=-
			lea		edi,[ebp + CodeBegin]
			mov		ecx,CodeLen
			xor		eax,eax
			cld
			rep		stosb
CodeEnd:
CodeLen		=		(offset CodeEnd - offset CodeBegin)

;-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; ���ܺ���
EncryptFunc	proc uses ebx ecx edx esi edi lpBuffer:LPVOID,nBuffSize:DWORD
			mov		esi,lpBuffer
			mov		edi,esi
			mov		ecx,nBuffSize
loc_loop:	lodsb
			inc		al
			stosb
			loop	loc_loop
			ret
EncryptFunc	endp

;-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; ������д���ļ�
WriteToFile	proc uses ebx ecx edx esi edi pszFileName:LPVOID,lpBuffer:LPVOID,dwBuffSize:DWORD
	LOCAL	hFile			:	DWORD
	LOCAL	dwWriteBytes	:	DWORD
	LOCAL	dwResult		:	DWORD
			mov		[hFile],INVALID_HANDLE_VALUE
			mov		[dwWriteBytes],0
			mov		[dwResult],FALSE

			;-=-=-=-=-=-���ļ�����д-=-=-=-=-=-
			invoke	CreateFileA,pszFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,
								CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL
			cmp		eax,INVALID_HANDLE_VALUE
			jnz		@F
			invoke	MessageBoxA,NULL,addr szErrMsg1,addr szErrTitle,MB_OK
			mov		[dwResult],FALSE
			jmp		loc_safe_ret
@@:			mov		[hFile],eax

			;-=-=-=-=-=-д����������-=-=-=-=-=-
			invoke	WriteFile,[hFile],lpBuffer,dwBuffSize,addr dwWriteBytes,NULL
			mov		eax,[dwWriteBytes]
			cmp		eax,dwBuffSize
			jz		@F
			invoke	MessageBoxA,NULL,addr szErrMsg2,addr szErrTitle,MB_OK
			mov		[dwResult],FALSE
			jmp		loc_safe_ret
@@:			mov		[dwResult],TRUE

loc_safe_ret:
			;-=-=-=-=-=-�ر��ļ�-=-=-=-=-=-
			cmp		[hFile],INVALID_HANDLE_VALUE
			jz		@F
			invoke	CloseHandle,[hFile]
@@:			mov		eax,[dwResult]
			ret
WriteToFile			endp

;-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; �������
Main:
			push	CodeLen
			push	offset CodeBegin
			call	EncryptFunc
			push	CodeLen
			push	offset CodeBegin
			push	offset lpszFileName
			call	WriteToFile
			cmp		eax,TRUE
			jz		@F
			invoke	MessageBoxA,NULL,addr szErrMsg3,addr szErrTitle,MB_OK
			jmp		lm_exit
@@:			invoke	MessageBoxA,NULL,addr szGoodMsg,addr szGoodTitle,MB_OK
lm_exit:	invoke	ExitProcess,0
end Main
