#include "vtsystem.h"
// Í¨ÐÅ´úÂë
#define PFHOOK_CODE_CHECKVT	0x800
#define PFHOOK_CODE_HOOK	0x801
#define PFHOOK_CODE_UNHOOK	0x802

typedef struct _SYSTEM_SERVICE_TABLE {
	PVOID ServiceTableBase;
	PVOID ServiceCounterTableBase;
	ULONGLONG NumberOfServices;
	PVOID ParamTableBase;
} SYSTEM_SERVICE_TABLE, *PSYSTEM_SERVICE_TABLE;

BOOLEAN HandleEvent_Hook(IN OUT PGUEST_REGS pRegs);
BOOLEAN HandleEvent_Unhook(IN OUT PGUEST_REGS pRegs);
void HandleEvent_CheckVT(IN OUT PGUEST_REGS pRegs);

extern "C"
{
	NTSYSAPI
		UCHAR *
		NTAPI
		PsGetProcessImageFileName(
			IN PEPROCESS proc
		);
}

ULONG64 GetKeServiceDescriptorTable64();
ULONG64 GetSSDTFunctionAddr(IN ULONG uID);
