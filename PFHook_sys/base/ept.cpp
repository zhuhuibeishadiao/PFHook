#include "header.h"
#include "vtsystem.h"
#include "ept.h"
#include "common.h"
#include "vtasm.h"

#define NUM_PAGES	24		// 24GB内存 注意这里，必须在系统物理内存以下
#define NUM_MAX_HOOK 256

extern VMX_CPU g_VMXCPU[128];
extern GUEST_REGS g_GuestRegs[128];
EptPml4Entry*  g_pPml4T;
EptPdpteEntry* g_pPdpteTable;
EptPdeEntry* g_pPdeTable[NUM_PAGES];
EptPteEntry* g_pPteTable[NUM_PAGES][512];

EPTHOOKITEM g_EptHookItems[NUM_MAX_HOOK];
PEPTREWATCHINFO g_EptRewatch[128];
int g_nEptHookCount = 0;

EptPml4Entry* EptInitialization()
{
	EptPml4Entry*  ept_PML4T;
	PHYSICAL_ADDRESS FirstPtePA, FirstPdePA, FirstPdptePA;
	ept_PML4T = (EptPml4Entry*)(ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 'aa'));
	RtlZeroMemory(ept_PML4T, PAGE_SIZE);

	EptPdpteEntry* ept_PDPTE = (EptPdpteEntry*)(ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 'aa'));
	RtlZeroMemory(ept_PDPTE, PAGE_SIZE);
	FirstPdptePA = MmGetPhysicalAddress(ept_PDPTE);

	ept_PML4T->Read = 1;
	ept_PML4T->Write = 1;
	ept_PML4T->Execute = 1;
	ept_PML4T->PhysAddr = FirstPdptePA.QuadPart >> 12;
	g_pPml4T = ept_PML4T;
	g_pPdpteTable = ept_PDPTE;
	for (ULONG64 a = 0;a < NUM_PAGES;a++)
	{
		EptPdeEntry* ept_PDE = (EptPdeEntry*)(ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 'aa')); // 普通模式
		RtlZeroMemory(ept_PDE, PAGE_SIZE);
		FirstPdePA = MmGetPhysicalAddress(ept_PDE);

		ept_PDPTE->Read = 1;
		ept_PDPTE->Write = 1;
		ept_PDPTE->Execute = 1;
		ept_PDPTE->PhysAddr = FirstPdePA.QuadPart >> 12;
		ept_PDPTE++;
		g_pPdeTable[a] = ept_PDE;

		for (int b = 0;b < 512;b++)
		{
			EptPteEntry* ept_PTE = (EptPteEntry*)(ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 'aa'));  // 普通模式
			g_pPteTable[a][b] = ept_PTE;
			RtlZeroMemory(ept_PTE, PAGE_SIZE);
			FirstPtePA = MmGetPhysicalAddress(ept_PTE);

			ept_PDE->PhysAddr = FirstPtePA.QuadPart >> 12;
			ept_PDE->Read = 1;
			ept_PDE->Write = 1;
			ept_PDE->Execute = 1;

			ept_PDE++;

			for (int c = 0;c < 512;c++)
			{
				ept_PTE->PhysAddr = (a*(1 << 30) + b*(1 << 21) + c*(1 << 12)) >> 12;
				ept_PTE->Read = 1;
				ept_PTE->Write = 1;
				ept_PTE->Execute = 1;
				ept_PTE->MemoryType = EPT_MEMORY_TYPE_WB;
				ept_PTE++;
			}
		}
	}
	return ept_PML4T;
}

BOOLEAN EptReleasePages()
{
	ExFreePoolWithTag(g_pPml4T,'aa');
	ExFreePoolWithTag(g_pPdpteTable,'aa');

	for (int i = 0;i<NUM_PAGES;i++)
	{
		ExFreePoolWithTag(g_pPdeTable[i], 'aa');

		for (int j = 0;j<512;j++)
		{
			ExFreePoolWithTag(g_pPteTable[i][j], 'aa');
		}
	}
	return TRUE;
}

EptPteEntry* EptGetPteAddressByPA(IN PHYSICAL_ADDRESS PA)
{
	ULONG Page_ID, Page_Offset;
	EptPteEntry* PageFirstPte;
	EptPteEntry* PagePte;
	PHYSICAL_ADDRESS lowPA,highPA;
	lowPA.QuadPart = PA.QuadPart & 0xFFFFFFFF;
	highPA.QuadPart = (PA.QuadPart & 0xFFFFFFFF00000000) >> 32;
	lowPA.QuadPart = lowPA.QuadPart & 0xFFFFF000;

	Page_ID = (lowPA.QuadPart >> 21) * 8 + (highPA.QuadPart * 4 * 0x1000);// 得到页表偏移
	Page_Offset = (lowPA.QuadPart >> 9) & 0xFFF;

	PageFirstPte = (EptPteEntry*)(*(PULONG64)((ULONG64)(g_pPteTable)+Page_ID));
	PagePte = (EptPteEntry*)((ULONG64)PageFirstPte + Page_Offset);
	return PagePte;

}

void HandleEptViolation()
{
	PHYSICAL_ADDRESS Guest_PA, Guest_LA;
	ULONG uGuest_PA_Q, uGuest_PA_Q_H, uGuest_LA_Q;
	ULONG64 Exit_Qualification;
	PEPT_ATTRIBUTE_PAGE pEpt_Attribute;
	EptPteEntry* pPte;
	ULONG uCPUID;
	ULONG uGuestRIP;
	
	uCPUID = KeGetCurrentProcessorNumber();
	uGuest_PA_Q = Vmx_VmRead(GUEST_PHYSICAL_ADDRESS);
	uGuest_PA_Q_H = Vmx_VmRead(GUEST_PHYSICAL_ADDRESS_HIGH);
	uGuest_LA_Q = Vmx_VmRead(GUEST_LINEAR_ADDRESS);
	Exit_Qualification = Vmx_VmRead(EXIT_QUALIFICATION);
	pEpt_Attribute = (PEPT_ATTRIBUTE_PAGE)&Exit_Qualification;

	Guest_PA.LowPart = uGuest_PA_Q;
	Guest_PA.HighPart = uGuest_PA_Q_H;
	Guest_LA.QuadPart = uGuest_LA_Q;

	uGuestRIP = Vmx_VmRead(GUEST_RIP);
	pPte = EptGetPteAddressByPA(Guest_PA);
	if (pEpt_Attribute->Read)
	{
		// Read Access
	}

	if (pEpt_Attribute->Write)
	{
		// Write Access
	}

	if (pEpt_Attribute->Execute)
	{
		// Execute Access
		for (int i = 0;i < g_nEptHookCount;i++)
		{
			if ((ULONG64)g_EptHookItems[i].pHookedVA == uGuestRIP)
			{
				if (!g_EptHookItems[i].bDisabled)
				{
					KdPrint(("From 0x%p to 0x%p.\n", uGuestRIP, g_EptHookItems[i].pJmpVA));
					Vmx_VmWrite(GUEST_RIP, (ULONG64)g_EptHookItems[i].pJmpVA);
				}
				g_EptHookItems[i].nHitCount++;
			}
			if (g_EptHookItems[i].pPte == pPte)
			{
				g_EptRewatch[uCPUID] = SetRewathInfo(pPte, g_EptHookItems[i].pOldPte, (PVOID64)Guest_PA.QuadPart);
			}

		}
	}


	pPte->Read = 1;
	pPte->Write = 1;
	pPte->Execute = 1;

	EptInvept();
	EnableMTF();
}

void HandleEptMisconfig()
{
	PHYSICAL_ADDRESS Guest_PA, Guest_LA;
	ULONG uGuest_PA_Q, uGuest_LA_Q;
	ULONG64 Exit_Qualification;
	PEPT_ATTRIBUTE_PAGE pEpt_Attribute;
	ULONG uCPUID;
	// 如果程序运行到了这里，则说明ept表填写错误

	uCPUID = KeGetCurrentProcessorNumber();
	uGuest_PA_Q = Vmx_VmRead(GUEST_PHYSICAL_ADDRESS);
	uGuest_LA_Q = Vmx_VmRead(GUEST_LINEAR_ADDRESS);
	Exit_Qualification = Vmx_VmRead(EXIT_QUALIFICATION);

	pEpt_Attribute = (PEPT_ATTRIBUTE_PAGE)&Exit_Qualification;
	Guest_PA.QuadPart = uGuest_PA_Q;
	Guest_LA.QuadPart = uGuest_LA_Q;

	KeBugCheck(0x12345678);
}

void HandleMTF()
{
	ULONG64 GuestRip = Vmx_VmRead(GUEST_RIP);
	ULONG64 GuestRsp = Vmx_VmRead(GUEST_RSP); // [rsp+8]储存有上一个地址

	EPTREWATCHINFO RewatchInfo;
	ULONG uCPUID = KeGetCurrentProcessorNumber();

	if (g_EptRewatch[uCPUID] == NULL)
		return;

	RewatchInfo = *(g_EptRewatch[uCPUID]);
	*(RewatchInfo.pPte) = RewatchInfo.pOldPte;
	EptInvept();

	ExFreePoolWithTag(g_EptRewatch[uCPUID], 'info');
	g_EptRewatch[uCPUID] = NULL;

	DisableMTF();
}

void EptInvept()
{
	ULONG uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	EptTablePointer EPTP = { 0 };
	PHYSICAL_ADDRESS phys = MmGetPhysicalAddress((void *)g_VMXCPU[uCPUID].ept_PML4T);

	// Set up the EPTP
	EPTP.Bits.PhysAddr = phys.QuadPart >> 12;
	EPTP.Bits.PageWalkLength = 3;

	Vmx_Invept((PVOID)&EPTP, 2);
}

PVOID EptAllocateNewPage()
{
	PVOID pPage;
	pPage = ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 0);
	if (!pPage)
		return NULL;
	RtlZeroMemory(pPage, PAGE_SIZE);
	return pPage;
}

void EnableMTF()
{
	ULONG64 uCPUBase;
	uCPUBase = Vmx_VmRead(CPU_BASED_VM_EXEC_CONTROL);
	uCPUBase |= CPU_BASED_MTF_TRAP_EXITING;
	Vmx_VmWrite(CPU_BASED_VM_EXEC_CONTROL, uCPUBase);
}

void DisableMTF()
{
	ULONG64 uCPUBase;
	uCPUBase = Vmx_VmRead(CPU_BASED_VM_EXEC_CONTROL);
	uCPUBase &= ~CPU_BASED_MTF_TRAP_EXITING;
	Vmx_VmWrite(CPU_BASED_VM_EXEC_CONTROL, uCPUBase);
}

VOID EnableTF()//开启TF单步调试功能
{
	ULONG64 uTemp64;
	_EFLAGS* Rflags;
	uTemp64 = Vmx_VmRead(GUEST_RFLAGS);
	Rflags = (_EFLAGS*)&uTemp64;

	Rflags->TF = 1;
	Vmx_VmWrite(GUEST_RFLAGS, uTemp64);

}


VOID DisableTF()//关闭TF单步调试功能
{
	ULONG64 uTemp64;
	_EFLAGS* Rflags;
	uTemp64 = Vmx_VmRead(GUEST_RFLAGS);
	Rflags = (_EFLAGS*)&uTemp64;

	Rflags->TF = 0;
	Vmx_VmWrite(GUEST_RFLAGS, uTemp64);
}

int AddEPTHookItem(IN PEPTHOOKITEM pItem)
{
	if (g_nEptHookCount > NUM_MAX_HOOK)
		return -1;

 	pItem->pPte->Execute = 0;

	pItem->pOldPte = *pItem->pPte;
	pItem->bDisabled = FALSE;

	g_EptHookItems[g_nEptHookCount] = *pItem;

	g_nEptHookCount++;
	return g_nEptHookCount - 1;
}

BOOLEAN RemoveEPTHookItem(IN int uItemID)
{
	EptPteEntry* pPte;
	if (g_nEptHookCount == 0)
		return FALSE;
	
	pPte = g_EptHookItems[uItemID].pPte;
	g_EptHookItems[uItemID].bDisabled = TRUE;
	return TRUE;
}

PEPTREWATCHINFO SetRewathInfo(IN EptPteEntry* pPte,IN EptPteEntry pOldPte,IN PVOID64 pContext)
{
	PEPTREWATCHINFO pInfo = (PEPTREWATCHINFO)ExAllocatePoolWithTag(NonPagedPool, sizeof(EPTREWATCHINFO),'info');
	if (pInfo == NULL)
		return NULL;
	RtlZeroMemory(pInfo, sizeof(EPTREWATCHINFO));
	
	pInfo->pPte = pPte;
	pInfo->pOldPte = pOldPte;
	pInfo->pContext = pContext;

	return pInfo;
}