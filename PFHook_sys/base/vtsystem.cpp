#include "header.h"
#include "vtsystem.h"
#include "vtasm.h"
#include "exithandler.h"
#include "common.h"
#include "ept.h"

VMX_CPU g_VMXCPU[128];

KMUTEX g_GlobalMutex;
EptPml4Entry* g_Pml4;
NTSTATUS AllocateVMXRegion()
{
	PVOID pVMXONRegion;
	PVOID pVMCSRegion;
	PVOID pHostEsp;
	ULONG64 uCPUID;

	uCPUID = KeGetCurrentProcessorNumber();
	pVMXONRegion = ExAllocatePoolWithTag(NonPagedPool,0x1000,'vmon'); //4KB
	if (!pVMXONRegion)
	{
		return STATUS_MEMORY_NOT_ALLOCATED;
	}
	RtlZeroMemory(pVMXONRegion,0x1000);

	pVMCSRegion = ExAllocatePoolWithTag(NonPagedPool,0x1000,'vmcs');
	if (!pVMCSRegion)
	{
		ExFreePoolWithTag(pVMXONRegion,0x1000);
		return STATUS_MEMORY_NOT_ALLOCATED;
	}
	RtlZeroMemory(pVMCSRegion,0x1000);

	pHostEsp = ExAllocatePoolWithTag(NonPagedPool,0x2000,'mini');
	if (!pHostEsp)
	{
		ExFreePoolWithTag(pVMXONRegion,0x1000);
		ExFreePoolWithTag(pVMCSRegion,0x1000);
		return STATUS_MEMORY_NOT_ALLOCATED;
	}
	RtlZeroMemory(pHostEsp,0x2000);

	g_VMXCPU[uCPUID].pVMXONRegion = pVMXONRegion;
	g_VMXCPU[uCPUID].pVMXONRegion_PA = MmGetPhysicalAddress(pVMXONRegion);
	g_VMXCPU[uCPUID].pVMCSRegion = pVMCSRegion;
	g_VMXCPU[uCPUID].pVMCSRegion_PA = MmGetPhysicalAddress(pVMCSRegion);
	g_VMXCPU[uCPUID].pHostEsp = pHostEsp;
	g_VMXCPU[uCPUID].ept_PML4T = g_Pml4;
	return STATUS_SUCCESS;
}

void SetupVMXRegion()
{
	VMX_BASIC_MSR Msr;
	ULONG uRevId;
	_CR4 uCr4;
	_EFLAGS uEflags;
	ULONG64 uCPUID;

	uCPUID = KeGetCurrentProcessorNumber();

	RtlZeroMemory(&Msr,sizeof(Msr));

	*((PULONG64)&Msr) = Asm_ReadMsr(MSR_IA32_VMX_BASIC);
	uRevId = Msr.RevId;

	*((PULONG)g_VMXCPU[uCPUID].pVMXONRegion) = uRevId;
	*((PULONG)g_VMXCPU[uCPUID].pVMCSRegion) = uRevId;

	*((PULONG64)&uCr4) = Asm_GetCr4();
	uCr4.VMXE = 1;
	Asm_SetCr4(*((PULONG64)&uCr4));

	Vmx_VmxOn(g_VMXCPU[uCPUID].pVMXONRegion_PA.QuadPart);
	*((PULONG64)&uEflags) = Asm_GetRflags();
	if (uEflags.CF != 0)
	{
		return;
	}
}

extern "C" void SetupVMCS()
{
	_EFLAGS uEflags;
	ULONG64 GdtBase,IdtBase;
	SEGMENT_SELECTOR SegmentSelector;
	ULONG64 uCPUBase;
	ULONG64 uCPUID;
	ULONG64 uTemp64;

	uCPUID = KeGetCurrentProcessorNumber();

	Vmx_VmClear(g_VMXCPU[uCPUID].pVMCSRegion_PA.QuadPart);
	*((PULONG64)&uEflags) = Asm_GetRflags();
	if (uEflags.CF != 0 || uEflags.ZF != 0)
	{
		return;
	}
	Vmx_VmPtrld(g_VMXCPU[uCPUID].pVMCSRegion_PA.QuadPart);

	GdtBase = Asm_GetGdtBase();
	IdtBase = Asm_GetIdtBase();

	//
	// 1.Guest State Area
	//
	Vmx_VmWrite(GUEST_CR0,Asm_GetCr0());
	Vmx_VmWrite(GUEST_CR3,Asm_GetCr3());
	Vmx_VmWrite(GUEST_CR4,Asm_GetCr4());

	Vmx_VmWrite(GUEST_DR7,0x400);
	Vmx_VmWrite(GUEST_RFLAGS,Asm_GetRflags());

	FillGuestSelectorData(GdtBase,ES,Asm_GetEs());
	FillGuestSelectorData(GdtBase,FS,Asm_GetFs());
	FillGuestSelectorData(GdtBase,DS,Asm_GetDs());
	FillGuestSelectorData(GdtBase,CS,Asm_GetCs());
	FillGuestSelectorData(GdtBase,SS,Asm_GetSs());
	FillGuestSelectorData(GdtBase,GS,Asm_GetGs());
	FillGuestSelectorData(GdtBase,TR,Asm_GetTr());
	FillGuestSelectorData(GdtBase,LDTR,Asm_GetLdtr());

	Vmx_VmWrite(GUEST_CS_BASE,0);
	Vmx_VmWrite(GUEST_DS_BASE,0);
	Vmx_VmWrite(GUEST_ES_BASE,0);
	Vmx_VmWrite(GUEST_SS_BASE,0);
	Vmx_VmWrite(GUEST_FS_BASE,Asm_ReadMsr(MSR_FS_BASE));
	Vmx_VmWrite(GUEST_GS_BASE,Asm_ReadMsr(MSR_GS_BASE));
	Vmx_VmWrite(GUEST_GDTR_BASE,GdtBase);
	Vmx_VmWrite(GUEST_GDTR_LIMIT,Asm_GetGdtLimit());
	Vmx_VmWrite(GUEST_IDTR_BASE,IdtBase);
	Vmx_VmWrite(GUEST_IDTR_LIMIT,Asm_GetIdtLimit());

	Vmx_VmWrite(GUEST_IA32_DEBUGCTL,Asm_ReadMsr(MSR_IA32_DEBUGCTL));
	Vmx_VmWrite(GUEST_IA32_DEBUGCTL_HIGH,Asm_ReadMsr(MSR_IA32_DEBUGCTL)>>32);
	Vmx_VmWrite(GUEST_IA32_EFER,Asm_ReadMsr(MSR_EFER));

	Vmx_VmWrite(GUEST_SYSENTER_CS,Asm_ReadMsr(MSR_IA32_SYSENTER_CS));
	Vmx_VmWrite(GUEST_SYSENTER_ESP,Asm_ReadMsr(MSR_IA32_SYSENTER_ESP));
	Vmx_VmWrite(GUEST_SYSENTER_EIP,Asm_ReadMsr(MSR_IA32_SYSENTER_EIP)); // KiFastCallEntry

	Vmx_VmWrite(GUEST_RSP,Asm_GetGuestRSP());
	Vmx_VmWrite(GUEST_RIP,Asm_GetGuestReturn());// 指定vmlaunch客户机的入口点 这里我们让客户机继续执行加载驱动的代码

	Vmx_VmWrite(GUEST_INTERRUPTIBILITY_INFO, 0);
	Vmx_VmWrite(GUEST_ACTIVITY_STATE, 0);
	Vmx_VmWrite(VMCS_LINK_POINTER, 0xffffffff);
	Vmx_VmWrite(VMCS_LINK_POINTER_HIGH, 0xffffffff);

	//
	// 2.Host State Area
	//
	Vmx_VmWrite(HOST_CR0,Asm_GetCr0());
	Vmx_VmWrite(HOST_CR3,Asm_GetCr3());
	Vmx_VmWrite(HOST_CR4,Asm_GetCr4());

	Vmx_VmWrite(HOST_ES_SELECTOR,KGDT64_R0_DATA);
	Vmx_VmWrite(HOST_CS_SELECTOR,KGDT64_R0_CODE);
	Vmx_VmWrite(HOST_SS_SELECTOR,KGDT64_R0_DATA);
	Vmx_VmWrite(HOST_DS_SELECTOR,KGDT64_R0_DATA);
	Vmx_VmWrite(HOST_FS_SELECTOR,(Asm_GetFs()&0xF8));
	Vmx_VmWrite(HOST_GS_SELECTOR,(Asm_GetGs()&0xF8));
	Vmx_VmWrite(HOST_TR_SELECTOR,(Asm_GetTr()&0xF8));

	Vmx_VmWrite(HOST_FS_BASE,Asm_ReadMsr(MSR_FS_BASE));
	Vmx_VmWrite(HOST_GS_BASE,Asm_ReadMsr(MSR_GS_BASE));
	InitializeSegmentSelector(&SegmentSelector,Asm_GetTr(),GdtBase);
	Vmx_VmWrite(HOST_TR_BASE,SegmentSelector.base);

	Vmx_VmWrite(HOST_GDTR_BASE,GdtBase);
	Vmx_VmWrite(HOST_IDTR_BASE,IdtBase);

	Vmx_VmWrite(HOST_IA32_EFER,Asm_ReadMsr(MSR_EFER));
	Vmx_VmWrite(HOST_IA32_SYSENTER_CS,Asm_ReadMsr(MSR_IA32_SYSENTER_CS));
	Vmx_VmWrite(HOST_IA32_SYSENTER_ESP,Asm_ReadMsr(MSR_IA32_SYSENTER_ESP));
	Vmx_VmWrite(HOST_IA32_SYSENTER_EIP,Asm_ReadMsr(MSR_IA32_SYSENTER_EIP)); // KiFastCallEntry

	Vmx_VmWrite(HOST_RSP,((ULONG64)g_VMXCPU[uCPUID].pHostEsp) + 0x1FFF);//8KB 0x2000
	Vmx_VmWrite(HOST_RIP,(ULONG64)&Asm_VMMEntryPoint);//这里定义我们的VMM处理程序入口

	//
	// 3.虚拟机运行控制域
	//
	Vmx_VmWrite(PIN_BASED_VM_EXEC_CONTROL,VmxAdjustControls(0,MSR_IA32_VMX_PINBASED_CTLS));

	Vmx_VmWrite(PAGE_FAULT_ERROR_CODE_MASK,0);
	Vmx_VmWrite(PAGE_FAULT_ERROR_CODE_MATCH,0);
	Vmx_VmWrite(TSC_OFFSET,0);
	Vmx_VmWrite(TSC_OFFSET_HIGH,0);

	uCPUBase = VmxAdjustControls(0,MSR_IA32_VMX_PROCBASED_CTLS);

/*	uCPUBase &= ~CPU_BASED_ACTIVATE_MSR_BITMAP;*/
 	uCPUBase &= ~CPU_BASED_CR3_LOAD_EXITING;
 	uCPUBase &= ~CPU_BASED_CR3_STORE_EXITING;
	uCPUBase |= CPU_BASED_ACTIVATE_SECONDARY_CONTROLS;
// 	uCPUBase &= ~CPU_BASED_CR8_LOAD_EXITING;
// 	uCPUBase &= ~CPU_BASED_CR8_STORE_EXITING; // 这个不能有，不然拦截不到TF
	uCPUBase |= CPU_BASED_RDTSC_EXITING;
	//uCPUBase |= CPU_BASED_MOV_DR_EXITING; // 拦截调试寄存器操作
	//uCPUBase |= CPU_BASED_USE_IO_BITMAPS; // 拦截键盘鼠标消息
	//uCPUBase |= CPU_BASED_ACTIVATE_MSR_BITMAP; // 拦截MSR操作

	Vmx_VmWrite(CPU_BASED_VM_EXEC_CONTROL,uCPUBase);

	uTemp64 = 0;
	//这里可以指定EPT 换内存页面 搞隐藏内存用的 但我这里暂时不都需要
	uTemp64 |= SECONDARY_EXEC_RDTSCP;
	uTemp64 |= SECONDARY_EXEC_ENABLE_EPT;
	uTemp64 |= SECONDARY_EXEC_ENABLE_VPID;
	Vmx_VmWrite(SECONDARY_VM_EXEC_CONTROL, uTemp64);
	
	Vmx_VmWrite(IO_BITMAP_A,0);
	Vmx_VmWrite(IO_BITMAP_A_HIGH,0);
	Vmx_VmWrite(IO_BITMAP_B,0);
	Vmx_VmWrite(IO_BITMAP_B_HIGH,0);
	Vmx_VmWrite(MSR_BITMAP, 0);
	Vmx_VmWrite(MSR_BITMAP_HIGH, 0);

	uTemp64 = 0;
	uTemp64 |= 1 << DEBUG_EXCEPTION;
	uTemp64 |= 1 << PAGE_FAULT_EXCEPTION;

	Vmx_VmWrite(EXCEPTION_BITMAP, uTemp64);
	

	Vmx_VmWrite(CR3_TARGET_COUNT,0);
	Vmx_VmWrite(CR3_TARGET_VALUE0,0);
	Vmx_VmWrite(CR3_TARGET_VALUE1,0);
	Vmx_VmWrite(CR3_TARGET_VALUE2,0);
	Vmx_VmWrite(CR3_TARGET_VALUE3,0);

	//
	// 4.VMEntry运行控制域
	//
	Vmx_VmWrite(VM_ENTRY_CONTROLS,VmxAdjustControls(VM_ENTRY_IA32E_MODE|VM_ENTRY_LOAD_IA32_EFER,MSR_IA32_VMX_ENTRY_CTLS));
	Vmx_VmWrite(VM_ENTRY_MSR_LOAD_COUNT,0);
	Vmx_VmWrite(VM_ENTRY_INTR_INFO_FIELD,0);


	//
	// 5.VMExit运行控制域
	//
	Vmx_VmWrite(VM_EXIT_CONTROLS,VmxAdjustControls(VM_EXIT_IA32E_MODE|VM_EXIT_ACK_INTR_ON_EXIT,MSR_IA32_VMX_EXIT_CTLS));
	Vmx_VmWrite(VM_EXIT_MSR_LOAD_COUNT,0);
	Vmx_VmWrite(VM_EXIT_MSR_STORE_COUNT,0);

	EptTablePointer EPTP = { 0 };
	PHYSICAL_ADDRESS phys = MmGetPhysicalAddress((void *)g_VMXCPU[uCPUID].ept_PML4T);

	// Set up the EPTP
	EPTP.Bits.PhysAddr = phys.QuadPart >> 12;
	EPTP.Bits.MemoryType = EPT_MEMORY_TYPE_WB;
	EPTP.Bits.PageWalkLength = 3;
	EPTP.Bits.D = 1;
	Vmx_VmWrite(EPT_POINTER, EPTP.unsignedVal & 0xFFFFFFFF);
	Vmx_VmWrite(EPT_POINTER_HIGH, EPTP.unsignedVal >> 32);
	Vmx_VmWrite(VIRTUAL_PROCESSOR_ID, uCPUID + 1);

	Vmx_VmLaunch();

	g_VMXCPU[uCPUID].bVTStartSuccess = FALSE;
}

void SetupVT()
{
	ULONG64 uCPUID;

	uCPUID = KeGetCurrentProcessorNumber();
	NTSTATUS status = STATUS_SUCCESS;
	if (!IsVTEnabled())
		return;
	status = AllocateVMXRegion();
	if (!NT_SUCCESS(status))
	{
		return;
	}
	SetupVMXRegion();
	g_VMXCPU[uCPUID].bVTStartSuccess = TRUE;

	Asm_SetupVMCS();

	if (g_VMXCPU[uCPUID].bVTStartSuccess)
	{
		
	}
	else KdPrint(("VT Engine Error(%d)!\n",KeGetCurrentProcessorNumber()));
}

void UnsetupVT()
{
	_CR4 uCr4;
	ULONG64 uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	if(g_VMXCPU[uCPUID].bVTStartSuccess)
	{
		Vmx_VmCall('SVT');

		*((PULONG64)&uCr4) = Asm_GetCr4();
		uCr4.VMXE = 0;
		Asm_SetCr4(*((PULONG64)&uCr4));

		ExFreePoolWithTag(g_VMXCPU[uCPUID].pVMXONRegion,'vmon');
		ExFreePoolWithTag(g_VMXCPU[uCPUID].pVMCSRegion,'vmcs');
		ExFreePoolWithTag(g_VMXCPU[uCPUID].pHostEsp,'mini');

	}
}

NTSTATUS StartVirtualTechnology()
{
	KeInitializeMutex(&g_GlobalMutex,0);
	KeWaitForMutexObject(&g_GlobalMutex,Executive,KernelMode,FALSE,0);
	g_Pml4 = EptInitialization();

	for (int i = 0;i<KeNumberProcessors;i++)
	{
		KeSetSystemAffinityThread((KAFFINITY)(1 << i));

		SetupVT();

		KeRevertToUserAffinityThread();
	}

	KeReleaseMutex(&g_GlobalMutex, FALSE);

	KdPrint(("VT Engine has been loaded!\n"));

	return STATUS_SUCCESS;
}

NTSTATUS StopVirtualTechnology()
{
	KeInitializeMutex(&g_GlobalMutex,0);
	KeWaitForMutexObject(&g_GlobalMutex,Executive,KernelMode,FALSE,0);
	for (int i = 0;i<KeNumberProcessors;i++)
	{
		KeSetSystemAffinityThread((KAFFINITY)(1 << i));

		UnsetupVT();

		KeRevertToUserAffinityThread();
	}
	EptReleasePages();

	KeReleaseMutex(&g_GlobalMutex,FALSE);
	return STATUS_SUCCESS;
}

BOOLEAN IsVTEnabled()
{
	ULONG64 uRet_EAX,uRet_ECX,uRet_EDX,uRet_EBX;
	_CPUID_ECX uCPUID;
	_CR0 uCr0;
	_CR4 uCr4;
	IA32_FEATURE_CONTROL_MSR msr;
	//1. CPUID
	Asm_CPUID(1,&uRet_EAX,&uRet_EBX,&uRet_ECX,&uRet_EDX);
	*((PULONG64)&uCPUID) = uRet_ECX;

	if (uCPUID.VMX != 1)
	{
		return FALSE;
	}

	// 2. CR0 CR4
	*((PULONG64)&uCr0) = Asm_GetCr0();
	*((PULONG64)&uCr4) = Asm_GetCr4();

	if (uCr0.PE != 1||uCr0.PG!=1||uCr0.NE!=1)
	{
		return FALSE;
	}

	if (uCr4.VMXE == 1)
	{
		return TRUE;
	}

	// 3. MSR
	*((PULONG64)&msr) = Asm_ReadMsr(MSR_IA32_FEATURE_CONTROL);
	if (msr.Lock!=1)
	{
		return FALSE;
	}
	return TRUE;
}