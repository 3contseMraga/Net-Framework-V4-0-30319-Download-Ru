//===========================================================================
/*
 * Hypervisor:����Intel VT����Ҫ����
 * Author	 :Joen
 * QQ		 :51753931
 * E-mail	 :Joen@JoenChen.com
 * Website	 :http://www.joenchen.com
 * ����ļ��ǿ���VT����VT��Ϣ��һЩ�����Ժ���
 */
//===========================================================================
#include "ntifs.h"
#include "stdint.h"
#include "Assembly.h"
#include "structs.h"
#include "HvmEpt.h"
#include "Hypervisor.h"


ULONG VmxAdjustControls( ULONG Ctl, ULONG Msr )
{
    LARGE_INTEGER MsrValue;

    MsrValue.QuadPart = _ReadMsr( Msr );
    Ctl &= MsrValue.HighPart;     /* bit == 0 in high word ==> must be zero */
    Ctl |= MsrValue.LowPart;      /* bit == 1 in low word  ==> must be one  */
    return Ctl;
}

//
// ��GDTȡ����Ӧ�Ķ�������, ���洢�ɽṹ��
//
NTSTATUS InitializeSegmentSelector(
    PSEGMENT_SELECTOR SegmentSelector,
    USHORT Selector,
    ULONG GdtBase )
{
    PSEGMENT_DESCRIPTOR2 SegDesc;

    if( !SegmentSelector )
    {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // �����ѡ���ӵ�T1 = 1��ʾ����LDT�е���, ����û��ʵ���������
    //
    if( Selector & 0x4 )
    {
        KdPrint( ( "SW-> Selector in the LDT failed\n" ) );
        return STATUS_INVALID_PARAMETER;
    }

    //
    // ��GDT��ȡ��ԭʼ�Ķ�������
    //
    SegDesc = ( PSEGMENT_DESCRIPTOR2 )( ( PUCHAR ) GdtBase + ( Selector & ~0x7 ) );

    //
    // ��ѡ����
    //
    SegmentSelector->Selector = Selector;

    //
    // �λ�ַ15-39λ 55-63λ
    //
    SegmentSelector->Base = SegDesc->base0 | SegDesc->base1 << 16 | SegDesc->base2 << 24;

    //
    // ���޳�0-15λ  47-51λ, ������ȡ��
    //
    SegmentSelector->Limit = SegDesc->limit0 | ( SegDesc->limit1attr1 & 0xf ) << 16;

    //
    // ������39-47 51-55 ע��۲�ȡ��
    //
    SegmentSelector->Attributes.UCHARs = SegDesc->attr0 | ( SegDesc->limit1attr1 & 0xf0 ) << 4;

    //
    // �����ж����Ե�DTλ, �ж��Ƿ���ϵͳ�����������Ǵ������ݶ�������
    //
    if( !( SegDesc->attr0 & LA_STANDARD ) )
    {
        ULONG64 tmp;

        //
        // �����ʾ��ϵͳ��������������������, �о�����Ϊ64λ׼���İ�,
        // 32λ����λ�ַֻ��32λ��. �ѵ�64λ������ʲô������?
        //
        tmp = ( *( PULONG64 )( ( PUCHAR ) SegDesc + 8 ) );

        SegmentSelector->Base = ( SegmentSelector->Base & 0xffffffff ) | ( tmp << 32 );
    }

    //
    // ���Ƕν��޵�����λ, 1Ϊ4K. 0Ϊ1BYTE
    //
    if( SegmentSelector->Attributes.fields.g )
    {
        //
        // �������λΪ1, ��ô�ͳ���4K. ���ƶ�12λ
        //
        SegmentSelector->Limit = ( SegmentSelector->Limit << 12 ) + 0xfff;
    }

    return STATUS_SUCCESS;
}


void CmSetBit32( ULONG* dword, ULONG bit )
{
    ULONG mask = ( 1 << bit );
    *dword = *dword | mask;
}

void CmClearBit32( ULONG* dword, ULONG bit )
{
    ULONG mask = 0xFFFFFFFF;
    ULONG sub = ( 1 << bit );
    mask = mask - sub;
    *dword = *dword & mask;
}

//
// ��ȡ���������޳�
//
ULONG GetSegmentDescriptorLimit(
    ULONG gdt_base,
    USHORT selector
)
{
    SEGMENT_SELECTOR SegmentSelector = { 0 };

    //
    // ��GDT���������������޳�
    //
    InitializeSegmentSelector( &SegmentSelector, selector, gdt_base );

    return SegmentSelector.Limit;
}

ULONG
GetSegmentDescriptorAR(
    ULONG gdt_base,
    USHORT selector
)
{
    SEGMENT_SELECTOR SegmentSelector = { 0 };
    ULONG uAccessRights;

    InitializeSegmentSelector( &SegmentSelector, selector, gdt_base );

    uAccessRights = ( ( PUCHAR ) & SegmentSelector.Attributes )[0] + \
                    ( ( ( PUCHAR ) & SegmentSelector.Attributes )[1] << 12 );

    if( !selector )
        uAccessRights |= 0x10000;

    return uAccessRights;
}


// ��ȡ������������
ULONG
GetSegmentDescriptorAttributes(
    ULONG gdt_base,
    USHORT Selector )
{
    SEGMENT_SELECTOR SegmentSelector = { 0 };
    ULONG uAccessRights;

    //
    // ��GDT��������������Ҫ�Ķ�������
    //
    InitializeSegmentSelector( &SegmentSelector, Selector, gdt_base );

    //
    // �����������������ΪʲôҪ����, �ο�Intel �ֲ�24��Table 24-2
    //
    uAccessRights = ( ( PUCHAR ) & SegmentSelector.Attributes )[0] +
                    ( ( ( PUCHAR ) & SegmentSelector.Attributes )[1] << 12 );

    if( !Selector )
    {
        uAccessRights |= 0x10000;
    }

    return uAccessRights;
}

// ��ȡ����������ַ
ULONG
GetSegmentDescriptorBase(
    ULONG gdt_base,
    USHORT seg_selector )
{
    ULONG Base = 0;
    SEGMENT_DESCRIPTOR	SegDescriptor = {0};

    //
    // ��GDT����������������
    //
    RtlCopyMemory( &SegDescriptor, ( ULONG * )( gdt_base + ( seg_selector >> 3 ) * 8 ), 8 );

    //
    // ���������ĸ�8λ
    //
    Base = SegDescriptor.BaseHi;
    Base <<= 8;

    //
    // ����������31-39λ
    //
    Base |= SegDescriptor.BaseMid;
    Base <<= 16;

    //
    // ���������� 15-31λ
    //
    Base |= SegDescriptor.BaseLo;

    return Base;
}

//
// ���ǲ���ӭ��#VMExit����. ����ָ��Ϳ���
//
VOID
HandleUnimplemented(
    P_CPU_VM_CONTEXT CpuInfo,
    ULONG ExitCode )
{
    ULONG Instlen;
    ULONG IInfo, AState;

    IInfo = _ReadVMCS( GUEST_INTERRUPTIBILITY_INFO );
    AState = _ReadVMCS( GUEST_ACTIVITY_STATE );

    Instlen = _ReadVMCS( VM_EXIT_INSTRUCTION_LEN );

    _WriteVMCS( GUEST_EIP, _ReadVMCS( GUEST_EIP ) + Instlen );
}

//
// ִ��cpuidָ��,
//
VOID HandleCpuid(
    PGUEST_REGS GuestReg
)
{
    ULONG Function, eax, ebx, ecx, edx;
    ULONG Instlen;

    Function = GuestReg->RegEax;

    _ExecuteCpuId( Function, &eax, &ebx, &ecx, &edx );

    GuestReg->RegEax = eax;
    GuestReg->RegEcx = ecx;
    GuestReg->RegEdx = edx;
    GuestReg->RegEbx = ebx;

    if( Function == 'Pdiy' )
    {
        GuestReg->RegEax = 0x11111111;
        GuestReg->RegEcx = 0x22222222;
        GuestReg->RegEdx = 0x33333333;
        GuestReg->RegEbx = 0x44444444;
    }

    Instlen = _ReadVMCS( VM_EXIT_INSTRUCTION_LEN );

    _WriteVMCS( GUEST_EIP, _ReadVMCS( GUEST_EIP ) + Instlen );
}

VOID HandleInvd(
    P_CPU_VM_CONTEXT CpuInfo
)
{
    ULONG Instlen;

    _ExecuteInvd();

    KdPrint( ( "SW-> Invd: %p\n",  _ReadVMCS( GUEST_EIP ) ) );

    Instlen = _ReadVMCS( VM_EXIT_INSTRUCTION_LEN );
    _WriteVMCS( GUEST_EIP, _ReadVMCS( GUEST_EIP ) + Instlen );
}


//
// ʶ��vmcallָ���Ƿ������������ر��������
// �������� eax == 'Joen' ecx == 'Chen' edx == 'SW' ebx = 'Quit'
//
VOID
HandleVmCall(
    P_CPU_VM_CONTEXT CpuInfo,
    PGUEST_REGS GuestReg )
{
    PIDT_ENTRY IdtBase;
    ULONG Instlen, Eip, Esp;

    Eip = _ReadVMCS( GUEST_EIP );

    Instlen = _ReadVMCS( VM_EXIT_INSTRUCTION_LEN );

    if( ( GuestReg->RegEax == 'Joen' ) &&
            ( GuestReg->RegEcx == 'Chen' ) &&
            ( GuestReg->RegEdx == 'SW' ) &&
            ( GuestReg->RegEbx == 'Quit' ) )
    {
        //
        //  Set the EIP and call VMXOFF
        //
        Eip = ( ULONG )&_GuestExitPoint;
        Esp = GuestReg->RegEsp;

        KdPrint( ( "SW-> VmxOff Guest Eip: %p Host Eps:%p Guest Esp: %p\n", Eip, _ReadVMCS( HOST_ESP ), Esp ) );

        _InvVpidAllContext();
        _InvEptAllContext();

        _ExecuteVmxOff( Esp, Eip );
        return;

    }
    else if ( GuestReg->RegEax == VMCALL_INIT_INVALID_PAGE )
    {
        //
        // ������PE�ļ�ҳ����Ϊ��Ч��������#VmExit
        //
        CpuInfo->EptInfo->ImageSize = GuestReg->RegEcx;


        Init_InvalidPage( CpuInfo->EptInfo, ( PTLB_TRANSLATION )GuestReg->RegEbx,
                    GuestReg->RegEcx, GuestReg->RegEdx );
    }
    else if ( GuestReg->RegEax == VMCALL_END_INVALID_PAGE )
    {
        //
        // �������̷ָ�
        //
        End_InvalidPage( CpuInfo->EptInfo, ( PTLB_TRANSLATION )GuestReg->RegEbx,
                   GuestReg->RegEcx );
    }
    else if ( GuestReg->RegEax == VMCALL_GET_ESP )
    {
        //
        // ��ȡCPUָ��
        //
        GuestReg->RegEax = _ReadVMCS( HOST_ESP );
    }

    _WriteVMCS( GUEST_EIP, Eip + Instlen );

}

//
// �����ִ��vmxָ��, ��ô����ʧ��!
//
VOID HandleVmInstruction(
    P_CPU_VM_CONTEXT CpuInfo
)
{
    ULONG Instlen;

    _WriteVMCS( GUEST_RFLAGS, _ReadVMCS( GUEST_RFLAGS ) | 0x1 );

    Instlen = _ReadVMCS( VM_EXIT_INSTRUCTION_LEN );
    _WriteVMCS( GUEST_EIP, _ReadVMCS( GUEST_EIP ) + Instlen );
}


//
// Cr�Ĵ�������
//
VOID HandleCrAccess(
    PGUEST_REGS GuestReg
)
{
    ULONG Exit;
    ULONG Cr;
    ULONG Reg;
    ULONG Instlen;
    PMOV_CR_QUALIFICATION ExitQualification;
    ULONG GuestEip = _ReadVMCS( GUEST_EIP );

    Exit = _ReadVMCS( EXIT_QUALIFICATION );
    ExitQualification = ( PMOV_CR_QUALIFICATION )&Exit;

    if( 0 == ExitQualification->ControlRegister )
    {
        Cr = _ReadVMCS( GUEST_CR0 );

    }
    else if( 3 == ExitQualification->ControlRegister )
    {
        Cr = _ReadVMCS( GUEST_CR3 );
    }
    else if( 4 == ExitQualification->ControlRegister )
    {
        Cr = _ReadVMCS( GUEST_CR4 );
    }

    if( 0 == ExitQualification->Register )
    {
        Reg = GuestReg->RegEax;
    }
    else if( 1 == ExitQualification->Register )
    {
        Reg = GuestReg->RegEcx;
    }
    else if( 2 == ExitQualification->Register )
    {
        Reg = GuestReg->RegEdx;
    }
    else if( 3 == ExitQualification->Register )
    {
        Reg = GuestReg->RegEbx;
    }
    else if( 4 == ExitQualification->Register )
    {
        Reg = GuestReg->RegEsp;
    }
    else if( 5 == ExitQualification->Register )
    {
        Reg = GuestReg->RegEbp;
    }
    else if( 6 == ExitQualification->Register )
    {
        Reg = GuestReg->RegEsi;
    }
    else if( 7 == ExitQualification->Register )
    {
        Reg = GuestReg->RegEdi;
    }

//---------------------------------------------------------------------------
    if( 0 == ExitQualification->AccessType )
    {
        // MOV_TO_CR
        if( 0 == ExitQualification->ControlRegister )
        {
            _WriteVMCS( GUEST_CR0, Reg );
        }
        else if( 3 == ExitQualification->ControlRegister )
        {
            _WriteVMCS( GUEST_CR3, Reg );
        }
        else if( 4 == ExitQualification->ControlRegister )
        {
            _WriteVMCS( GUEST_CR4, Reg );
        }
    }
    else if( 1 == ExitQualification->AccessType )
    {

        // MOV_FROM_CR
        if( 0 == ExitQualification->Register )
        {
            GuestReg->RegEax = Cr;
        }
        else if( 1 == ExitQualification->Register )
        {
            GuestReg->RegEcx = Cr;
        }
        else if( 2 == ExitQualification->Register )
        {
            GuestReg->RegEdx = Cr;
        }
        else if( 3 == ExitQualification->Register )
        {
            GuestReg->RegEbx = Cr;
        }
        else if( 4 == ExitQualification->Register )
        {
            GuestReg->RegEsp = Cr;
        }
        else if( 5 == ExitQualification->Register )
        {
            GuestReg->RegEbp = Cr;
        }
        else if( 6 == ExitQualification->Register )
        {
            GuestReg->RegEsi = Cr;
        }
        else if( 7 == ExitQualification->Register )
        {
            GuestReg->RegEdi = Cr;
        }
    }

    _InvVpidAllContext();
    _InvEptAllContext();

    Instlen = _ReadVMCS( VM_EXIT_INSTRUCTION_LEN );
    _WriteVMCS( GUEST_EIP, _ReadVMCS( GUEST_EIP ) + Instlen );
}

void
VmxInternalHvmInjectException(
    ULONG type,
    ULONG trap,
    ULONG error_code )
{
    ULONG v;

    //
    // Write the VM-entry interruption-information field
    //
    v = ( INTR_INFO_VALID_MASK | trap | type );

    //
    // Check if bits 11 (deliver code) and 31 (valid) are set. In this
    // case, error code has to be delivered to guest OS
    //
    if( error_code != HVM_DELIVER_NO_ERROR_CODE )
    {
        _WriteVMCS( VM_ENTRY_EXCEPTION_ERROR_CODE, error_code );
        v |= INTR_INFO_DELIVER_CODE_MASK;
    }

    _WriteVMCS( VM_ENTRY_INTR_INFO_FIELD, v );
}

void VmxHvmInjectHwException(
    ULONG trap,
    ULONG error_code )
{
    VmxInternalHvmInjectException( INTR_TYPE_HW_EXCEPTION, trap, error_code );
}

//
// ������쳣
//
VOID
HandleException(
    P_CPU_VM_CONTEXT CpuInfo
)
{
    ULONG trap, error_code;
    ULONG Instlen;
    PINTERRUPT_INJECT_INFO_FIELD InjectEvent;
    ULONG ExitInterrupt = _ReadVMCS( VM_EXIT_INTR_INFO );
    ULONG GuestEip = _ReadVMCS( GUEST_EIP );
    ULONG GuestCr3 = _ReadVMCS( GUEST_CR3 );
    ULONG ExitQualification = _ReadVMCS( EXIT_QUALIFICATION );

    trap = ExitInterrupt & INTR_INFO_VECTOR_MASK;

    //
    // Check if bits 11 (deliver code) and 31 (valid) are set. In this
    // case, error code has to be delivered to guest OS
    //
    if( ( ExitInterrupt & INTR_INFO_DELIVER_CODE_MASK ) &&
            ( ExitInterrupt & INTR_INFO_VALID_MASK ) )
    {
        error_code = _ReadVMCS( VM_EXIT_INTR_ERROR_CODE );;
    }
    else
    {
        error_code = HVM_DELIVER_NO_ERROR_CODE;
    }

    if( trap == TRAP_PAGE_FAULT )
    {
        //
        //  ������Guestע��#PF
        //
        ULONG ErrorCode;
        ULONG InjectEvent;
        PINTERRUPT_INJECT_INFO_FIELD pInjectEvent;

        pInjectEvent = ( PINTERRUPT_INJECT_INFO_FIELD )&InjectEvent;
        ErrorCode = _ReadVMCS( VM_EXIT_INTR_ERROR_CODE );
        ExitQualification = _ReadVMCS( EXIT_QUALIFICATION );

        _SetCr2( ExitQualification );
        _WriteVMCS( VM_ENTRY_EXCEPTION_ERROR_CODE, ErrorCode );

        InjectEvent = 0;
        pInjectEvent->Vector = PAGE_FAULT_EXCEPTION;
        pInjectEvent->InterruptionType = HARDWARE_EXCEPTION;
        pInjectEvent->DeliverErrorCode = 1;
        pInjectEvent->Valid = 1;

        _WriteVMCS( VM_ENTRY_INTR_INFO_FIELD, InjectEvent );
        _WriteVMCS( GUEST_EIP, _ReadVMCS( GUEST_EIP ) );

    }
//---------------------------------------------------------------------------
    else
    {
        //
        // ����ָ��, Ҳ����Hypervisor�Ե����쳣
        //
        Instlen = _ReadVMCS( VM_EXIT_INSTRUCTION_LEN );

        _WriteVMCS( GUEST_EIP, _ReadVMCS( GUEST_EIP ) + Instlen );
        return;
    }

    /* Pass hw exception to the guest OS */
    VmxHvmInjectHwException( trap, error_code );
}

//
// ��������о���Windows�Ĵ��ڹ��̺�����. ����#VmExit�����ﴦ��
//
VOID HandleVmExit(
    P_CPU_VM_CONTEXT CpuInfo,
    PGUEST_REGS GuestReg )
{
    ULONG ExitCode;


    //
    // �ѿͻ���esp�Ĵ��������
    //
    GuestReg->RegEsp = _ReadVMCS( GUEST_ESP );

    //
    // �˳�VM��ԭ��
    //
    ExitCode = _ReadVMCS( VM_EXIT_REASON );

//---------------------------------------------------------------------------
    if( ExitCode == EXIT_REASON_CR_ACCESS )
    {
        //
        // Cr �Ĵ���������д��
        //
        HandleCrAccess( GuestReg );
    }
    else if ( ExitCode == EXIT_REASON_EPT_MISCONFIG )
    {
        // Ept���ô���
        KdPrint( ( "SW-> ept���ô���\n" ) );
    }
    else if( ExitCode == EXIT_REASON_EPT_VIOLATION )
    {
        //
        // Handle the EPT violation
        //
        Handle_Exec_Ept( CpuInfo->EptInfo, GuestReg );
    }
    else if( ExitCode == EXIT_REASON_EXCEPTION_NMI )
    {
        //
        // �ж��쳣����, ���ﲻ��Ҫ����Guest.Eip
        //
        HandleException( CpuInfo );
    }
    else if( ExitCode == EXIT_REASON_EXTERNAL_INTERRUPT ||
             ExitCode == EXIT_REASON_TRIPLE_FAULT ||
             ExitCode == EXIT_REASON_INIT ||
             ExitCode == EXIT_REASON_SIPI ||
             ExitCode == EXIT_REASON_IO_SMI ||
             ExitCode == EXIT_REASON_OTHER_SMI ||
             ExitCode == EXIT_REASON_PENDING_INTERRUPT ||
             ExitCode == EXIT_REASON_TASK_SWITCH ||
             ExitCode == EXIT_REASON_HLT ||
             ExitCode == EXIT_REASON_INVLPG ||
             ExitCode == EXIT_REASON_RDPMC ||
             ExitCode == EXIT_REASON_RDTSC ||
             ExitCode == EXIT_REASON_RSM )
    {
        //
        // ���ǲ���ӭ��#VMExit����. ����ָ��Ϳ���
        //
        HandleUnimplemented( CpuInfo, ExitCode );
    }
    else if( ExitCode == EXIT_REASON_CPUID )
    {
        //
        // ִ��cpuidָ��
        //
        HandleCpuid( GuestReg );
    }
    else if( ExitCode == EXIT_REASON_INVD )
    {
        HandleInvd( CpuInfo );
    }
    else if( ExitCode == EXIT_REASON_VMCALL )
    {
        //
        // #VMExit Off Vmx
        //
        HandleVmCall( CpuInfo, GuestReg );
    }
    else if( ExitCode == EXIT_REASON_VMCLEAR ||
             ExitCode == EXIT_REASON_VMLAUNCH ||
             ExitCode == EXIT_REASON_VMPTRLD ||
             ExitCode == EXIT_REASON_VMPTRST ||
             ExitCode == EXIT_REASON_VMREAD ||
             ExitCode == EXIT_REASON_VMRESUME ||
             ExitCode == EXIT_REASON_VMWRITE ||
             ExitCode == EXIT_REASON_VMXOFF ||
             ExitCode == EXIT_REASON_VMXON )
    {
        //
        // Execute Vmx Instruction
        //
        HandleVmInstruction( CpuInfo );
    }
    else if( ExitCode == EXIT_REASON_DR_ACCESS )
    {
        //
        // Dr �Ĵ���������д��
        //
        //HandleDrAccess( CpuInfo, GuestReg );
    }
    else if( ExitCode == EXIT_REASON_IO_INSTRUCTION )
    {
        //
        // IO�˿ڷ���
        //
        // HandleIoAccess( CpuInfo );
    }
    else if( ExitCode == EXIT_REASON_MSR_READ )
    {
        //
        // MSR �Ĵ�������
        //
        //HandleMsrRead( CpuInfo, GuestReg );
    }
    else if( ExitCode == EXIT_REASON_MSR_WRITE )
    {
        //
        // MSR�Ĵ���д��
        //
        //HandleMsrWrite( CpuInfo, GuestReg );
    }
    else if( ExitCode == EXIT_REASON_INVALID_GUEST_STATE ||
             ExitCode == EXIT_REASON_MSR_LOADING ||
             ExitCode == EXIT_REASON_MWAIT_INSTRUCTION ||
             ExitCode == EXIT_REASON_MONITOR_INSTRUCTION ||
             ExitCode == EXIT_REASON_PAUSE_INSTRUCTION ||
             ExitCode == EXIT_REASON_MACHINE_CHECK ||
             ExitCode == EXIT_REASON_TPR_BELOW_THRESHOLD )
    {
        HandleUnimplemented( CpuInfo, ExitCode );
    }
    else
    {
        HandleUnimplemented( CpuInfo, ExitCode );
    }

    //
    // дesp���ͻ���,esp���ܻ����,��CrAccess�ǲ���
    //
    _WriteVMCS( GUEST_ESP, GuestReg->RegEsp );

}



//
// ��д VT Vmx�ṹ
//
NTSTATUS
SetupVMX(
    P_CPU_VM_CONTEXT CpuInfo
)
{
    NTSTATUS Status;
    ULONG ulSize;
    ULONG64	 Msr;
    PVMX_BASIC_MSR Vmx;

    Status = STATUS_SUCCESS;

    RtlZeroMemory( CpuInfo, sizeof( CPU_VM_CONTEXT ) );

    //
    // ��ģ�ͼĴ���(������VMX�İ汾, ���ڴ�����)
    //
    Msr = _ReadMsr( MSR_IA32_VMX_BASIC );
    Vmx = ( PVMX_BASIC_MSR )&Msr;

    //
    // Vmx�ṹ���С.
    //
    ulSize = Vmx->szVmxOnRegion;

//---------------------------------------------------------------------------

    //
    // Allocate memory for VMXON, Ȼ��ִ��vmxonָ��
    //
    CpuInfo->VmxonVirtualAddress = MmAllocateNonCachedMemory( 0x1000 );

    if( CpuInfo->VmxonVirtualAddress == NULL )
    {
        KdPrint( ( "SW-> Vmxon allocate memroy failed\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( CpuInfo->VmxonVirtualAddress, 0x1000 );

    //
    // vmxon�汾��.
    //
    *( ULONG * )CpuInfo->VmxonVirtualAddress = Vmx->RevId;

    //
    // vmxon�ṹ�ĵ������ַ.
    //
    CpuInfo->VmxonPyhicalAddress  = MmGetPhysicalAddress( CpuInfo->VmxonVirtualAddress );

//     KdPrint( ( "SW-> Vmxon Pyhical Address:\t %p Low: %p\n",
//                CpuInfo->VmxonPyhicalAddress.HighPart, CpuInfo->VmxonPyhicalAddress.LowPart ) );

    //
    // ִ��vmxonָ��.
    //

    _ExecuteVmxOn( CpuInfo->VmxonPyhicalAddress.LowPart,
                   CpuInfo->VmxonPyhicalAddress.HighPart );

    // ���CF���λ
    if( _VmFailInvalid() )
    {
        KdPrint( ( "SW-> Vmxon Execute failed\n" ) );
        Status = STATUS_UNSUCCESSFUL;
        goto __Exit;
    }

//---------------------------------------------------------------------------
    //
    // ��д�Լ���ָ��, ���˳���ʱ���ͷ�
    //
    CpuInfo->Self = CpuInfo;

    //
    // ������VMCS�ṹ
    //
    CpuInfo->VmcsVirtualAddress = MmAllocateNonCachedMemory( 0x1000 );

    if( CpuInfo->VmcsVirtualAddress == NULL )
    {
        KdPrint( ( "SW-> VMCS allocate memroy failed\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( CpuInfo->VmcsVirtualAddress, 0x1000 );

    //
    // ��д�汾��
    //
    *( ULONG * )CpuInfo->VmcsVirtualAddress = Vmx->RevId;

    CpuInfo->VmcsPyhicalAddress = MmGetPhysicalAddress( CpuInfo->VmcsVirtualAddress );

    //KdPrint( ( "SW-> VMCS Pyhical Address High: %p Low: %p\n",
    //           CpuInfo->VmcsPyhicalAddress.HighPart, CpuInfo->VmcsPyhicalAddress.LowPart ) );
//---------------------------------------------------------------------------
    //
    // �����MSRλͼ.
    //
    CpuInfo->MSRBitmapVirtualAddress = MmAllocateNonCachedMemory( 0x4000 );

    if( CpuInfo->MSRBitmapVirtualAddress == NULL )
    {
        KdPrint( ( "SW-> MSR Bitmap allocate memroy failed\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( CpuInfo->MSRBitmapVirtualAddress, 0x4000 );

    CpuInfo->MSRBitmapPyhicalAddress = MmGetPhysicalAddress( CpuInfo->MSRBitmapVirtualAddress );

    //KdPrint( ( "SW-> MSR Bitmap Pyhical Address High: %p Low: %p\n",
    //           CpuInfo->MSRBitmapPyhicalAddress.HighPart, CpuInfo->MSRBitmapPyhicalAddress.LowPart ) );

//---------------------------------------------------------------------------
    //
    // ����� IOBitmapA
    //
    CpuInfo->IOBitmapVirtualAddressA = MmAllocateNonCachedMemory( 0x1000 );

    if( CpuInfo->IOBitmapVirtualAddressA == NULL )
    {
        KdPrint( ( "SW-> IOBitmapA allocate memroy failed\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( CpuInfo->IOBitmapVirtualAddressA, 0x1000 );
    CpuInfo->IOBitmapPyhicalAddressA = MmGetPhysicalAddress( CpuInfo->IOBitmapVirtualAddressA );

    // KdPrint( ( "SW-> IOBitmapA Pyhical Address High: %p Low: %p\n",
    //          CpuInfo->IOBitmapPyhicalAddressA.HighPart, CpuInfo->IOBitmapPyhicalAddressA.LowPart ) );

//---------------------------------------------------------------------------
    //
    // ����� IOBitmapB
    //
    CpuInfo->IOBitmapVirtualAddressB = MmAllocateNonCachedMemory( 0x1000 );

    if( CpuInfo->IOBitmapVirtualAddressB == NULL )
    {
        KdPrint( ( "SW-> IOBitmapB allocate memroy failed\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( CpuInfo->IOBitmapVirtualAddressB, 0x1000 );

    CpuInfo->IOBitmapPyhicalAddressB = MmGetPhysicalAddress( CpuInfo->IOBitmapVirtualAddressB );

    //KdPrint( ( "SW-> IOBitmapB Pyhical Address High: %p Low: %p\n",
    //           CpuInfo->IOBitmapPyhicalAddressB.HighPart, CpuInfo->IOBitmapPyhicalAddressB.LowPart ) );
//---------------------------------------------------------------------------
    //
    // �������д Host Stack �ṹ, 8K�ڴ�, ע�������ﲻҪ��MmAllocateNonCachedMemory, ������.
    //
    CpuInfo->HostEsp = ExAllocatePoolWithTag( NonPagedPool, 0x8000, 'Esp' );

    if( CpuInfo->HostEsp == NULL )
    {
        KdPrint( ( "SW-> Host Stack allocate memroy failed\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( CpuInfo->HostEsp, 0x8000 );

    // KdPrint( ( "SW-> Host Stack adddress: %p\n", CpuInfo->HostEsp ) );
//---------------------------------------------------------------------------
    //
    // ����EptInfo�ṹ�ڴ�
    //
    CpuInfo->EptInfo = ExAllocatePoolWithTag( NonPagedPool, sizeof( EPT_INFO ), 'EPT' );

    if( CpuInfo->EptInfo == NULL )
    {
        KdPrint( ( "SW-> Ept Info allocate memroy failed\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( CpuInfo->EptInfo, sizeof( EPT_INFO ) );
//---------------------------------------------------------------------------
    //
    // ����������δ��������ڴ��ʼ��EPT
    //
    CpuInfo->EptInfo->EptPml4TablePointer = InitEptIdentityMap( CpuInfo->EptInfo );

    if ( CpuInfo->EptInfo->EptPml4TablePointer == NULL )
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

//---------------------------------------------------------------------------
__Exit:

    if( !NT_SUCCESS( Status ) )
    {
        if ( CpuInfo->Self )
        {
            ExFreePoolWithTag( CpuInfo->Self, 'Joen' );
            CpuInfo->Self = NULL;
        }

        if( CpuInfo->VmxonVirtualAddress )
        {
            MmFreeNonCachedMemory( CpuInfo->VmxonVirtualAddress, 0x1000 );
            CpuInfo->VmxonVirtualAddress = NULL;

        }

        if( CpuInfo->VmcsVirtualAddress )
        {
            MmFreeNonCachedMemory( CpuInfo->VmcsVirtualAddress, 0x1000 );
            CpuInfo->VmcsVirtualAddress = NULL;
        }

        if( CpuInfo->IOBitmapVirtualAddressA )
        {
            MmFreeNonCachedMemory( CpuInfo->IOBitmapVirtualAddressA, 0x1000 );
            CpuInfo->IOBitmapVirtualAddressA = NULL;
        }

        if( CpuInfo->IOBitmapVirtualAddressB )
        {
            MmFreeNonCachedMemory( CpuInfo->IOBitmapVirtualAddressB, 0x1000 );
            CpuInfo->IOBitmapVirtualAddressB = NULL;
        }

        if( CpuInfo->MSRBitmapVirtualAddress )
        {
            MmFreeNonCachedMemory( CpuInfo->MSRBitmapVirtualAddress, 0x1000 );
            CpuInfo->MSRBitmapVirtualAddress = NULL;
        }

        if( CpuInfo->HostEsp )
        {
            ExFreePool( CpuInfo->HostEsp );
            CpuInfo->HostEsp = NULL;
        }

        if( CpuInfo->EptInfo )
        {
            ExFreePool( CpuInfo->EptInfo );
            CpuInfo->EptInfo = NULL;
        }
    }

    return Status;
}


//
// ����VTʱ����дVMCS�ṹ
//
NTSTATUS
SetupVMCS(
    P_CPU_VM_CONTEXT CpuInfo,
    PVOID GuestEsp
)
{
    ULONG64	Msr;
    ULONG_PTR Cr0;
    ULONG GdtBase;
    ULONG ExceptionBitmap;
    EPTP EptPointer = {0};
    ULONG32 Interceptions;
    PHYSICAL_ADDRESS PhysicalAddress;

    if( !CpuInfo || !CpuInfo->VmcsVirtualAddress )
    {
        return STATUS_INVALID_PARAMETER;
    }

    PhysicalAddress = CpuInfo->VmcsPyhicalAddress;

    //
    // ִ�� Vmxclean �����, �� vmx_ptrld �󶨵���ǰProcessor
    //
    _ExecuteVmClear( PhysicalAddress.LowPart, PhysicalAddress.HighPart );
    _ExecuteVmPtrLd( PhysicalAddress.LowPart, PhysicalAddress.HighPart );

    //
    // ���VMCS��ƫ��4��VMX�˳�ԭ��ָʾ��(VMX-abort Indicator )
    //
    RtlZeroMemory( ( PULONG )CpuInfo->VmcsVirtualAddress + 1, 4 );
//---------------------------------------------------------------------------
    /*
     * �����ǰ���Intel �ֲ�������VMCS����������ɲ���������д
     * VMCS�������ܹ�6����ɲ���. �ֱ���
     *
     * 1.�ͻ���״̬��(Guest State Area)
     *
     * 2.������״̬��(Host State Area )
     *
     * 3.�����ִ�п�����( VM-Execuction Control Fields )
     *
     * 4.VMEntry��Ϊ������( VM-Entry Control Fields )
     *
     * 5.VMExit��Ϊ������( VM-Exit Control Fields )
     *
     * 6.VMExit�����Ϣ��( VM-Exit Information Fields )(ֻ��)
     */
//---------------------------------------------------------------------------
    //
    // ��д1.�ͻ���״̬��(Guest State Area)
    //

    // ��дGuest ���ƼĴ���״̬
    Cr0 = _GetCr0();
    CmSetBit32( &Cr0, 0 );	// PE
    CmSetBit32( &Cr0, 5 );	// NE
    CmSetBit32( &Cr0, 31 );	// PG
    _WriteVMCS( GUEST_CR0, Cr0 );
    _WriteVMCS( GUEST_CR3, _GetCr3() );
    _WriteVMCS( GUEST_CR4, _GetCr4() );

    // ��дDR7�Ĵ���
    _WriteVMCS( GUEST_DR7, 0x400 );

    // ��дGuest Esp, Eip, Eflags
    _WriteVMCS( GUEST_ESP, ( ULONG ) GuestEsp );

    //
    // GUEST_EIP ָ��ĵ�ַ, ����VT�Ժ�ʹ����￪ʼ���е�
    //
    _WriteVMCS( GUEST_EIP, ( ULONG )&_GuestEntryPoint );

    KdPrint( ( "SW-> Guest Entry Point:%p\n", &_GuestEntryPoint ) );

    _WriteVMCS( GUEST_RFLAGS, _GetEflags() );

    // ��дGuest������ѡ���� CS, SS, DS, ES, FS, GS, LDTR, TR
    _WriteVMCS( GUEST_CS_SELECTOR, _GetCsSelector() & 0xfff8 );
    _WriteVMCS( GUEST_SS_SELECTOR, _GetSsSelector() & 0xfff8 );
    _WriteVMCS( GUEST_DS_SELECTOR, _GetDsSelector() & 0xfff8 );
    _WriteVMCS( GUEST_ES_SELECTOR, _GetEsSelector() & 0xfff8 );
    _WriteVMCS( GUEST_FS_SELECTOR, _GetFsSelector() & 0xfff8 );
    _WriteVMCS( GUEST_GS_SELECTOR, _GetGsSelector() & 0xfff8 );
    _WriteVMCS( GUEST_LDTR_SELECTOR, _GetLdtrSelector() & 0xfff8 );
    _WriteVMCS( GUEST_TR_SELECTOR, _GetTrSelector() & 0xfff8 );

    // ��дGuest����������ַ
    GdtBase = _GetGdtBase();
    _WriteVMCS( GUEST_CS_BASE, GetSegmentDescriptorBase( GdtBase, _GetCsSelector() ) );
    _WriteVMCS( GUEST_SS_BASE, GetSegmentDescriptorBase( GdtBase, _GetSsSelector() ) );
    _WriteVMCS( GUEST_DS_BASE, GetSegmentDescriptorBase( GdtBase, _GetDsSelector() ) );
    _WriteVMCS( GUEST_ES_BASE, GetSegmentDescriptorBase( GdtBase, _GetEsSelector() ) );
    _WriteVMCS( GUEST_FS_BASE, GetSegmentDescriptorBase( GdtBase, _GetFsSelector() ) );
    _WriteVMCS( GUEST_GS_BASE, GetSegmentDescriptorBase( GdtBase, _GetGsSelector() ) );
    _WriteVMCS( GUEST_LDTR_BASE, GetSegmentDescriptorBase( GdtBase, _GetLdtrSelector() ) );
    _WriteVMCS( GUEST_TR_BASE, GetSegmentDescriptorBase( GdtBase, _GetTrSelector() ) );

    // ��дGuest���������޳�
    _WriteVMCS( GUEST_CS_LIMIT, GetSegmentDescriptorLimit( GdtBase, _GetCsSelector() ) );
    _WriteVMCS( GUEST_SS_LIMIT, GetSegmentDescriptorLimit( GdtBase, _GetSsSelector() ) );
    _WriteVMCS( GUEST_DS_LIMIT, GetSegmentDescriptorLimit( GdtBase, _GetDsSelector() ) );
    _WriteVMCS( GUEST_ES_LIMIT, GetSegmentDescriptorLimit( GdtBase, _GetEsSelector() ) );
    _WriteVMCS( GUEST_FS_LIMIT, GetSegmentDescriptorLimit( GdtBase, _GetFsSelector() ) );
    _WriteVMCS( GUEST_GS_LIMIT, GetSegmentDescriptorLimit( GdtBase, _GetGsSelector() ) );
    _WriteVMCS( GUEST_LDTR_LIMIT, GetSegmentDescriptorLimit( GdtBase, _GetLdtrSelector() ) );
    _WriteVMCS( GUEST_TR_LIMIT, GetSegmentDescriptorLimit( GdtBase, _GetTrSelector() ) );

    // ��дGuest������������
    _WriteVMCS( GUEST_CS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, _GetCsSelector() ) );
    _WriteVMCS( GUEST_SS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, _GetSsSelector() ) );
    _WriteVMCS( GUEST_DS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, _GetDsSelector() ) );
    _WriteVMCS( GUEST_ES_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, _GetEsSelector() ) );
    _WriteVMCS( GUEST_FS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, _GetFsSelector() ) );
    _WriteVMCS( GUEST_GS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, _GetGsSelector() ) );
    _WriteVMCS( GUEST_LDTR_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, _GetLdtrSelector() ) );
    _WriteVMCS( GUEST_TR_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, _GetTrSelector() ) );

    // ��дGuest GDTR/IDTR��ַ
    _WriteVMCS( GUEST_GDTR_BASE, _GetGdtBase() );
    _WriteVMCS( GUEST_IDTR_BASE, _GetIdtBase() );

    // ��дGuest GDTR/IDTR �޳�
    _WriteVMCS( GUEST_GDTR_LIMIT, _GetGdtLimit() );
    _WriteVMCS( GUEST_IDTR_LIMIT, _GetIdtLimit() );

    // ��дGuest MSR IA32_DEBUGCTL �Ĵ���ֵ
    Msr = _ReadMsr( MSR_IA32_DEBUGCTL );
    _WriteVMCS( GUEST_IA32_DEBUGCTL, ( ULONG )( Msr & 0xFFFFFFFF ) );
    _WriteVMCS( GUEST_IA32_DEBUGCTL_HIGH, ( ULONG )( Msr >> 32 ) );

    // ��дGuest MSR IA32_SYSENTER_CS
    Msr = _ReadMsr( MSR_IA32_SYSENTER_CS );
    _WriteVMCS( GUEST_SYSENTER_CS, ( ULONG )( Msr & 0xFFFFFFFF ) );

    // ��дGuest MSR IA32_SYSENTER_ESP
    Msr = _ReadMsr( MSR_IA32_SYSENTER_ESP );
    _WriteVMCS( GUEST_SYSENTER_ESP, ( ULONG )( Msr & 0xFFFFFFFF ) );

    // ��дGuest MSR IA32_SYSENTER_EIP
    Msr = ( ULONG64 )_ReadMsr( MSR_IA32_SYSENTER_EIP );

    _WriteVMCS( GUEST_SYSENTER_EIP, ( ULONG )( Msr & 0xFFFFFFFF ) );

    //
    // ��Ȼ����������Է��� IA32_PERF_GLOBAL_CTRL(���ܼ���), IA32_PAT,
    // IA32_EFER(����64λҪ��) �⼸��MSR�Ĵ���û����д.
    //
//===========================================================================

    //
    // ��д2.������״̬��(Host State Area )
    //

    // Guest���״̬
    _WriteVMCS( GUEST_ACTIVITY_STATE, 0 );

    // Guest�ж�����״̬
    _WriteVMCS( GUEST_INTERRUPTIBILITY_INFO, 0 );

    // VMCS����ָ��(����)
    _WriteVMCS( VMCS_LINK_POINTER, 0xFFFFFFFF );
    _WriteVMCS( VMCS_LINK_POINTER_HIGH, 0xFFFFFFFF );

    //
    // �����Ƴٵ����쳣, VMX��ռ��ʱ��, PDPTE�û����д
    //
//===========================================================================

    //
    // ��д2.������״̬��(Host State Area )
    //

    // ���ƼĴ��� CR0, CR3, CR4
    _WriteVMCS( HOST_CR0, _GetCr0() );
    _WriteVMCS( HOST_CR3, _GetCr3() );
    _WriteVMCS( HOST_CR4, _GetCr4() );

    // ����Host ESP, EIP
    _WriteVMCS( HOST_ESP, ( ULONG )( ( PUCHAR ) CpuInfo->HostEsp + 0x7000 ) );

    //
    // ���ֵ�ǳ���Ҫ��. ����#VMExit�¼���ʱ��, ��������
    //
    _WriteVMCS( HOST_EIP, ( ULONG )&_ExitEventHandler );
    KdPrint( ( "SW-> VmExit EntryPoint:%p\n", &_ExitEventHandler ) );


    // ��дHost CS, SS, DS, ES, FS, GS��TR�Ĵ���
    _WriteVMCS( HOST_CS_SELECTOR, _GetCsSelector() & 0xfff8 );
    _WriteVMCS( HOST_SS_SELECTOR, _GetSsSelector() & 0xfff8 );
    _WriteVMCS( HOST_DS_SELECTOR, _GetDsSelector() & 0xfff8 );
    _WriteVMCS( HOST_ES_SELECTOR, _GetEsSelector() & 0xfff8 );
    _WriteVMCS( HOST_FS_SELECTOR, _GetFsSelector() & 0xfff8 );
    _WriteVMCS( HOST_GS_SELECTOR, _GetGsSelector() & 0xfff8 );
    _WriteVMCS( HOST_TR_SELECTOR, _GetTrSelector() & 0xfff8 );

    // ��д Host FS, GS and TR ��ַ
    _WriteVMCS( HOST_FS_BASE, GetSegmentDescriptorBase( GdtBase, _GetFsSelector() ) );
    _WriteVMCS( HOST_GS_BASE, GetSegmentDescriptorBase( GdtBase, _GetGsSelector() ) );
    _WriteVMCS( HOST_TR_BASE, GetSegmentDescriptorBase( GdtBase, _GetTrSelector() ) );


    // ��дHost GDTR/IDTR base
    _WriteVMCS( HOST_GDTR_BASE, _GetGdtBase() );
    _WriteVMCS( HOST_IDTR_BASE, _GetIdtBase() );

    // ��ʼ�� Guest �� Idt
    // InitVMMIDT( CpuInfo, ( PIDT_ENTRY )CpuInfo->HostIdtArea );

    // ��д Host IA32_SYSENTER_ESP/EIP/CS
    Msr = _ReadMsr( MSR_IA32_SYSENTER_ESP );
    _WriteVMCS( HOST_IA32_SYSENTER_ESP, ( ULONG )( Msr & 0xFFFFFFFF ) );

    Msr = _ReadMsr( MSR_IA32_SYSENTER_EIP );
    _WriteVMCS( HOST_IA32_SYSENTER_EIP, ( ULONG )( Msr & 0xFFFFFFFF ) );

    Msr = _ReadMsr( MSR_IA32_SYSENTER_CS );
    _WriteVMCS( HOST_IA32_SYSENTER_CS, ( ULONG )( Msr & 0xFFFFFFFF ) );

    //
    // IA32_PERF_GLOBAL_CTRL, IA32_PAT, IA32_EFER û����д
    //
//===========================================================================
    //
    // ��д 3.�����ִ�п�����( VM-Execuction Control Fields )
    //

    //
    // ��д 3.1 ������ŵ������ִ�п��� Pin-based VM-execution controls
    //
    _WriteVMCS( PIN_BASED_VM_EXEC_CONTROL,
                VmxAdjustControls( 0, MSR_IA32_VMX_PINBASED_CTLS ) );

    //
    // ��д 3.2 ���ڴ������������ִ�п��� Primary processor-based VM-execution controls
    // ����������쳣λͼ, DR�Ĵ�������, ����MSR�����Ƿ����#VMExit. �ȵ�
    //

    Interceptions = VmxAdjustControls( 0, MSR_IA32_VMX_PROCBASED_CTLS );
    Interceptions |= CPU_BASED_ACTIVATE_MSR_BITMAP;	// MSR�Ĵ�������
    Interceptions |= CPU_BASED_USE_IO_BITMAPS;		// IO�Ĵ�������
    /*	Interceptions |=  CPU_BASED_MOV_DR_EXITING;		// DR�Ĵ������� */
    Interceptions &= ~CPU_BASED_CR3_LOAD_EXITING;	// �ر�����CR3�Ĵ���
    Interceptions &= ~CPU_BASED_CR3_STORE_EXITING;	// �ر�����CR3�Ĵ���
    _WriteVMCS( CPU_BASED_VM_EXEC_CONTROL, Interceptions );

    //
    // ��д 3.3 Exception bitmap �����쳣λͼ
    //
    ExceptionBitmap = 0;
    // ExceptionBitmap |= 1 << DEBUG_EXCEPTION;
    // ExceptionBitmap |= 1 << BREAKPOINT_EXCEPTION;
    // ExceptionBitmap |= 1 << PAGE_FAULT_EXCEPTION;

    _WriteVMCS( EXCEPTION_BITMAP, ExceptionBitmap );

    // ����ҳ����
    _WriteVMCS( PAGE_FAULT_ERROR_CODE_MASK, 0 );
    _WriteVMCS( PAGE_FAULT_ERROR_CODE_MATCH, 0 );

    //
    // ��д 3.4 I/O bitmap IOλͼ, �������������60�Ŷ˿�.. Ҳ���Ǽ�������
    //
    _WriteVMCS( IO_BITMAP_A_HIGH, CpuInfo->IOBitmapPyhicalAddressA.HighPart );
    _WriteVMCS( IO_BITMAP_A,      CpuInfo->IOBitmapPyhicalAddressA.LowPart );
    _WriteVMCS( IO_BITMAP_B_HIGH, CpuInfo->IOBitmapPyhicalAddressB.HighPart );
    _WriteVMCS( IO_BITMAP_B,      CpuInfo->IOBitmapPyhicalAddressB.LowPart );

    // ioPort = 0x60;

    // ( ( PUCHAR )( CpuInfo->pIOBitmapVirtualAddressA ) )[ioPort / 8] = 1 << ( ioPort % 8 );

    // ��д 3.5 ʱ���������ƫ��(Time-Stamp Counter Offset )
    _WriteVMCS( TSC_OFFSET, 0 );
    _WriteVMCS( TSC_OFFSET_HIGH, 0 );

    // ��д 3.6 �����/Hypervisor���κ� CR0/CR4������������, û����

    // ��д 3.7 CR3���ʿ���.
    _WriteVMCS( CR3_TARGET_COUNT, 0 );
    _WriteVMCS( CR3_TARGET_VALUE0, 0 );
    _WriteVMCS( CR3_TARGET_VALUE1, 0 );
    _WriteVMCS( CR3_TARGET_VALUE2, 0 );
    _WriteVMCS( CR3_TARGET_VALUE3, 0 );

    // ��д 3.8 APIC���ʿ���,û����


    //
    // ��д3.9 MSR λͼ��ַ(MSR Bitmap Address )
    // ͨ����MSR Bitmap��1��������Ҫ���ص�MSR�Ĵ�������, ��ǰ��д��0��ʾ����Ҫ����
    //
    RtlFillMemory( CpuInfo->MSRBitmapVirtualAddress , 0x4000, 0 );

    _WriteVMCS( MSR_BITMAP, CpuInfo->MSRBitmapPyhicalAddress.LowPart );
    _WriteVMCS( MSR_BITMAP_HIGH, CpuInfo->MSRBitmapPyhicalAddress.HighPart );

    // 3.10 ִ����VMCSָ��(Executive-VMCS Pointer ), û����

//===========================================================================
    //
    // 3.11 ����EPTָ��( Extended Page Table Pointer )
    //
    Msr = _ReadVMCS( SECONDARY_VM_EXEC_CONTROL );

    PhysicalAddress = MmGetPhysicalAddress( ( void * ) CpuInfo->EptInfo->EptPml4TablePointer );

    // ����EPTָ��
    EptPointer.Bits.PhysAddr = PhysicalAddress.LowPart >> 12;
    EptPointer.Bits.PageWalkLength = 3;
    _WriteVMCS( EPT_POINTER, EptPointer.unsignedVal & 0xFFFFFFFF );
    _WriteVMCS( EPT_POINTER_HIGH, 0 );

    // Set the guest VPID to a non-zero value
    _WriteVMCS( VIRTUAL_PROCESSOR_ID, VM_VPID );


    // 1����EPT, bit 5��1����VPID����
    _WriteVMCS( SECONDARY_VM_EXEC_CONTROL, ( ULONG )Msr | ( 1 << 5 ) | ( 1 << 1 ) );

    Msr = _ReadVMCS( CPU_BASED_VM_EXEC_CONTROL );

    //
    // CPU_BASED_VM_EXEC_CONTROL�ֶ�32λ��1��ָʾSECONDARY_VM_EXEC_CONTROL�ֶε�ֵΪ��Чֵ
    //
    _WriteVMCS( CPU_BASED_VM_EXEC_CONTROL, ( ULONG )Msr | ( 1 << 31 ) );

    // 3.12 �������ʶ��(Virtual Processor Identifier, VPID), û����
//===========================================================================
    //
    // 4.VMEntry��Ϊ������( VM-Entry Control Fields )
    //

    _WriteVMCS( VM_ENTRY_CONTROLS, VmxAdjustControls( 0, MSR_IA32_VMX_ENTRY_CTLS ) );

    // ��д#VMEntryʱ�洢����MSR�Ĵ�������
    _WriteVMCS( VM_ENTRY_MSR_LOAD_COUNT, 0 );
    _WriteVMCS( VM_ENTRY_INTR_INFO_FIELD, 0 );

//===========================================================================
    //
    // 5.VMExit��Ϊ������( VM-Exit Control Fields )
    //
    _WriteVMCS( VM_EXIT_CONTROLS,
                VmxAdjustControls( VM_EXIT_ACK_INTR_ON_EXIT, MSR_IA32_VMX_EXIT_CTLS ) );

    // ��д#VmExitʱ�洢����MSR�Ĵ�������
    _WriteVMCS( VM_EXIT_MSR_STORE_COUNT, 0 );
    _WriteVMCS( VM_EXIT_MSR_LOAD_COUNT, 0 );

    //
    // ������Ȼ�кܶ���Ϣû���õ�
    //
//---------------------------------------------------------------------------

    return STATUS_SUCCESS;
}

//
// �������⻯
//
NTSTATUS
LaunchVirtualize(
    P_CPU_VM_CONTEXT CpuInfo
)
{
    //
    // VMLaunch��Ӧ�÷���, �ɹ��Ļ�Ӧ�ý���VMCS�е�Guest Eip��
    //
    _ExecuteVmLaunch();

    if( _VmFailInvalid() )
    {
        KdPrint( ( "SW-> VMLaunch failed\n" ) );
        return STATUS_UNSUCCESSFUL;
    }

    if( _VmLaunchFailValid() )
    {
        KdPrint( ( "SW-> VMLaunch failed Error Code: %p\n", _ReadVMCS( VM_INSTRUCTION_ERROR ) ) );
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_UNSUCCESSFUL;
}

//
//	����Hyervisor ��������GuestEsp
//
NTSTATUS
NTAPI
LoadHypervisor(
    PVOID GuestEsp
)
{
    NTSTATUS Status;
    P_CPU_VM_CONTEXT CpuInfo;
    PHYSICAL_ADDRESS Highest = {0};
    PHYSICAL_ADDRESS Lowest = {0};

    Highest.LowPart = ~0;


    //
    // ΪProcessor����ṹ, ����ṹ�Ǻ���Ҫ��.
    //
    CpuInfo = ( P_CPU_VM_CONTEXT )MmAllocateContiguousMemorySpecifyCache(
                  sizeof( CPU_VM_CONTEXT ), Lowest,Highest,Lowest,0 );

    if( !CpuInfo )
    {
        KdPrint( ( "SW-> Cpu Context allocate memroy failed\n" ) );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( CpuInfo, sizeof( CPU_VM_CONTEXT ) );

    KdPrint( ( "SW-> Vmx Cpu Address  :%p\n", CpuInfo ) );

//---------------------------------------------------------------------------
    //
    // ��дProcessor ��Cpu�ṹ ��Ҫ�Ƿ����ڴ�Vmx�ṹ, ����VMCS. IOλͼ,
    // VMCS.MSRλͼ VMCS.Host Stack, VMCS.IDTλͼ.
    //
    Status = SetupVMX( CpuInfo );

    if( Status == STATUS_UNSUCCESSFUL )
    {
        MmFreeContiguousMemory( CpuInfo );
        return STATUS_UNSUCCESSFUL;
    }

//---------------------------------------------------------------------------
    //
    // ��дProcessor ��Cpu�ṹ��Ҫ����дVMCS�ṹ.
    //
    Status = SetupVMCS( CpuInfo, GuestEsp );

    if( Status == STATUS_UNSUCCESSFUL )
    {
        MmFreeContiguousMemory( CpuInfo );
        return STATUS_UNSUCCESSFUL;
    }

    //
    // �� VIRT_CPU �ṹ�帴�Ƶ�HOST��ջ��ȥ
    //
    RtlMoveMemory( ( PUCHAR )CpuInfo->HostEsp + 0x7000, CpuInfo, sizeof( CPU_VM_CONTEXT ) );

    //
    // �������⻯. �����������ʱ�Ѿ�����vt no-rootģʽ
    //
    Status = LaunchVirtualize( CpuInfo );

    return Status;
}

//
// �ر�Hypervisor
//
NTSTATUS NTAPI UnLoadHypervisor( )
{
    ULONG64 Efer;
    NTSTATUS Status;
    P_CPU_VM_CONTEXT CpuInfo = NULL;

    //
    // ��ȡCPUָ��
    //
    CpuInfo = ( P_CPU_VM_CONTEXT )_ExecuteVmcall( VMCALL_GET_ESP, 0, 0, 0 );

    //
    // �ر�EPT
    //
    if ( CpuInfo->EptInfo->EptPml4TablePointer != NULL )
    {
        UINT reg = _ReadVMCS( SECONDARY_VM_EXEC_CONTROL );
        _WriteVMCS( SECONDARY_VM_EXEC_CONTROL, reg & ~( ( 1 << 5 ) | ( 1 << 1 ) ) );

        _WriteVMCS( EPT_POINTER, 0 );
        _WriteVMCS( EPT_POINTER_HIGH, 0 );
    }

    //
    // �ر�Hypervisor
    //
    if( !NT_SUCCESS( Status = _MakeHyperExitCall( ) ) )
    {
        KdPrint( ( "SW-> _MakeHyperExitCall() failed Status %x \n", Status ) );

        return Status;
    }

    //
    // �ͷ�EPT�ڴ�
    //
    if ( CpuInfo->EptInfo->EptPml4TablePointer != NULL )
    {
        FreeEptIdentityMap( CpuInfo->EptInfo,
                            CpuInfo->EptInfo->EptPml4TablePointer );

        CpuInfo->EptInfo->EptPml4TablePointer = NULL;
    }

    if( CpuInfo->VmxonVirtualAddress )
    {
        MmFreeNonCachedMemory( CpuInfo->VmxonVirtualAddress, 0x1000 );
        CpuInfo->VmxonVirtualAddress = NULL;

    }

    if( CpuInfo->VmcsVirtualAddress )
    {
        MmFreeNonCachedMemory( CpuInfo->VmcsVirtualAddress, 0x1000 );
        CpuInfo->VmcsVirtualAddress = NULL;
    }

    if( CpuInfo->IOBitmapVirtualAddressA )
    {
        MmFreeNonCachedMemory( CpuInfo->IOBitmapVirtualAddressA, 0x1000 );
        CpuInfo->IOBitmapVirtualAddressA = NULL;
    }

    if( CpuInfo->IOBitmapVirtualAddressB )
    {
        MmFreeNonCachedMemory( CpuInfo->IOBitmapVirtualAddressB, 0x1000 );
        CpuInfo->IOBitmapVirtualAddressB = NULL;
    }

    if( CpuInfo->MSRBitmapVirtualAddress )
    {
        MmFreeNonCachedMemory( CpuInfo->MSRBitmapVirtualAddress, 0x1000 );
        CpuInfo->MSRBitmapVirtualAddress = NULL;
    }

    if( CpuInfo->HostEsp )
    {
        ExFreePoolWithTag( CpuInfo->HostEsp, 'Esp' );
        CpuInfo->HostEsp = NULL;
    }

    if ( CpuInfo->Self )
    {
        MmFreeContiguousMemory( CpuInfo->Self );
        CpuInfo->Self = NULL;
    }

    if( CpuInfo->EptInfo )
    {
        ExFreePool( CpuInfo->EptInfo );
        CpuInfo->EptInfo = NULL;
    }

    return STATUS_SUCCESS;
}


//
// ���ϵͳ����VT֧�����
//
NTSTATUS CheckVtSupport()
{
    int i;
    ULONG cr4;
    ULONG64 msr;
    ULONG CpuAffinity;
    ULONG ulNumberOfActiveCpu;
    ULONG eax, ebx, ecx, edx;
    IA32_VMX_PROCBASED_CTLS_MSR Ctls;
    IA32_VMX_PROCBASED_CTLS2_MSR Ctls2;

    //
    // vmx supported by cpu ?
    //
    _ExecuteCpuId( 0, &eax, &ebx, &ecx, &edx );

    if ( eax < 1 )
    {
        KdPrint( ( "SW-> extended CPUID functions not implemented\n" ) );
        return STATUS_UNSUCCESSFUL;
    }

    /* Intel Genuine */
    if ( !( ebx == 0x756e6547 && ecx == 0x6c65746e && edx == 0x49656e69 ) )
    {
        KdPrint( ( "SW-> not an INTEL processor\n" ) );
        return STATUS_UNSUCCESSFUL;
    }

    //
    // vmx supported if bit 5 equals 1
    //
    _ExecuteCpuId( 0x1, &eax, &ebx, &ecx, &edx );

    if ( !( ecx & 0x20 ) )
    {
        KdPrint( ( "SW-> VMX not supported\n" ) );
        return STATUS_UNSUCCESSFUL;
    }

    //
    // ���BIOS�Ƿ���VT
    //
    msr = _ReadMsr( MSR_IA32_FEATURE_CONTROL );

    if( !( msr & 4 ) )
    {
        KdPrint ( ( "SW-> VMX is not supported: IA32_FEATURE_CONTROL is %llx\n", msr ) );
        return STATUS_UNSUCCESSFUL;
    }

    //
    // ���cpu�Ƿ�֧��Secondary ProcBased Control. EPT��������
    //
    *( ULONG64* )&Ctls = _ReadMsr( MSR_IA32_VMX_PROCBASED_CTLS );

    if( Ctls.SControls != 1  )
    {
        KdPrint ( ( "SW-> Not Secondary ProcBased Control Supported: %llx\n", *( ULONG64* )&Ctls ) );
        return STATUS_UNSUCCESSFUL;
    }

    //
    // ���cpu�Ƿ�֧��EPT
    //
    *( ULONG64* )&Ctls2 = _ReadMsr( MSR_IA32_VMX_PROCBASED_CTLS2 );

    if( (  Ctls2.EnableEpt == 0 || Ctls2.EnableVpid == 0 ) )
    {
        KdPrint ( ( "SW-> Not VPID/EPT Supported %llx\n", *( ULONG64* )&Ctls2 ) );
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Check if PAE is enabled.
    //
    cr4 = _GetCr4();

    if( cr4 & 0x00000020 )
    {
        KdPrint ( ( "SW-> PAE must be disabled Cr4: %x\n", cr4 ) );
        KdPrint ( ( "SW-> bcdedit /set pae forcedisable\n" ) );
		KdPrint ( ( "SW-> bcdedit /set nx alwaysoff\n" ) );

        return STATUS_UNSUCCESSFUL;
    }

    //
    // We do not support multi-core CPUs
    //
    CpuAffinity = KeQueryActiveProcessors();
    ulNumberOfActiveCpu = 0;

    for ( i = 0; i < 32; ++i )
    {
        if ( ( CpuAffinity >> i ) & 1 )
            ++ulNumberOfActiveCpu;
    }

    if ( ulNumberOfActiveCpu > 1 )
    {
        KdPrint ( ( "SW-> do not support multi-core CPUs\n" ) );
        return STATUS_UNSUCCESSFUL;
    }

    //
    // ����CR4 X86_CR4_VMXE Ϊ����VMģʽ��׼��
    //
    _SetCr4( _GetCr4() | X86_CR4_VMXE );
    cr4 = _GetCr4();

    if( !( cr4 & X86_CR4_VMXE ) )
    {
        KdPrint ( ( "SW-> VMX is not supported Cr4: %x\n", cr4 ) );
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}



