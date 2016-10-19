#include "header.h"
#include "common.h"
#include "vtasm.h"
#include "vtsystem.h"

NTSTATUS InitializeSegmentSelector( PSEGMENT_SELECTOR SegmentSelector, USHORT Selector, ULONG64 GdtBase )
{
	PSEGMENT_DESCRIPTOR2 SegDesc;

	if( !SegmentSelector )
	{
		return STATUS_INVALID_PARAMETER;
	}

	//
	// 如果段选择子的T1 = 1表示索引LDT中的项, 这里没有实现这个功能
	//
	if( Selector & 0x4 )
	{
		
		return STATUS_INVALID_PARAMETER;
	}

	//
	// 在GDT中取出原始的段描述符
	//
	SegDesc = ( PSEGMENT_DESCRIPTOR2 )( ( PUCHAR ) GdtBase + ( Selector & ~0x7 ) );

	//
	// 段选择子
	//
	SegmentSelector->sel = Selector;

	//
	// 段基址15-39位 55-63位
	//
	SegmentSelector->base = SegDesc->base0 | SegDesc->base1 << 16 | SegDesc->base2 << 24;

	//
	// 段限长0-15位  47-51位, 看它的取法
	//
	SegmentSelector->limit = SegDesc->limit0 | ( SegDesc->limit1attr1 & 0xf ) << 16;

	//
	// 段属性39-47 51-55 注意观察取法
	//
	SegmentSelector->attributes.UCHARs = SegDesc->attr0 | ( SegDesc->limit1attr1 & 0xf0 ) << 4;

	//
	// 这里判断属性的DT位, 判断是否是系统段描述符还是代码数据段描述符
	//
	if( !( SegDesc->attr0 & LA_STANDARD ) )
	{
		ULONG64 tmp;

		//
		// 这里表示是系统段描述符或者门描述符, 感觉这是为64位准备的吧,
		// 32位下面段基址只有32位啊. 难道64位下面有什么区别了?
		//
		tmp = ( *( PULONG64 )( ( PUCHAR ) SegDesc + 8 ) );

		SegmentSelector->base = ( SegmentSelector->base & 0xffffffff ) | ( tmp << 32 );
	}

	//
	// 这是段界限的粒度位, 1为4K. 0为1BYTE
	//
	if( SegmentSelector->attributes.fields.g )
	{
		//
		// 如果粒度位为1, 那么就乘以4K. 左移动12位
		//
		SegmentSelector->limit = ( SegmentSelector->limit << 12 ) + 0xfff;
	}

	return STATUS_SUCCESS;
}

ULONG NTAPI VmxAdjustControls(
								ULONG64 Ctl,
								ULONG64 Msr
								)
{
	LARGE_INTEGER MsrValue;

	MsrValue.QuadPart = Asm_ReadMsr(Msr);
	Ctl &= MsrValue.HighPart;     /* bit == 0 in high word ==> must be zero */
	Ctl |= MsrValue.LowPart;      /* bit == 1 in low word  ==> must be one  */
	return Ctl;
}


NTSTATUS FillGuestSelectorData(ULONG64 GdtBase, ULONG Segreg, USHORT
							   Selector)
{
	SEGMENT_SELECTOR SegmentSelector = { 0 };
	ULONG uAccessRights;

	InitializeSegmentSelector(&SegmentSelector, Selector, GdtBase);
	uAccessRights = ((PUCHAR)& SegmentSelector.attributes)[0] + (((PUCHAR)&
		SegmentSelector.attributes)[1] << 12);

	if (!Selector)
		uAccessRights |= 0x10000;

	Vmx_VmWrite(GUEST_ES_SELECTOR + Segreg * 2, Selector & 0xFFF8);
	Vmx_VmWrite(GUEST_ES_BASE + Segreg * 2, SegmentSelector.base);
	Vmx_VmWrite(GUEST_ES_LIMIT + Segreg * 2, SegmentSelector.limit);
	Vmx_VmWrite(GUEST_ES_AR_BYTES + Segreg * 2, uAccessRights);


// 	if ((Segreg == LDTR) || (Segreg == TR))
// 		// don't setup for FS/GS - their bases are stored in MSR values
// 		Vmx_VmWrite(GUEST_ES_BASE + Segreg * 2, SegmentSelector.base);

	return STATUS_SUCCESS;
}