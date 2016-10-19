#pragma once
#include "common.h"
#include "ept.h"
#include "global.h"
GUEST_REGS g_GuestRegs[128];
extern VMX_CPU g_VMXCPU[128];

void HandleCPUID()
{
	ULONG64 uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	if (g_GuestRegs[uCPUID].rax == 'Mini')
	{
		g_GuestRegs[uCPUID].rbx = 0x88888888;
		g_GuestRegs[uCPUID].rcx = 0x11111111;
		g_GuestRegs[uCPUID].rdx = 0x12345678;
	}
	if (g_GuestRegs[uCPUID].rax == PFHOOK_CODE_HOOK)
	{
		HandleEvent_Hook(&g_GuestRegs[uCPUID]);
	}
	else if (g_GuestRegs[uCPUID].rax == PFHOOK_CODE_UNHOOK)
	{
		HandleEvent_Unhook(&g_GuestRegs[uCPUID]);
	}
	else if (g_GuestRegs[uCPUID].rax == PFHOOK_CODE_CHECKVT)
	{
		HandleEvent_CheckVT(&g_GuestRegs[uCPUID]);
	}
	else Asm_CPUID(g_GuestRegs[uCPUID].rax,&g_GuestRegs[uCPUID].rax,&g_GuestRegs[uCPUID].rbx,&g_GuestRegs[uCPUID].rcx,&g_GuestRegs[uCPUID].rdx);
}

void HandleInvd()
{
	Asm_Invd();
}

void HandleVmCall()
{
	ULONG64 JmpEIP;
	ULONG64 uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	if (g_GuestRegs[uCPUID].rax == 'SVT')
	{
		JmpEIP = g_GuestRegs[uCPUID].rip + Vmx_VmRead(VM_EXIT_INSTRUCTION_LEN);
		Vmx_VmxOff();
		Asm_AfterVMXOff(g_GuestRegs[uCPUID].rsp,JmpEIP);
	}
}

void HandleMsrRead()
{
	ULONG64 uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	switch(g_GuestRegs[uCPUID].rcx)
	{
	case MSR_IA32_SYSENTER_CS:
		{
			g_GuestRegs[uCPUID].rax = Vmx_VmRead(GUEST_SYSENTER_CS) & 0xFFFFFFFF;
			g_GuestRegs[uCPUID].rdx = Vmx_VmRead(GUEST_SYSENTER_CS) >> 32;
			break;
		}
	case MSR_IA32_SYSENTER_ESP:
		{
			g_GuestRegs[uCPUID].rax = Vmx_VmRead(GUEST_SYSENTER_ESP) & 0xFFFFFFFF;
			g_GuestRegs[uCPUID].rdx = Vmx_VmRead(GUEST_SYSENTER_ESP) >> 32;
			break;
		}
	case MSR_IA32_SYSENTER_EIP:	// KiFastCallEntry
		{
			g_GuestRegs[uCPUID].rax = Vmx_VmRead(GUEST_SYSENTER_EIP) & 0xFFFFFFFF;
			g_GuestRegs[uCPUID].rdx = Vmx_VmRead(GUEST_SYSENTER_EIP) >> 32;
			break;
		}
	case MSR_FS_BASE:
		{
			g_GuestRegs[uCPUID].rax = Vmx_VmRead(GUEST_FS_BASE) & 0xFFFFFFFF;
			g_GuestRegs[uCPUID].rdx = Vmx_VmRead(GUEST_FS_BASE) >> 32;
			break;
		}
	case MSR_GS_BASE:
		{
			g_GuestRegs[uCPUID].rax = Vmx_VmRead(GUEST_GS_BASE) & 0xFFFFFFFF;
			g_GuestRegs[uCPUID].rdx = Vmx_VmRead(GUEST_GS_BASE) >> 32;
			break;
		}
	case MSR_EFER:
		{
			g_GuestRegs[uCPUID].rax = Asm_ReadMsr(MSR_EFER) & 0xFFFFFFFF;
			g_GuestRegs[uCPUID].rdx = Asm_ReadMsr(MSR_EFER) >> 32;
			break;
		}
	case   MSR_SHADOW_GS_BASE:
		{
			g_GuestRegs[uCPUID].rax = Asm_ReadMsr(MSR_SHADOW_GS_BASE) & 0xFFFFFFFF;
			g_GuestRegs[uCPUID].rdx = Asm_ReadMsr(MSR_SHADOW_GS_BASE) >> 32;
			break;
		}
	default:
		g_GuestRegs[uCPUID].rax = Asm_ReadMsr(g_GuestRegs[uCPUID].rcx) & 0xFFFFFFFF;
		g_GuestRegs[uCPUID].rdx = Asm_ReadMsr(g_GuestRegs[uCPUID].rcx) >> 32;
	}

}

void HandleMsrWrite()
{
	ULONG64 uCPUID;
	ULONG64 uMsr;
	uCPUID = KeGetCurrentProcessorNumber();
	uMsr = (g_GuestRegs[uCPUID].rax & 0xFFFFFFFF) | (g_GuestRegs[uCPUID].rdx << 32);
	switch(g_GuestRegs[uCPUID].rcx)
	{
	case MSR_IA32_SYSENTER_CS:
		{
			Vmx_VmWrite(GUEST_SYSENTER_CS,uMsr);
			break;
		}
	case MSR_IA32_SYSENTER_ESP:
		{
			Vmx_VmWrite(GUEST_SYSENTER_ESP, uMsr);
			break;
		}
	case MSR_IA32_SYSENTER_EIP:	// KiFastCallEntry
		{
			Vmx_VmWrite(GUEST_SYSENTER_EIP, uMsr);
			break;
		}
	case MSR_FS_BASE:
		{
			Vmx_VmWrite(GUEST_FS_BASE, uMsr);
			break;
		}
	case MSR_GS_BASE:
		{
			Vmx_VmWrite(GUEST_GS_BASE, uMsr);
			break;
		}
	case MSR_EFER:
		{
			Asm_WriteMsr(MSR_EFER, uMsr | EFER_LME);
			break;
		}
	case   MSR_SHADOW_GS_BASE:
	{
		Asm_WriteMsr(g_GuestRegs[uCPUID].rcx, uMsr);
		break;
	}
	default:
		Asm_WriteMsr(g_GuestRegs[uCPUID].rcx, uMsr);
	}
}

void HandleCrAccess()
{
	ULONG64		movcrControlRegister;
	ULONG64		movcrAccessType;
	ULONG64		movcrOperandType;
	ULONG64		movcrGeneralPurposeRegister;
	ULONG64		ExitQualification;
	ULONG64 uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();

	ExitQualification = Vmx_VmRead(EXIT_QUALIFICATION) ;
	movcrControlRegister = ( ExitQualification & 0x0000000F );
	movcrAccessType = ( ( ExitQualification & 0x00000030 ) >> 4 );
	movcrOperandType = ( ( ExitQualification & 0x00000040 ) >> 6 );
	movcrGeneralPurposeRegister = ( ( ExitQualification & 0x00000F00 ) >> 8 );
	if (movcrOperandType == 0)
	{
		if (movcrControlRegister == 3)
		{
			if (movcrAccessType == 0) // 写到CR3
			{
				switch (movcrGeneralPurposeRegister)
				{
				case 0:
				{
					Vmx_VmWrite(GUEST_CR3, g_GuestRegs[uCPUID].rax);
					break;
				}
				case 1:
				{
					Vmx_VmWrite(GUEST_CR3, g_GuestRegs[uCPUID].rcx);
					break;
				}
				case 2:
				{
					Vmx_VmWrite(GUEST_CR3, g_GuestRegs[uCPUID].rdx);
					break;
				}
				case 3:
				{
					Vmx_VmWrite(GUEST_CR3, g_GuestRegs[uCPUID].rbx);
					break;
				}
				case 4:
				{
					Vmx_VmWrite(GUEST_CR3, g_GuestRegs[uCPUID].rsp);
					break;
				}
				case 5:
				{
					Vmx_VmWrite(GUEST_CR3, g_GuestRegs[uCPUID].rbp);
					break;
				}
				case 6:
				{
					Vmx_VmWrite(GUEST_CR3, g_GuestRegs[uCPUID].rsi);
					break;
				}
				case 7:
				{
					Vmx_VmWrite(GUEST_CR3, g_GuestRegs[uCPUID].rdi);
					break;
				}
				default:
					break;
				}
			}
		}
		else if (movcrAccessType == 1)
		{
			switch (movcrGeneralPurposeRegister)
			{
			case 0:
			{
				g_GuestRegs[uCPUID].rax = g_GuestRegs[uCPUID].cr3;
				break;
			}
			case 1:
			{
				g_GuestRegs[uCPUID].rcx = g_GuestRegs[uCPUID].cr3;
				break;
			}
			case 2:
			{
				g_GuestRegs[uCPUID].rdx = g_GuestRegs[uCPUID].cr3;
				break;
			}
			case 3:
			{
				g_GuestRegs[uCPUID].rbx = g_GuestRegs[uCPUID].cr3;
				break;
			}
			case 4:
			{
				g_GuestRegs[uCPUID].rsp = g_GuestRegs[uCPUID].cr3;
				break;
			}
			case 5:
			{
				g_GuestRegs[uCPUID].rbp = g_GuestRegs[uCPUID].cr3;
				break;
			}
			case 6:
			{
				g_GuestRegs[uCPUID].rsi = g_GuestRegs[uCPUID].cr3;
				break;
			}
			case 7:
			{
				g_GuestRegs[uCPUID].rdi = g_GuestRegs[uCPUID].cr3;
				break;
			}
			default:
				break;
			}
		}
	}

}

void HandleRDTSC()
{
	ULONG uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	Asm_Rdtsc(&g_GuestRegs[uCPUID].rax, &g_GuestRegs[uCPUID].rdx);
}

// Work for Windows 10
void HandleRDTSCP()
{
	ULONG uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	g_GuestRegs[uCPUID].rax = (Asm_GetTSC() & 0xFFFFFFFF);
	g_GuestRegs[uCPUID].rdx = (Asm_GetTSC() >> 32);
}

VOID EventInject(ULONG32 trap)
{
	ULONG32 InjectEvent = (ULONG32)Vmx_VmRead(VM_EXIT_INTR_INFO);
	PINTERRUPT_INJECT_INFO_FIELD pInjectEvent = (PINTERRUPT_INJECT_INFO_FIELD)&InjectEvent;
	ULONG64 error_code = Vmx_VmRead(VM_EXIT_INTR_ERROR_CODE);
	ULONG32 IInfo = (ULONG32)Vmx_VmRead(GUEST_INTERRUPTIBILITY_INFO);
	PINTERRUPT_IBILITY_INFO pINTERRUPTIBILITY = (PINTERRUPT_IBILITY_INFO)&IInfo;
	//ULONG32 IInfo = (ULONG32)Vmx_VmRead ( GUEST_INTERRUPTIBILITY_INFO );
	//PINTERRUPT_IBILITY_INFO pINTERRUPTIBILITY=(PINTERRUPT_IBILITY_INFO)&IInfo;

	if (trap == TRAP_INT3)
	{
		InjectEvent = 0;
		pInjectEvent->Vector = BREAKPOINT_EXCEPTION; //7-0
		pInjectEvent->InterruptionType = SOFTWARE_EXCEPTION;//10-8
		pInjectEvent->DeliverErrorCode = 0;//11
		pInjectEvent->Reserved = 0;//32-12
		pInjectEvent->Valid = 1;//31

	}
	else if (trap == TRAP_INTO)
	{
		InjectEvent = 0;
		pInjectEvent->Vector = OVERFLOW_EXCEPTION; //7-0
		pInjectEvent->InterruptionType = SOFTWARE_EXCEPTION;//10-8
		pInjectEvent->DeliverErrorCode = 0;//11
		pInjectEvent->Reserved = 0;//32-12
		pInjectEvent->Valid = 1;//31

	}
	else if (trap == TRAP_DEBUG)
	{
		InjectEvent = 0;
		pInjectEvent->Vector = DEBUG_EXCEPTION; //7-0
		pInjectEvent->InterruptionType = HARDWARE_EXCEPTION;//10-8
		pInjectEvent->DeliverErrorCode = 0;//11
		pInjectEvent->Reserved = 0;//32-12
		pInjectEvent->Valid = 1;//31


	}
	else if (trap == TRAP_PAGE_FAULT)
	{
		//
		//  注入#PF
		//

		Asm_SetCr2(Vmx_VmRead(EXIT_QUALIFICATION));
		//VmxWrite( VM_ENTRY_EXCEPTION_ERROR_CODE, error_code );
		InjectEvent = 0;
		pInjectEvent->Vector = PAGE_FAULT_EXCEPTION;
		pInjectEvent->InterruptionType = HARDWARE_EXCEPTION;
		pInjectEvent->DeliverErrorCode = 1;
		pInjectEvent->Reserved = 0;//32-12
		pInjectEvent->Valid = 1;//31

	}
	else if (trap == TRAP_GP)
	{
		//
		//  注入#PF
		//

		Asm_SetCr2(Vmx_VmRead(EXIT_QUALIFICATION));
		InjectEvent = 0;
		pInjectEvent->Vector = GENERAL_PROTECTION_EXCEPTION;
		pInjectEvent->InterruptionType = HARDWARE_EXCEPTION;
		pInjectEvent->DeliverErrorCode = 1;
		pInjectEvent->Reserved = 0;//32-12
		pInjectEvent->Valid = 1;//31

	}
	else if (trap == TRAP_MTF)
	{
		pInjectEvent->Vector = DIVIDE_ERROR_EXCEPTION; //7-0
		pInjectEvent->InterruptionType = OTHER_EVENT;//10-8
		pInjectEvent->DeliverErrorCode = 0;
		pInjectEvent->Reserved = 0;//32-12
		pInjectEvent->Valid = 1;//31
		pINTERRUPTIBILITY->MOV_SS = 1;
		Vmx_VmWrite(GUEST_INTERRUPTIBILITY_INFO, IInfo);

	}
	else if (trap == 0x80)
	{
		pInjectEvent->Vector = 0x80; //7-0
		pInjectEvent->InterruptionType = OTHER_EVENT;//10-8
		pInjectEvent->DeliverErrorCode = 0;
		pInjectEvent->Reserved = 0;//32-12
		pInjectEvent->Valid = 1;//31

	}

	Vmx_VmWrite(VM_ENTRY_INSTRUCTION_LEN, Vmx_VmRead(VM_EXIT_INSTRUCTION_LEN));//这里插入我们自己的调试事件，然后程序就会开始一唱歌
	Vmx_VmWrite(VM_ENTRY_INTR_INFO_FIELD, InjectEvent);

	if (error_code != -1)
		Vmx_VmWrite(VM_ENTRY_EXCEPTION_ERROR_CODE, error_code);


}

BOOLEAN DrnReadWriht(ULONG64 Drn, PGUEST_REGS pGuestRegs)
{
	if (pGuestRegs->rax == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->rbx == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->rcx == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->rdx == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->rsp == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->rbp == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->rsi == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->rdi == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->r8 == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->r9 == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->r10 == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->r11 == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->r12 == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->r13 == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->r14 == Drn)
	{
		return TRUE;
	}
	if (pGuestRegs->r15 == Drn)
	{
		return TRUE;
	}
	return FALSE;
}

VOID DebugInt1DrnAccess(PGUEST_REGS pGuestRegs)
{
	ULONG64 RegDr0, RegDr1, RegDr2, RegDr3, RegDr6, RegDr7;
	ULONG32 DebugDr6, GuestDr7;
	PDEBUG_DR6 pRegDr6;
	PDEBUG_DR7 pRegDr7;
	ULONG64 GuestRip = Vmx_VmRead(GUEST_RIP);
	DebugDr6 = (ULONG32)Asm_GetDr6();
	GuestDr7 = (ULONG32)Vmx_VmRead(GUEST_DR7);
	pRegDr6 = (PDEBUG_DR6)&DebugDr6;
	pRegDr7 = (PDEBUG_DR7)&GuestDr7;
	RegDr0 = Asm_GetDr0();
	RegDr1 = Asm_GetDr1();
	RegDr2 = Asm_GetDr2();
	RegDr3 = Asm_GetDr3();
	RegDr6 = Asm_GetDr6();
	RegDr7 = Asm_GetDr7();
	if (GuestRip == RegDr0)
	{
		if (pRegDr7->L0 || pRegDr7->G0)
		{
			pRegDr6->B0 = 1;
		}
	}
	else if (GuestRip == RegDr1)
	{
		if (pRegDr7->L1 || pRegDr7->G1)
		{
			pRegDr6->B1 = 1;
		}
	}
	else if (GuestRip == RegDr2)
	{
		if (pRegDr7->L2 || pRegDr7->G2)
		{
			pRegDr6->B2 = 1;
		}
	}
	else if (GuestRip == RegDr3)
	{
		if (pRegDr7->L2 || pRegDr7->G2)
		{
			pRegDr6->B2 = 1;
		}
	}
	else if (DrnReadWriht(RegDr0, pGuestRegs))
	{
		if (pRegDr7->L0 || pRegDr7->G0)
		{
			pRegDr6->B0 = 1;
		}
	}
	else if (DrnReadWriht(RegDr1, pGuestRegs))
	{
		if (pRegDr7->L1 || pRegDr7->G1)
		{
			pRegDr6->B1 = 1;
		}
	}
	else if (DrnReadWriht(RegDr2, pGuestRegs))
	{
		if (pRegDr7->L2 || pRegDr7->G2)
		{
			pRegDr6->B2 = 1;
		}
	}
	else if (DrnReadWriht(RegDr3, pGuestRegs))
	{
		if (pRegDr7->L3 || pRegDr7->G3)
		{
			pRegDr6->B3 = 1;
		}
	}
	else
	{
		return;
	}
	pRegDr7->GD = 0;
	Asm_SetDr7(GuestDr7);
	Asm_SetDr6(DebugDr6);
}

VOID HandleException()
{
	ULONG32 Event, InjectEvent, DebugDr6, GuestDr7;
	ULONG64 ErrorCode, ExitQualification, GuestRip;
	PINTERRUPT_INFO_FIELD pEvent;
	PINTERRUPT_INJECT_INFO_FIELD pInjectEvent;
	PDEBUG_DR6 pRegDr6;
	PDEBUG_DR7 pRegDr7;
	ULONG64 uCurCPU;
	GuestDr7 = (ULONG32)Vmx_VmRead(GUEST_DR7);
	pRegDr7 = (PDEBUG_DR7)&GuestDr7;
	Event = (ULONG32)Vmx_VmRead(VM_EXIT_INTR_INFO);
	pEvent = (PINTERRUPT_INFO_FIELD)&Event;
	uCurCPU = KeGetCurrentProcessorNumber();

	InjectEvent = 0;
	pInjectEvent = (PINTERRUPT_INJECT_INFO_FIELD)&InjectEvent;

	GuestRip = Vmx_VmRead(GUEST_RIP);
	switch (pEvent->InterruptionType)
	{
	case NMI_INTERRUPT:
	{
		InjectEvent = 0;
		pInjectEvent->Vector = NMI_INTERRUPT;
		pInjectEvent->InterruptionType = NMI_INTERRUPT;
		pInjectEvent->DeliverErrorCode = 0;
		pInjectEvent->Valid = 1;
		Vmx_VmWrite(VM_ENTRY_INTR_INFO_FIELD, InjectEvent);
		break;
	}
	case EXTERNAL_INTERRUPT:
		break;

	case HARDWARE_EXCEPTION:
	{
		switch (pEvent->Vector)
		{
		case DEBUG_EXCEPTION:
		{
			DebugInt1DrnAccess(&g_GuestRegs[uCurCPU]);
			DebugDr6 = (ULONG32)Asm_GetDr6();
			pRegDr6 = (PDEBUG_DR6)&DebugDr6;
			EventInject(TRAP_DEBUG);
			break;
		}


		case PAGE_FAULT_EXCEPTION:
		{
			EventInject(TRAP_PAGE_FAULT);
			break;
		}


		default:
			break;
		}
		break;
	}

	case SOFTWARE_EXCEPTION:
	{
		switch (pEvent->Vector)
		{
		case BREAKPOINT_EXCEPTION:
			EventInject(TRAP_INT3);	// 于是我们不用IDTHook也可以检测int3断点了
			break;

		case OVERFLOW_EXCEPTION:
			break;
		default:
			break;
		}
		break;
	}


	default:
		break;
	}
}

extern "C" ULONG64 GetGuestRegsAddress()
{
	ULONG64 uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	return (ULONG64)&g_GuestRegs[uCPUID];
}

extern "C" void VMMEntryPoint()
{
	ULONG64 ExitReason;
	ULONG64 ExitInstructionLength;
	ULONG64 GuestResumeEIP;
	ULONG64 uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	ExitReason = Vmx_VmRead(VM_EXIT_REASON);
	ExitInstructionLength = Vmx_VmRead(VM_EXIT_INSTRUCTION_LEN);

	g_GuestRegs[uCPUID].rsp = Vmx_VmRead(GUEST_RSP);
	g_GuestRegs[uCPUID].rip = Vmx_VmRead(GUEST_RIP);
	g_GuestRegs[uCPUID].cr3 = Vmx_VmRead(GUEST_CR3);
	switch (ExitReason)
	{
	case EXIT_REASON_EXCEPTION_NMI:
	{
		HandleException();
		return;
	}
	case EXIT_REASON_TRIPLE_FAULT:
	{
		break;
	}
	case EXIT_REASON_CPUID:
	{
		HandleCPUID();
		break;
	}
	case EXIT_REASON_INVD:
	{
		HandleInvd();
		break;
	}
	case EXIT_REASON_VMCALL:
	{
		HandleVmCall();
		break;
	}
	case EXIT_REASON_MSR_READ:
	{
		HandleMsrRead();
		break;
	}
	case EXIT_REASON_MSR_WRITE:
	{
		HandleMsrWrite();
		break;
	}
	case EXIT_REASON_RDTSC:  // 16
	{
		HandleRDTSC();
		break;
	}
	case EXIT_REASON_RDTSCP: // 51
	{
		HandleRDTSCP();
		break;
	}
	case EXIT_REASON_EPT_VIOLATION:
	{
		HandleEptViolation();
		return;
	}
	case EXIT_REASON_EPT_MISCONFIG:
	{
		HandleEptMisconfig();
		return;
	}
	case EXIT_REASON_MTF_TRAP_FLAG:
	{
		HandleMTF();
		return;
	}
	default:
		break;
	}
	GuestResumeEIP = g_GuestRegs[uCPUID].rip+ExitInstructionLength;
	Vmx_VmWrite(GUEST_RIP,GuestResumeEIP);
	Vmx_VmWrite(GUEST_RSP,g_GuestRegs[uCPUID].rsp);
}