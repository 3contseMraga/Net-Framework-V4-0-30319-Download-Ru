/*******************************************************
/*����������ܡ�����������ʵ��
/*  ��15�� �����ټ���
/*15.3.4 ��ֹ����������
/* code by forgot 2008.3
/*(c)  ��ѩ�����ȫ��վ www.pediy.com 2000-2008
********************************************************/

		@get_api_addr	"NTDLL.DLL","ZwContinue"
		xchg	ebx,eax

;  �õ�Ntdll.dll��ZwContinue��ַ

		call	a1
		dd	0
a1:	push	PAGE_READWRITE
		push	5
		push	ebx
		call	VirtualProtect
		@check	0,"Error: cannot deprotect the region!"

;   �����ڴ��дȨ��

		lea	edi,_ZwContinue_b
		mov	ecx,0Fh
		mov	esi,ebx
		rep	movsb

;   Edi�Ĵ���ָ�������Զ����һ���СΪ0F���ڴ�����
;   ������Ntdll.ZwContinue�����Ĵ�С��rep movsbָ���
;   ԭʼZwContinue�������Ƶ�����ָ����ZwContinue_b��

		lea	eax,_ZwContinue
		mov	edi,ebx
		call 	make_jump
		
;   _ZwContinue����ַ����eax��ԭ������ַ����edi,����
;   make_jump��ԭ������ͷ����һ����תָ����ü�������

		@debug 	"attach debugger to me now!",
MB_ICONINFORMATION



exit:	mov	byte ptr [flag],1

;   �������ã�flagΪ1
		push 	0
		@callx 	ExitProcess

make_jump:
		pushad
		mov	byte ptr [edi],0E9h
		sub	eax,edi
		sub	eax,5
		mov	dword ptr [edi+1],eax
		popad
		ret

;   �������мĴ�����������ת��ʹZwContinueԭ����������
;   �ǵ�_ZwContinueִ��

flag	db	0

;   ����flag�������ж��Ƿ񱻸��ӵ���

_ZwContinue:	pushad
		cmp	byte ptr [flag],0
		jne	we_q
		@debug	"Debugger found!",MB_ICONERROR
we_q:	popad

;   �ж�flag�Ƿ�Ϊ0�����Ϊ0����⵽���������������
;   ִ������Ĵ��룬�������Ǹ��Ƶ�ZwContinueԭʼ����

_ZwContinue_b:	db	0Fh dup (0)

comment $		
		77F5B638 > B8 20000000      MOV EAX,20
		77F5B63D   BA 0003FE7F      MOV EDX,7FFE0300
		77F5B642   FFD2             CALL EDX
		77F5B644   C2 0800          RETN 8
$

;   ������ɺ����￴����Ӧ�����ϱߵ�����

end start
