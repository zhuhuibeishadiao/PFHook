/*
Author:Xiaobao
QQ:1121402724
*/
#pragma  once

typedef struct
{
	unsigned PE : 1;
	unsigned MP : 1;
	unsigned EM : 1;
	unsigned TS : 1;
	unsigned ET : 1;
	unsigned NE : 1;
	unsigned Reserved_1 : 10;
	unsigned WP : 1;
	unsigned Reserved_2 : 1;
	unsigned AM : 1;
	unsigned Reserved_3 : 10;
	unsigned NW : 1;
	unsigned CD : 1;
	unsigned PG : 1;
	unsigned Reserved_64 : 32;
}_CR0;

typedef struct
{
	unsigned VME:1;
	unsigned PVI:1;
	unsigned TSD:1;
	unsigned DE:1;
	unsigned PSE:1;
	unsigned PAE:1;
	unsigned MCE:1;
	unsigned PGE:1;
	unsigned PCE:1;
	unsigned OSFXSR:1;
	unsigned PSXMMEXCPT:1;
	unsigned UNKONOWN_1:1;		//These are zero
	unsigned UNKONOWN_2:1;		//These are zero
	unsigned VMXE:1;			//It's zero in normal
	unsigned Reserved:18;		//These are zero
	unsigned Reserved_64 : 32;
}_CR4;

typedef struct
{
	unsigned CF:1;
	unsigned Unknown_1:1;	//Always 1
	unsigned PF:1;
	unsigned Unknown_2:1;	//Always 0
	unsigned AF:1;
	unsigned Unknown_3:1;	//Always 0
	unsigned ZF:1;
	unsigned SF:1;
	unsigned TF:1;
	unsigned IF:1;
	unsigned DF:1;
	unsigned OF:1;
	unsigned TOPL:2;
	unsigned NT:1;
	unsigned Unknown_4:1;
	unsigned RF:1;
	unsigned VM:1;
	unsigned AC:1;
	unsigned VIF:1;
	unsigned VIP:1;
	unsigned ID:1;
	unsigned Reserved:10;	//Always 0
	unsigned Reserved_64:32;	//Always 0
}_EFLAGS;

typedef struct
{
	unsigned SSE3:1;
	unsigned PCLMULQDQ:1;
	unsigned DTES64:1;
	unsigned MONITOR:1;
	unsigned DS_CPL:1;
	unsigned VMX:1;
	unsigned SMX:1;
	unsigned EIST:1;
	unsigned TM2:1;
	unsigned SSSE3:1;
	unsigned Reserved:22;
	unsigned Reserved_64:32;
}_CPUID_ECX;

typedef struct _IA32_FEATURE_CONTROL_MSR
{
	unsigned Lock			:1;		// Bit 0 is the lock bit - cannot be modified once lock is set
	unsigned Reserved1		:1;		// Undefined
	unsigned EnableVmxon	:1;		// Bit 2. If this bit is clear, VMXON causes a general protection exception
	unsigned Reserved2		:29;	// Undefined
	unsigned Reserved3		:32;	// Undefined

} IA32_FEATURE_CONTROL_MSR;

typedef struct _VMX_BASIC_MSR
{
	unsigned RevId: 32;//∞Ê±æ∫≈–≈œ¢
	unsigned szVmxOnRegion: 12;
	unsigned ClearBit: 1;
	unsigned Reserved: 3;
	unsigned PhysicalWidth: 1;
	unsigned DualMonitor: 1;
	unsigned MemoryType: 4;
	unsigned VmExitInformation: 1;
	unsigned Reserved2: 9;
} VMX_BASIC_MSR, *PVMX_BASIC_MSR;

extern "C" ULONG64 Asm_GetRflags();
extern "C" ULONG64 Asm_GetCs();
extern "C" ULONG64 Asm_GetDs();
extern "C" ULONG64 Asm_GetEs();
extern "C" ULONG64 Asm_GetFs();
extern "C" ULONG64 Asm_GetGs();
extern "C" ULONG64 Asm_GetSs();
extern "C" ULONG64 Asm_GetLdtr();
extern "C" ULONG64 Asm_GetTr();

extern "C" void Asm_SetGdtr(ULONG64 uBase,ULONG64 uLimit);
extern "C" void Asm_SetIdtr(ULONG64 uBase,ULONG64 uLimit);

extern "C" ULONG64 Asm_GetGdtBase();
extern "C" ULONG64 Asm_GetIdtBase();
extern "C" ULONG64 Asm_GetGdtLimit();
extern "C" ULONG64 Asm_GetIdtLimit();

extern "C" ULONG64 Asm_GetCr0();
extern "C" ULONG64 Asm_GetCr2();
extern "C" ULONG64 Asm_GetCr3();
extern "C" ULONG64 Asm_GetCr4();
extern "C" void Asm_SetCr0(ULONG64 uNewCr0);
extern "C" void Asm_SetCr2(ULONG64 uNewCr2);
extern "C" void Asm_SetCr3(ULONG64 uNewCr3);
extern "C" void Asm_SetCr4(ULONG64 uNewCr4);

extern "C" ULONG64 Asm_GetDr0();
extern "C" ULONG64 Asm_GetDr1();
extern "C" ULONG64 Asm_GetDr2();
extern "C" ULONG64 Asm_GetDr3();
extern "C" ULONG64 Asm_GetDr6();
extern "C" ULONG64 Asm_GetDr7();
extern "C" void Asm_SetDr0(ULONG64 uNewDr0);
extern "C" void Asm_SetDr1(ULONG64 uNewDr1);
extern "C" void Asm_SetDr2(ULONG64 uNewDr2);
extern "C" void Asm_SetDr3(ULONG64 uNewDr3);
extern "C" void Asm_SetDr6(ULONG64 uNewDr6);
extern "C" void Asm_SetDr7(ULONG64 uNewDr7);

extern "C" ULONG64 Asm_ReadMsr(ULONG64 uIndex);
extern "C" void Asm_WriteMsr(ULONG64 uIndex,ULONG64 QuadPart);

extern "C" void Asm_CPUID(ULONG64 uFn,PULONG64 uRet_EAX,PULONG64 uRet_EBX,PULONG64 uRet_ECX,PULONG64 uRet_EDX);
extern "C" void Asm_Invd();
extern "C" void Asm_Rdtsc(PULONG64 uRet_EAX, PULONG64 uRet_EDX);
extern "C" ULONG64 Asm_GetTSC();

extern "C" void Vmx_VmxOn(ULONG64 QuadPart);
extern "C" void Vmx_VmxOff();
extern "C" void Vmx_VmClear(ULONG64 QuadPart);
extern "C" void Vmx_VmPtrld(ULONG64 QuadPart);
extern "C" ULONG64 Vmx_VmRead(ULONG64 uField);
extern "C" void Vmx_VmWrite(ULONG64 uField,ULONG64 uValue);
extern "C" void Vmx_VmLaunch();
extern "C" void Vmx_VmResume();
extern "C" void Vmx_VmCall(ULONG64 uCallNumber);
extern "C" void Vmx_Invept(PVOID  Ep4ta, ULONG64 inval);

extern "C" void Asm_VMMEntryPoint();

extern "C" void Asm_SetupVMCS();

extern "C" ULONG64 Asm_GetGuestReturn();
extern "C" ULONG64 Asm_GetGuestRSP();

extern "C" void Asm_AfterVMXOff(ULONG64 JmpESP,ULONG64 JmpEIP);

extern "C" void Asm_MyNtOpenProcess();
extern "C" void Asm_MyNtOpenThread();
extern "C" void Asm_MyNtCreateUserProcess();
extern "C" void Asm_MyNtCreateThreadEx();
extern "C" void Asm_MyNtCreateFile_Win7();
extern "C" void Asm_MyNtCreateFile_Win10();
extern "C" void Asm_MyNtLoadDriver();
extern "C" void Asm_MyNtCreateKey();
extern "C" void Asm_MyNtQueryKey_Win7();
extern "C" void Asm_MyNtQueryKey_Win10();
extern "C" void Asm_MyNtDeleteFile();
extern "C" void Asm_MyNtReadVirtualMemory();