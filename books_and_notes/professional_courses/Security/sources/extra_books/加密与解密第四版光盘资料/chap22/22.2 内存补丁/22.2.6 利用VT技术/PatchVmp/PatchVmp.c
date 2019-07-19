//===========================================================================
/*
 * PatchVmp:����VT����EPT�����ƹ�ǿ��У�飬��Ӧ�ó���򲹶���
 * Author	 :Joen
 * QQ		 :51753931
 * E-mail	 :Joen@JoenChen.com
 * Website	 :http://www.joenchen.com
 *
 */
//===========================================================================
#include "ntifs.h"
#include "stdint.h"
#include "structs.h"
#include "Assembly.h"
#include "HvmEpt.h"
#include "Hypervisor.h"
#include "ntimage.h"
#include "Opcode.h"

SPLIT_PROCESS_INFO SplitPsInfo;

// Ҫ���зָ�Ľ�������
char TargetAppName[] = "pediy.exe";


BYTE * MapInImageHeader(
    PHYSICAL_ADDRESS physAddr
)
{
    BYTE *pePtr = NULL;
    UINT imageSize = 0;

    pePtr = MmMapIoSpace( physAddr, PAGE_SIZE, 0 );

    if ( pePtr == NULL || *pePtr != 'M' || *( pePtr + 1 ) != 'Z' )
    {
        DbgPrint( "Invalid physical address!" );

        if ( pePtr != NULL )
        {
            MmUnmapIoSpace( pePtr, PAGE_SIZE );
        }

        return NULL;
    }

    return pePtr;
}

UINT GetImageSize(
    BYTE *peBaseAddr
)
{
    IMAGE_DOS_HEADER *dosHeader = NULL;
    IMAGE_NT_HEADERS *ntHeaders = NULL;
    SHORT *ptr = ( SHORT * ) peBaseAddr;

    dosHeader = ( IMAGE_DOS_HEADER * ) peBaseAddr;
    ntHeaders = ( IMAGE_NT_HEADERS * ) ( ( BYTE * ) peBaseAddr + dosHeader->e_lfanew );

    return ntHeaders->OptionalHeader.SizeOfImage;
}

PMDLX LockProcessMemory(
    PVOID startAddr,
    UINT len,
    PEPROCESS proc,
    PKAPC_STATE apcstate )
{
    PMDLX mdl = NULL;

    //
    // Attach to process to ensure virtual addresses are correct
    //
    KeStackAttachProcess( proc, apcstate );

	//
    // Create MDL to represent the image
	//
    mdl = IoAllocateMdl( startAddr, ( ULONG ) len, FALSE, FALSE, NULL );

    if ( mdl == NULL )
        return NULL;

	//
    // Attempt to probe and lock the pages into memory
	//
    __try
    {
        MmProbeAndLockPages( mdl, UserMode, ReadAccess );

    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        DbgPrint( "Unable to ProbeAndLockPages! Error: %x\r\n", GetExceptionCode() );

        IoFreeMdl( mdl );
        mdl = NULL;
    }

    KeUnstackDetachProcess( apcstate );

    return mdl;
}


VOID ProcessCreateMonitor( PEPROCESS Proc )
{
    SHORT numExecSections = 0;
    BYTE *pePtr = NULL;
    KAPC_STATE ApcState;
	PHYSICAL_ADDRESS tmpPhys;


    //
    // ��ȡҪ�ָ���̵Ļ�����Ϣ
    //
    SplitPsInfo.ModuleBase = PsGetProcessSectionBaseAddress( Proc );
    SplitPsInfo.CodePtr = SplitPsInfo.ModuleBase;

    KeStackAttachProcess( Proc, &ApcState );

    SplitPsInfo.cr3 = _GetCr3();

    SplitPsInfo.BasePhysicalAddress =
        MmGetPhysicalAddress( SplitPsInfo.ModuleBase );

    KeUnstackDetachProcess( &ApcState );

    SplitPsInfo.KernelModulePtr =
        MapInImageHeader( SplitPsInfo.BasePhysicalAddress );

    SplitPsInfo.ImageSize =
        GetImageSize( SplitPsInfo.KernelModulePtr );

    //
    // ȷ��Windowsû����������ҳ
    //
   SplitPsInfo.LockedMdl = LockProcessMemory(
                                SplitPsInfo.ModuleBase, SplitPsInfo.ImageSize, Proc, &ApcState );

    if( SplitPsInfo.LockedMdl == NULL )
    {
        MapOutImageHeader( SplitPsInfo.KernelModulePtr );
        return;
    }

    //
    // ��ʼ��Translation�ṹ, CodePtr = ����ִ������
    //
    SplitPsInfo.Translation = InitTranslation(
                                     SplitPsInfo.CodePtr,
                                     SplitPsInfo.ImageSize,
                                     Proc,
                                     &ApcState,
                                     &SplitPsInfo );


	//
	// Pediy_OpCode�������滻�����ڵ�0x401000��ʼ��1000 BYTE
	//
	RtlMoveMemory( SplitPsInfo.Opcode, Pediy_OpCode, sizeof( Pediy_OpCode ));

	tmpPhys = MmGetPhysicalAddress( SplitPsInfo.Opcode );

    //
    // ������PE�ļ�ҳ����Ϊ��Ч��������#VmExit
    //
    _ExecuteVmcall( VMCALL_INIT_INVALID_PAGE,
                    (ULONG)SplitPsInfo.Translation, 
					SplitPsInfo.ImageSize, tmpPhys.LowPart );
}

VOID ProcessExitMonitor( PEPROCESS proc )
{
    KAPC_STATE apcstate;

    //
    // ȡ��ҳ����Ϊ��Ч��ֹͣ����#VmExit
    //
    _ExecuteVmcall( VMCALL_END_INVALID_PAGE,
                    (ULONG)SplitPsInfo.Translation, SplitPsInfo.ImageSize, 0 );

    if( SplitPsInfo.LockedMdl != NULL )
    {
        UnlockProcessMemory( proc, &apcstate, SplitPsInfo.LockedMdl );
        SplitPsInfo.LockedMdl = NULL;
    }	

    if( SplitPsInfo.Translation != NULL )
    {
        freeTranslation( SplitPsInfo.ImageSize,
                              SplitPsInfo.Translation,
                              SplitPsInfo.targetPhys,
                              SplitPsInfo.targetPtes );

        SplitPsInfo.Translation = NULL;
        SplitPsInfo.targetPhys = NULL;
        SplitPsInfo.targetPtes = NULL;
    }

    if ( SplitPsInfo.KernelModulePtr )
    {
        MapOutImageHeader( SplitPsInfo.KernelModulePtr );
        SplitPsInfo.KernelModulePtr = NULL;
    }
}

//
// ���̴���Monitor �ڵ�IRQL����ʹ���ڴ溯��
//
void ProcessMonitor(
    HANDLE ParentId,
    HANDLE ProcessId,
    BOOLEAN Create
)
{
    PEPROCESS Eprocess;
    char *procName;

    // Get the 8.3 image name
    PsLookupProcessByProcessId( ProcessId, &Eprocess );

    procName = PsGetProcessImageFileName( Eprocess );

	//
    // Check if this is the target process
	//
    if( strncmp( TargetAppName, procName, strlen( TargetAppName ) ) == 0 )
    {
        if( Create  )
        {
            ProcessCreateMonitor( Eprocess );
        }
        else
        {
            ProcessExitMonitor( Eprocess );
        }
    }
}


//
// ��������ж������
//
VOID DriverUnload(
    IN PDRIVER_OBJECT DriverObject
)
{
    PsSetCreateProcessNotifyRoutine( &ProcessMonitor, TRUE );

    UnLoadHypervisor();
	
	if (SplitPsInfo.Opcode)
	{
		MmFreeContiguousMemory( SplitPsInfo.Opcode );
		SplitPsInfo.Opcode = NULL;
	}

    return;
}


//
// �����������
//
NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegPath
)
{
    ULONG eax, ebx, ecx, edx;
	PHYSICAL_ADDRESS MemAddr = {0};
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    DbgPrint ( "SW-> Build Time: %s \n", "" __DATE__ "/" __TIME__ );

    // KdPrint( ( "SW-> Driver Module Base: %x\n", DriverObject->DriverStart ) );
    KdPrint( ( "SW-> -----------------------------------------------------------------\n" ) );

    //
    // ���cpu Intel Vt֧�����
    //
    if( !NT_SUCCESS( CheckVtSupport() ) )
    {
        return STATUS_UNSUCCESSFUL;
    }

    _ExecuteCpuId( 'Pdiy', &eax, &ebx, &ecx, &edx );

    KdPrint( ( "SW-> Execute cpuid eax:%p ebx:%p ecx:%p edx:%p\n",
               eax, ebx, ecx, edx ) );

    //
    // ���غͿ���Intel Vt
    //
    if( !NT_SUCCESS( _LoadHypervisor( 0 ) ) )
    {
        return STATUS_UNSUCCESSFUL;
    }

	//
    // ����һ�����̻ص�, �����޸�PE�ļ�TLBӳ��
	//
    Status = PsSetCreateProcessNotifyRoutine( &ProcessMonitor, FALSE );

    if( !NT_SUCCESS( Status ) )
    {
        KdPrint( ( "SW-> PsSetCreateProcessNotifyRoutine failed!\n" ) );
        return STATUS_UNSUCCESSFUL;
    }

    DriverObject->DriverUnload = &DriverUnload;

    _ExecuteCpuId( 'Pdiy', &eax, &ebx, &ecx, &edx );

	MemAddr.LowPart = ~0;
	SplitPsInfo.Opcode = ( BYTE * ) MmAllocateContiguousMemory(
		sizeof( Pediy_OpCode ), MemAddr );


    KdPrint( ( "SW-> Execute cpuid eax:%p ebx:%p ecx:%p edx:%p\n",
               eax, ebx, ecx, edx ) );

    KdPrint( ( "SW-> *****************************************************************\n" ) );
    KdPrint( ( "SW-> *                                                               *\n" ) );
    KdPrint( ( "SW-> *             Hypervisor Loading Successfully                   *\n" ) );
    KdPrint( ( "SW-> *                                                               *\n" ) );
    KdPrint( ( "SW-> *****************************************************************\n" ) );

    return STATUS_SUCCESS;
}

