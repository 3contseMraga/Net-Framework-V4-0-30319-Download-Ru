//===========================================================================
/*
 * ShadowWalker:����intel VT����ept�ڴ���������
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

SPLIT_PROCESS_INFO SplitPsInfo;


// Ҫ���зָ�Ľ�������
char TargetAppName[] = "Split.exe";

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


void CopyPe(
    PEPROCESS proc,
    PKAPC_STATE apc,
    BYTE *srcPtr,
    BYTE *targetPtr,
    UINT len )
{
    if( srcPtr == NULL || targetPtr == NULL )
        return;

    //
    // Attach to the process and copy the image to the passed buffer
    //
    KeStackAttachProcess( proc, apc );

    memcpy( targetPtr, srcPtr, len );

    KeUnstackDetachProcess( apc );
}


ULONG GetTlbSections(BYTE *peBaseAddr)
{
	IMAGE_DOS_HEADER *dosHeader = NULL;
	IMAGE_NT_HEADERS *ntHeaders = NULL;
	IMAGE_SECTION_HEADER *sectionHeader = NULL;
	ULONG i, j = 0, execSectionCount = 0, numSections = 0;

	dosHeader = (IMAGE_DOS_HEADER *) peBaseAddr;
	ntHeaders = (IMAGE_NT_HEADERS *) ((BYTE *) peBaseAddr + dosHeader->e_lfanew);
	numSections = ntHeaders->FileHeader.NumberOfSections;
	sectionHeader = (IMAGE_SECTION_HEADER *) &ntHeaders[1];

	for (i = 0; i < numSections; i++)
	{
		if (strcmp( "NewSeon", sectionHeader[i].Name) == 0 )
		{
			return sectionHeader[i].VirtualAddress;
		}
	}

	return 0;
}


VOID ProcessCreateMonitor( PEPROCESS Proc )
{
    SHORT numExecSections = 0;
    BYTE *pePtr = NULL;
    KAPC_STATE ApcState;
	ULONG NewSectionOffset;

    PHYSICAL_ADDRESS MemAddr = {0};

    //
    // ��ȡҪ�ָ���̵Ļ�����Ϣ
    //
    SplitPsInfo.ModuleBase = PsGetProcessSectionBaseAddress( Proc );
    SplitPsInfo.CodePtr = SplitPsInfo.ModuleBase;

    KeStackAttachProcess( Proc, &ApcState );

    SplitPsInfo.cr3 = _GetCr3();

    SplitPsInfo.BasePhysicalAddress =
        MmGetPhysicalAddress( SplitPsInfo.ModuleBase );

    SplitPsInfo.Eprocess = Proc;

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
    // ��������ָ���PE�ļ�
    //
    MemAddr.LowPart = ~0;

    SplitPsInfo.DataPtr = ( BYTE * ) MmAllocateContiguousMemory(
                              SplitPsInfo.ImageSize, MemAddr );

    RtlZeroMemory( ( void * ) SplitPsInfo.DataPtr, SplitPsInfo.ImageSize );

    //
    // ������PE�ļ����Ƴ���һ�ݣ�һ����ΪCodePtr����һ����ΪDataPtr
    //
    CopyPe( Proc, &ApcState, SplitPsInfo.CodePtr,
            SplitPsInfo.DataPtr, SplitPsInfo.ImageSize );

	NewSectionOffset = GetTlbSections(SplitPsInfo.KernelModulePtr );

	if (NewSectionOffset == 0)
	{
		KdPrint( ( "SW-> Did not find the NewSecon section\n" ));
		return;
		
	}

    KdPrint( ( "SW-> Code Execute [0]:%x [1]:%x [2]:%x [3]:%x \n",
               *( SplitPsInfo.DataPtr+NewSectionOffset ),
               *( SplitPsInfo.DataPtr+NewSectionOffset+1 ),
               *( SplitPsInfo.DataPtr+NewSectionOffset+2 ),
               *( SplitPsInfo.DataPtr+NewSectionOffset+3 ) ) );

			   
    *( SplitPsInfo.DataPtr+NewSectionOffset+0 ) = 'P';
    *( SplitPsInfo.DataPtr+NewSectionOffset+1 ) = 'd';
    *( SplitPsInfo.DataPtr+NewSectionOffset+2 ) = 'i';
    *( SplitPsInfo.DataPtr+NewSectionOffset+3 ) = 'y';

    KdPrint( ( "SW-> Data access [0]:%x [1]:%x [2]:%x [3]:%x \n",
               *( SplitPsInfo.DataPtr+NewSectionOffset ),
               *( SplitPsInfo.DataPtr+NewSectionOffset+1 ),
               *( SplitPsInfo.DataPtr+NewSectionOffset+2 ),
               *( SplitPsInfo.DataPtr+NewSectionOffset+3 ) ) );

    //
    // DataPtrΪ���ݷ�������
    // CodePtrΪ����ִ������
    //
    SplitPsInfo.Translation = InitTranslation(
                                     SplitPsInfo.CodePtr,
                                     SplitPsInfo.DataPtr,
                                     SplitPsInfo.ImageSize,
                                     Proc,
                                     &ApcState,
                                     &SplitPsInfo );

    //
    // �����ָ�
    //
    _ExecuteVmcall( VMCALL_INIT_SPLIT,
                    (ULONG)SplitPsInfo.Translation, SplitPsInfo.ImageSize, 0 );
}

VOID ProcessExitMonitor( PEPROCESS proc )
{
    KAPC_STATE apcstate;

    //
    // ����ָ�
    //
    _ExecuteVmcall( VMCALL_END_SPLIT,
                    (ULONG)SplitPsInfo.Translation, SplitPsInfo.ImageSize, 0 );

    if( SplitPsInfo.LockedMdl != NULL )
    {
        UnlockProcessMemory( proc, &apcstate, SplitPsInfo.LockedMdl );
        SplitPsInfo.LockedMdl = NULL;
    }

    if( SplitPsInfo.DataPtr != NULL )
    {
        MmFreeContiguousMemory( ( PVOID ) SplitPsInfo.DataPtr );
        SplitPsInfo.DataPtr = NULL;
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

    KdPrint( ( "SW-> Execute cpuid eax:%p ebx:%p ecx:%p edx:%p\n",
               eax, ebx, ecx, edx ) );

    KdPrint( ( "SW-> *****************************************************************\n" ) );
    KdPrint( ( "SW-> *                                                               *\n" ) );
    KdPrint( ( "SW-> *             Hypervisor Loading Successfully                   *\n" ) );
    KdPrint( ( "SW-> *                                                               *\n" ) );
    KdPrint( ( "SW-> *****************************************************************\n" ) );

    return STATUS_SUCCESS;
}

