#include "header.h"
#include "global.h"
#include "vtsystem.h"
#include "vtasm.h"
#include "ept.h"

BOOLEAN HandleEvent_Hook(IN OUT PGUEST_REGS pRegs)
{
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS proc;
	EPTHOOKITEM item;
	KAPC_STATE state;
	PHYSICAL_ADDRESS pa;
	ULONG uCPUID;
	ULONG64 uPID, uHookedAddr, uJmpAddr;
	int uItemID;
	
	uCPUID = KeGetCurrentProcessorNumber();
	uPID = pRegs->rbx;
	uHookedAddr = pRegs->rcx;
	uJmpAddr = pRegs->rdx;

	if (uHookedAddr == 0 || uJmpAddr == 0)
		goto Error;
	status = PsLookupProcessByProcessId((HANDLE)uPID, &proc);
	if (NT_SUCCESS(status))
	{
		KeStackAttachProcess(proc, &state); // 这里必须切换进程，这样才能获取到真正的物理地址

		pa = MmGetPhysicalAddress((PVOID)uHookedAddr);
		if (!pa.QuadPart)
			goto Error;

		item.pPte = EptGetPteAddressByPA(pa);
		if (!item.pPte)
			goto Error;

		item.pHookedVA = (PVOID)uHookedAddr;
		item.pJmpVA = (PVOID)uJmpAddr;

		uItemID = AddEPTHookItem(&item);
		if (uItemID == -1)
			goto Error;

		KeUnstackDetachProcess(&state);

		KdPrint(("EPTHook %s[%d] 0x%llX --> 0x%llX\n", PsGetProcessImageFileName(proc), uPID, uHookedAddr, uJmpAddr));

		pRegs->rsi = uItemID;
		return TRUE;
	}

Error:
	pRegs->rsi = -1;
	return FALSE;
}

BOOLEAN HandleEvent_Unhook(IN OUT PGUEST_REGS pRegs)
{
	PEPROCESS proc;
	NTSTATUS status = STATUS_SUCCESS;
	EPTHOOKITEM item;
	KAPC_STATE state;
	PHYSICAL_ADDRESS pa;
	ULONG uCPUID;
	ULONG64 uPID, uHookedAddr, uJmpAddr;
	int uItemID;

	uCPUID = KeGetCurrentProcessorNumber();
	uItemID = pRegs->rcx;

	if (RemoveEPTHookItem(uItemID))
	{
		pRegs->rsi = 1;
		return TRUE;
	}

Error:
	pRegs->rsi = 0;
	return FALSE;
}

void HandleEvent_CheckVT(IN OUT PGUEST_REGS pRegs)
{
	pRegs->rsi = 1;
}

// 这部分代码是以前弄无痕R0 Hook时写的
ULONG64 GetKeServiceDescriptorTable64()
{
	ULONG64 pKiSystemCall64 = Asm_ReadMsr(0xc0000082);
	ULONG uOffset;
	UCHAR a1, a2, a3;

	for (int i = 0;i <= 0x500;i++)
	{
		if (MmIsAddressValid((PVOID)(i + pKiSystemCall64)) && MmIsAddressValid((PVOID)(i + pKiSystemCall64 + 1)) && MmIsAddressValid((PVOID)(i + pKiSystemCall64 + 2)))
		{
			a1 = *((UCHAR*)(i + pKiSystemCall64));
			a2 = *((UCHAR*)(i + 1 + pKiSystemCall64));
			a3 = *((UCHAR*)(i + 2 + pKiSystemCall64));
			if (a1 == 0x4C && a2 == 0x8D && a3 == 0x15)
			{
				uOffset = *((PULONG)(i + pKiSystemCall64 + 3));
				return i + pKiSystemCall64 + uOffset + 7;
			}
		}
	}
	return 0;
}

ULONG64 GetSSDTFunctionAddr(IN ULONG uID)
{
	PSYSTEM_SERVICE_TABLE pKeServiceDescriptorTable64;
	ULONG64 qwTemp;
	ULONG dwTemp;
	pKeServiceDescriptorTable64 = (PSYSTEM_SERVICE_TABLE)GetKeServiceDescriptorTable64();
	if (!pKeServiceDescriptorTable64)
		return 0;
	qwTemp = (ULONG64)pKeServiceDescriptorTable64->ServiceTableBase + 4 * uID;
	dwTemp = *((PULONG)(qwTemp));
	dwTemp = dwTemp >> 4;
	return (ULONG64)pKeServiceDescriptorTable64->ServiceTableBase + dwTemp;
}