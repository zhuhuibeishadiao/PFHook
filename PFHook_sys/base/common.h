#pragma once


#define LA_ACCESSED		0x01
#define LA_READABLE		0x02    // for code segments
#define LA_WRITABLE		0x02    // for data segments
#define LA_CONFORMING	0x04    // for code segments
#define LA_EXPANDDOWN	0x04    // for data segments
#define LA_CODE			0x08
#define LA_STANDARD		0x10
#define LA_DPL_0		0x00
#define LA_DPL_1		0x20
#define LA_DPL_2		0x40
#define LA_DPL_3		0x60
#define LA_PRESENT		0x80

#define LA_LDT64		0x02
#define LA_ATSS64		0x09
#define LA_BTSS64		0x0b
#define LA_CALLGATE64	0x0c
#define LA_INTGATE64	0x0e
#define LA_TRAPGATE64	0x0f

#define HA_AVAILABLE	0x01
#define HA_LONG			0x02
#define HA_DB			0x04
#define HA_GRANULARITY	0x08

#define P_PRESENT			0x01
#define P_WRITABLE			0x02
#define P_USERMODE			0x04
#define P_WRITETHROUGH		0x08
#define P_CACHE_DISABLED	0x10
#define P_ACCESSED			0x20
#define P_DIRTY				0x40
#define P_LARGE				0x80
#define P_GLOBAL			0x100

#define	PML4_BASE	0xFFFFF6FB7DBED000 //和windows内核的四个常量对应
#define	PDP_BASE	0xFFFFF6FB7DA00000 //#define PXE_BASE 0xFFFFF6FB7DBED000UI64
#define	PD_BASE		0xFFFFF6FB40000000 //#define PPE_BASE 0xFFFFF6FB7DA00000UI64
#define	PT_BASE		0xFFFFF68000000000 //#define PDE_BASE 0xFFFFF6FB40000000UI64
//#define PTE_BASE 0xFFFFF68000000000UI64

#define ITL_TAG	'LTI'

#define BP_GDT_LIMIT	0x6f
#define BP_IDT_LIMIT	0xfff
#define BP_TSS_LIMIT	0x68    // 0x67 min

#define BP_GDT_LIMIT	0x6f
#define BP_IDT_LIMIT	0xfff
#define BP_TSS_LIMIT	0x68    // 0x67 min


#define TRAP_MTF						0
#define TRAP_DEBUG						1
#define TRAP_INT3						3
#define TRAP_INTO						4
#define TRAP_GP					    13
#define TRAP_PAGE_FAULT					14
#define TRAP_INVALID_OP					6

/* 
* Attribute for segment selector. This is a copy of bit 40:47 & 52:55 of the
* segment descriptor. 
*/
typedef union
{
	USHORT UCHARs;
	struct
	{
		USHORT type:4;              /* 0;  Bit 40-43 */
		USHORT s:1;                 /* 4;  Bit 44 */
		USHORT dpl:2;               /* 5;  Bit 45-46 */
		USHORT p:1;                 /* 7;  Bit 47 */
		// gap!       
		USHORT avl:1;               /* 8;  Bit 52 */
		USHORT l:1;                 /* 9;  Bit 53 */
		USHORT db:1;                /* 10; Bit 54 */
		USHORT g:1;                 /* 11; Bit 55 */
		USHORT Gap:4;
	} fields;
} SEGMENT_ATTRIBUTES;

typedef struct _SEGMENT_SELECTOR
{
	USHORT sel;
	SEGMENT_ATTRIBUTES attributes;
	ULONG limit;
	ULONG64 base;
} SEGMENT_SELECTOR, *PSEGMENT_SELECTOR;

typedef struct _SEGMENT_DESCRIPTOR
{
	USHORT limit0;
	USHORT base0;
	UCHAR base1;
	UCHAR attr0;
	UCHAR limit1attr1;
	UCHAR base2;
} SEGMENT_DESCRIPTOR, *PSEGMENT_DESCRIPTOR;

typedef struct _TSS64
{
	ULONG Reserved0;
	PVOID RSP0;
	PVOID RSP1;
	PVOID RSP2;
	ULONG64 Reserved1;
	PVOID IST1;
	PVOID IST2;
	PVOID IST3;
	PVOID IST4;
	PVOID IST5;
	PVOID IST6;
	PVOID IST7;
	ULONG64 Reserved2;
	USHORT Reserved3;
	USHORT IOMapBaseAddress;
} TSS64,
*PTSS64;

typedef struct	_SEG_DESCRIPTOR
{
	unsigned	LimitLo	:16;
	unsigned	BaseLo	:16;
	unsigned	BaseMid	:8;
	unsigned	Type	:4;
	unsigned	System	:1;
	unsigned	DPL		:2;
	unsigned	Present	:1;
	unsigned	LimitHi	:4;
	unsigned	AVL		:1;
	unsigned	L		:1;
	unsigned	DB		:1;
	unsigned	Gran	:1;		// Granularity
	unsigned	BaseHi	:8;

} SEG_DESCRIPTOR;

enum SEGREGS
{
	ES = 0,
	CS,
	SS,
	DS,
	FS,
	GS,
	LDTR,
	TR
};

typedef struct _DEBUG_DR6_
{
	unsigned B0 : 1;//Dr0断点访问
	unsigned B1 : 1;//Dr1断点访问
	unsigned B2 : 1;//Dr2断点访问
	unsigned B3 : 1;//Dr3断点访问
	unsigned Reverted : 9;
	unsigned BD : 1;//有DEBUG寄存器访问引发的#DB异常
	unsigned BS : 1;//有单步引发的#DB异常
	unsigned BT : 1;//有TASK switch 任务切换引发的#DB异常
	unsigned Reverted2 : 16;
}DEBUG_DR6, *PDEBUG_DR6;

typedef struct _DEBUG_DR7_
{

	unsigned L0 : 1; //0 DR0断点#DB
	unsigned G0 : 1; //1
	unsigned L1 : 1; //2 DR1断点#DB
	unsigned G1 : 1; //3
	unsigned L2 : 1; //4 DR2断点#DB
	unsigned G2 : 1; //5
	unsigned L3 : 1; //6 DR3断点#DB
	unsigned G3 : 1; //7
	unsigned LE : 1; //8
	unsigned GE : 1; //9
	unsigned reserved : 3; //001  //10-11-12
	unsigned GD : 1; //13...允许对DEBUG寄存器访问产生#DB异常
	unsigned reserved2 : 2; //00
	unsigned RW0 : 2;//设置DR0访问类型 00B执行断点 01B写断点 10B IO读/写断点11B 读/写断点
	unsigned LEN0 : 2;//设置DR0字节长度 00B一个字节 01B WORD 10B QWORD 11B DWORD 
	unsigned RW1 : 2;//设置DR1访问类型
	unsigned LEN1 : 2;//设置DR1字节长度
	unsigned RW2 : 2;//设置DR2访问类型
	unsigned LEN2 : 2;//设置DR2字节长度
	unsigned RW3 : 2;//设置DR3访问类型
	unsigned LEN3 : 2;//设置DR3字节长度

}DEBUG_DR7, *PDEBUG_DR7;

typedef struct _INTERRUPT_INJECT_INFO_FIELD{
	unsigned Vector : 8;
	unsigned InterruptionType : 3;
	unsigned DeliverErrorCode : 1;
	unsigned Reserved : 19;
	unsigned Valid : 1;
} INTERRUPT_INJECT_INFO_FIELD, *PINTERRUPT_INJECT_INFO_FIELD;

typedef struct _INTERRUPT_IBILITY_INFO {
	unsigned STI : 1;
	unsigned MOV_SS : 1;
	unsigned SMI : 1;
	unsigned NMI : 1;
	unsigned Reserved : 27;
} INTERRUPT_IBILITY_INFO, *PINTERRUPT_IBILITY_INFO;


#define DIVIDE_ERROR_EXCEPTION 0
#define DEBUG_EXCEPTION 1
#define NMI_INTERRUPT 2
#define BREAKPOINT_EXCEPTION 3
#define OVERFLOW_EXCEPTION 4
#define BOUND_EXCEPTION 5
#define INVALID_OPCODE_EXCEPTION 6
#define DEVICE_NOT_AVAILABLE_EXCEPTION 7
#define DOUBLE_FAULT_EXCEPTION 8
#define COPROCESSOR_SEGMENT_OVERRUN 9
#define INVALID_TSS_EXCEPTION 10
#define SEGMENT_NOT_PRESENT 11
#define STACK_FAULT_EXCEPTION 12
#define GENERAL_PROTECTION_EXCEPTION 13
#define PAGE_FAULT_EXCEPTION 14
#define X87_FLOATING_POINT_ERROR 16
#define ALIGNMENT_CHECK_EXCEPTION 17
//#define MACHINE_CHECK_EXCEPTION 18
#define SIMD_FLOATING_POINT_EXCEPTION 19

#define EXTERNAL_INTERRUPT 0
#define HARDWARE_EXCEPTION 3
#define SOFTWARE_INTERRUPT 4
#define PRIVILEGED_SOFTWARE_EXCEPTION 5
#define SOFTWARE_EXCEPTION 6
#define OTHER_EVENT 7

typedef struct _INTERRUPT_INFO_FIELD {
	unsigned Vector : 8;
	unsigned InterruptionType : 3;
	unsigned ErrorCodeValid : 1;
	unsigned NMIUnblocking : 1;
	unsigned Reserved : 18;
	unsigned Valid : 1;
} INTERRUPT_INFO_FIELD, *PINTERRUPT_INFO_FIELD;

typedef struct
{
	USHORT limit0;
	USHORT base0;
	UCHAR  base1;
	UCHAR  attr0;
	UCHAR  limit1attr1;
	UCHAR  base2;
} SEGMENT_DESCRIPTOR2, *PSEGMENT_DESCRIPTOR2;


ULONG NTAPI VmxAdjustControls(
								ULONG64 Ctl,
								ULONG64 Msr
								);

NTSTATUS FillGuestSelectorData(ULONG64 GdtBase, ULONG Segreg, USHORT Selector);

NTSTATUS InitializeSegmentSelector( PSEGMENT_SELECTOR SegmentSelector, USHORT Selector, ULONG64 GdtBase );