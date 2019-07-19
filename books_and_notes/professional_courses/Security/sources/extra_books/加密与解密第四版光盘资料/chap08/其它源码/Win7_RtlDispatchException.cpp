//Exception Flags
#define EXCEPTION_NONCONTINUABLE 0x1    // Noncontinuable exception
#define EXCEPTION_UNWINDING 0x2         // Unwind is in progress
#define EXCEPTION_EXIT_UNWIND 0x4       // Exit unwind is in progress
#define EXCEPTION_STACK_INVALID 0x8     // Stack out of limits or unaligned
#define EXCEPTION_NESTED_CALL 0x10      // Nested exception handler call
#define EXCEPTION_TARGET_UNWIND 0x20    // Target unwind in progress
#define EXCEPTION_COLLIDED_UNWIND 0x40  // Collided exception handler call

//MmExecutionFlags on Win7
#define MEM_EXECUTE_OPTION_DISABLE 0x1 
#define MEM_EXECUTE_OPTION_ENABLE 0x2
#define MEM_EXECUTE_OPTION_DISABLE_THUNK_EMULATION 0x4
#define MEM_EXECUTE_OPTION_PERMANENT 0x8
#define MEM_EXECUTE_OPTION_EXECUTE_DISPATCH_ENABLE 0x10
#define MEM_EXECUTE_OPTION_IMAGE_DISPATCH_ENABLE 0x20
#define MEM_EXECUTE_OPTION_DISABLE_EXCEPTIONCHAIN_VALIDATION 0x40
#define MEM_EXECUTE_OPTION_VALID_FLAGS 0x7f

//NtGlobalFlag
#define FLG_ENABLE_CLOSE_EXCEPTIONS     0x00400000      // kernel mode only
#define FLG_ENABLE_EXCEPTION_LOGGING    0x00800000      // kernel mode only

//Part of ProcessInformationClass
#define ProcessExecuteFlags	34

typedef struct _DISPATCHER_CONTEXT {
	PEXCEPTION_REGISTRATION_RECORD RegistrationPointer;
} DISPATCHER_CONTEXT;

//
// Execute handler for exception function prototype.
//

EXCEPTION_DISPOSITION
	RtlpExecuteHandlerForException (
	IN PEXCEPTION_RECORD ExceptionRecord,
	IN PVOID EstablisherFrame,
	IN OUT PCONTEXT ContextRecord,
	IN OUT PVOID DispatcherContext,
	IN PEXCEPTION_ROUTINE ExceptionRoutine
	);

VOID
	RtlpGetStackLimits (
	OUT PULONG LowLimit,
	OUT PULONG HighLimit
	);

EXCEPTION_DISPOSITION
	RtlCallVectoredExceptionHandlers (
	IN PEXCEPTION_RECORD ExceptionRecord,
	IN OUT PCONTEXT ContextRecord
	);

EXCEPTION_DISPOSITION
	RtlCallVectoredContinueHandlers (
	IN PEXCEPTION_RECORD ExceptionRecord,
	IN OUT PCONTEXT ContextRecord
	);

PEXCEPTION_REGISTRATION_RECORD
	RtlpGetRegistrationHead (
	VOID
	);

BOOLEAN
	RtlIsValidHandler (
	IN PEXCEPTION_ROUTINE Handler,
	IN ULONG ProcessExecuteFlag
	);

BOOLEAN __stdcall RtlDispatchException(PEXCEPTION_RECORD pExcptRec, CONTEXT *pContext)
{
	BOOLEAN Completion; 
	PEXCEPTION_RECORD pExcptRec;
	EXCEPTION_REGISTRATION_RECORD *RegistrationPointerForCheck;
	EXCEPTION_REGISTRATION_RECORD *RegistrationPointer;
	EXCEPTION_REGISTRATION_RECORD *NestedRegistration;
	EXCEPTION_DISPOSITION Disposition; 
	EXCEPTION_RECORD ExceptionRecord1;
	DISPATCHER_CONTEXT DispatcherContext;
	ULONG ProcessExecuteOption;
	ULONG StackBase,StackLimit; 
	BOOLEAN IsSEHOPEnable;
	NTSTATUS status;

	Completion = FALSE;

	// ���ȵ���VEH�쳣�������̣��䷵��ֵ����EXCEPTION_CONTINUE_EXECUTION (0xffffffff)��EXCEPTION_CONTINUE_SEARCH (0x0)�������
	// ���Ǵ�Windows XP��ʼ������µ��쳣����ʽ
	// ����ֵ����EXCEPTION_CONTINUE_SEARCH����ô�ͽ����쳣�ַ�����
	if (RtlCallVectoredExceptionHandlers(pExcptRec, pContext)  !=  EXCEPTION_CONTINUE_SEARCH )
	{
		Completion = TRUE;
	}
	else
	{
		// ��ȡջ���ڴ淶Χ
		RtlpGetStackLimits(&StackLimit, &StackBase);
		ProcessExecuteOption = 0;

		// ��fs:[0]��ȡSEH����ͷ�ڵ�
		RegistrationPointerForCheck = RtlpGetRegistrationHead();

		// Ĭ�ϼ���SEHOP�����Ѿ����ã�����һ�ֶ�SEH���İ�ȫ�Խ�����ǿ��֤�Ļ���
		IsSEHOPEnable = TRUE; 

		// ��ѯ���̵�ProcessExecuteFlags��־�������Ƿ����SEHOP��֤
		status = ZwQueryInformationProcess(NtCurrentProcess(), ProcessExecuteFlags, &ProcessExecuteOption, sizeof(ULONG), NULL) ;

		// �ڲ�ѯʧ�ܣ�����û�����ñ�־λʱ������SEHOP��ǿ��֤
		// Ҳ����˵��ֻ������ȷ��ѯ��������SEHOPʱ�Ų��������ǿ��֤
		if ( NT_SUCCESS(status) 
			&& (ProcessExecuteOption & MEM_EXECUTE_OPTION_DISABLE_EXCEPTIONCHAIN_VALIDATION) )
		{
			// ��ȷʵδ����SEHOP��ǿУ����ƣ����ô˱�־
			IsSEHOPEnable = FALSE; 
		}
		else
		{
			// ���򣬽��п�ʼSEHOP��֤
			if ( RegistrationPointerForCheck == -1 )
				break;

			//��֤SEH���и���������Ч�Բ����������һ�����
			do
			{
				// �����������������Ϊջ��Ч����ʱ����ִ�л���ջ��SEH����
				    // 1.SEH�ڵ㲻��ջ��
				if ( (ULONG)RegistrationPointerForCheck < StackLimit 
					|| (ULONG)RegistrationPointerForCheck + 8 > StackBase
					// 2.SEH�ڵ��λ��û�а�ULONG����
					|| (ULONG)RegistrationPointerForCheck & 3 
					// 3.Handler��ջ��
					|| ((ULONG)RegistrationPointerForCheck->Handler < StackLimit || (ULONG)RegistrationPointerForCheck->Handler >= StackBase) )
				{
					pExcptRec->ExceptionFlags |= EXCEPTION_STACK_INVALID;
					goto DispatchExit;
				}
				// ȡSEH������һ�����
				RegistrationPointerForCheck = RegistrationPointerForCheck->Next;
			}
			while ( RegistrationPointerForCheck != -1 );

			// ��ʱRegistrationPointerForCheckָ�����һ���ڵ�
			// ���TEB->SameTebFlags�е�RtlExceptionAttachedλ(��9λ)�����ã������һ������Handlerȴ����Ԥ��İ�ȫSEH����ôSEHOPУ�鲻ͨ��������ִ���κ�SEHHandler
			if ((NtCurrentTeb()->SameTebFlags & 0x200) && RegistrationPointerForCheck->Handler != FinalExceptionHandler)
			{
				goto DispatchExit;
			}
		}
		
		// ��fs:[0]��ȡSEH����ͷ�ڵ�
		RegistrationPointer = RtlpGetRegistrationHead();
		NestedRegistration = NULL;

		// ����SEH����ִ��Handler
		while ( TRUE )
		{
			if ( RegistrationPointer == -1 ) //-1��ʾSEH���Ľ���
				goto DispatchExit;

			// ��SEHOP����δ������������������У�飬��֮����Ҫ����ΪSEHOP�����Ѿ���֤����
			if ( !IsSEHOPEnable )
			{
				if ( (ULONG)RegistrationPointer < StackLimit 
					|| (ULONG)RegistrationPointer + 8 > StackBase 
					|| (ULONG)RegistrationPointer & 3 
					|| ((ULONG)RegistrationPointer->Handler < StackLimit || (ULONG)RegistrationPointer->Handler >= StackBase) )
				{
					pExcptRec->ExceptionFlags |= EXCEPTION_STACK_INVALID;
					goto DispatchExit;
				}
			}

			// ����RtlIsValidHandler��Handler������ǿ��֤��Ҳ����SafeSEH����
			if (!RtlIsValidHandler(RegistrationPointer->Handler, ProcessExecuteOption))
			{
				pExcptRec->ExceptionFlags |= EXCEPTION_STACK_INVALID;
				goto DispatchExit;
			}

			// ִ��SEHHandler
			Disposition = RtlpExecuteHandlerForException(pExcptRec, RegistrationPointer, pContext, &DispatcherContext, RegistrationPointer->Handler);
			if ( NestedRegistration == RegistrationPointer )
			{
				pExcptRec->ExceptionFlags &=  (~EXCEPTION_NESTED_CALL);
				NestedRegistration = NULL;
			}

			// ���SEHHandler��ִ�н��
			switch(Disposition)
			{
			case ExceptionContinueExecution :
				if ((ExceptionRecord->ExceptionFlags &
					EXCEPTION_NONCONTINUABLE) != 0) {
						ExceptionRecord1.ExceptionCode = STATUS_NONCONTINUABLE_EXCEPTION;
						ExceptionRecord1.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
						ExceptionRecord1.ExceptionRecord = ExceptionRecord;
						ExceptionRecord1.NumberParameters = 0;
						RtlRaiseException(&ExceptionRecord1);

				} else {
					Completion = TRUE;
					goto DispatchExit;
				}

			case ExceptionContinueSearch :
				if (ExceptionRecord->ExceptionFlags & EXCEPTION_STACK_INVALID)
					goto DispatchExit;

				break;

			case ExceptionNestedException :
				ExceptionRecord->ExceptionFlags |= EXCEPTION_NESTED_CALL;
				if (DispatcherContext.RegistrationPointer > NestedRegistration) {
					NestedRegistration = DispatcherContext.RegistrationPointer;
				}

				break;

			default :
				ExceptionRecord1.ExceptionCode = STATUS_INVALID_DISPOSITION;
				ExceptionRecord1.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
				ExceptionRecord1.ExceptionRecord = ExceptionRecord;
				ExceptionRecord1.NumberParameters = 0;
				RtlRaiseException(&ExceptionRecord1);
				break;
			}

			// ȡSEH������һ�����
			RegistrationPointer = RegistrationPointer->Next;           // Next
		}
	}

DispatchExit:

	// ����VEH��ContinueHandler
	// ֻҪRtlDispatchException�����������أ���ôContinueHandler�ܻ���SEHִ����Ϻ󱻵���
	RtlCallVectoredContinueHandlers(pExcptRec, pContext);
	return Completion;
}