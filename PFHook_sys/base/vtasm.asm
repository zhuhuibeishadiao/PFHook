GetGuestRegsAddress Proto
VMMEntryPoint Proto
SetupVMCS Proto

.data
GuestRSP qword ?
GuestReturn qword ?

EntryRAX qword ?
EntryRCX qword ?
EntryRDX qword ?
EntryRBX qword ?
EntryRSP qword ?
EntryEBP qword ?
EntryESI qword ?
EntryRDI qword ?
EntryR8 qword ?
EntryR9 qword ?
EntryR10 qword ?
EntryR11 qword ?
EntryR12 qword ?
EntryR13 qword ?
EntryR14 qword ?
EntryR15 qword ?
EntryRflags qword ?

.code

Asm_CPUID	Proc
	push	rbp
	mov		rbp, rsp
	push	rbx
	push	rsi

	mov		[rbp+18h], rdx
	mov		eax, ecx
	cpuid
	mov		rsi, [rbp+18h]
	mov		[rsi], eax
	mov		[r8], ebx
	mov		[r9], ecx
	mov		rsi, [rbp+30h]
	mov		[rsi], edx	

	pop		rsi
	pop		rbx
	mov		rsp, rbp
	pop		rbp
	ret
Asm_CPUID 	Endp

Asm_ReadMsr		Proc
	xor rax,rax
	xor rdx,rdx
	
	rdmsr
	shl rdx,32
	or rax,rdx
	ret
Asm_ReadMsr		Endp

Asm_WriteMsr	Proc
	xor rax,rax
	
	mov	eax,edx
	shr rdx,32
	wrmsr
	ret
Asm_WriteMsr 	Endp

Asm_Invd Proc
	invd
	ret
Asm_Invd Endp

Asm_GetCs PROC
	mov		rax, cs
	ret
Asm_GetCs ENDP

Asm_GetDs PROC
	mov		rax, ds
	ret
Asm_GetDs ENDP

Asm_GetEs PROC
	mov		rax, es
	ret
Asm_GetEs ENDP

Asm_GetSs PROC
	mov		rax, ss
	ret
Asm_GetSs ENDP

Asm_GetFs PROC
	mov		rax, fs
	ret
Asm_GetFs ENDP

Asm_GetGs PROC
	mov		rax, gs
	ret
Asm_GetGs ENDP


Asm_GetCr0		Proc
	mov 	rax, cr0
	ret
Asm_GetCr0 		Endp

Asm_GetCr3		Proc
	
	mov 	rax, cr3
	ret
Asm_GetCr3 		Endp

Asm_GetCr4		Proc
	mov 	rax, cr4
	ret
Asm_GetCr4 		Endp

Asm_SetCr0		Proc
	mov	cr0, rcx
	ret
Asm_SetCr0 		Endp

Asm_SetCr2		Proc
	mov	cr2, rcx
	ret
Asm_SetCr2 		Endp

Asm_SetCr3		Proc
	mov	cr3, rcx
	ret
Asm_SetCr3 		Endp

Asm_SetCr4		Proc
	mov cr4, rcx
	ret
Asm_SetCr4 		Endp

Asm_GetDr0 PROC
	mov		rax, dr0
	ret
Asm_GetDr0 ENDP

Asm_GetDr1 PROC
	mov		rax, dr1
	ret
Asm_GetDr1 ENDP

Asm_GetDr2 PROC
	mov		rax, dr2
	ret
Asm_GetDr2 ENDP

Asm_GetDr3 PROC
	mov		rax, dr3
	ret
Asm_GetDr3 ENDP

Asm_GetDr6 PROC
	mov		rax, dr6
	ret
Asm_GetDr6 ENDP

Asm_GetDr7 PROC
	mov		rax, dr7
	ret
Asm_GetDr7 ENDP

Asm_SetDr0 PROC
	mov		dr0, rcx
	ret
Asm_SetDr0 ENDP

Asm_SetDr1 PROC
	mov		dr1, rcx
	ret
Asm_SetDr1 ENDP

Asm_SetDr2 PROC
	mov		dr2, rcx
	ret
Asm_SetDr2 ENDP

Asm_SetDr3 PROC
	mov		dr3, rcx
	ret
Asm_SetDr3 ENDP

Asm_SetDr6 PROC
	mov		dr6, rcx
	ret
Asm_SetDr6 ENDP

Asm_SetDr7 PROC
	mov		dr7, rcx
	ret
Asm_SetDr7 ENDP

Asm_GetRflags PROC
	pushfq
	pop		rax
	ret
Asm_GetRflags ENDP

Asm_GetIdtBase PROC
	LOCAL	idtr[10]:BYTE
	
	sidt	idtr
	mov		rax, qword PTR idtr[2]
	ret
Asm_GetIdtBase ENDP

Asm_GetIdtLimit PROC
	LOCAL	idtr[10]:BYTE
	
	xor rax,rax
	sidt	idtr
	mov		ax, WORD PTR idtr[0]
	ret
Asm_GetIdtLimit ENDP

Asm_GetGdtBase PROC
	LOCAL	gdtr[10]:BYTE

	sgdt	gdtr
	mov		rax, qword PTR gdtr[2]
	ret
Asm_GetGdtBase ENDP

Asm_GetGdtLimit PROC
	LOCAL	gdtr[10]:BYTE
	
	xor rax,rax
	sgdt	gdtr
	mov		ax, WORD PTR gdtr[0]
	ret
Asm_GetGdtLimit ENDP

Asm_GetLdtr PROC
	sldt	rax
	ret
Asm_GetLdtr ENDP

Asm_GetTr PROC
	str	rax
	ret
Asm_GetTr ENDP

Asm_SetGdtr		Proc
	push	rcx
	shl	rdx, 16
	push	rdx
	
	lgdt	fword ptr [rsp+2]
	pop	rax
	pop	rax
	ret
Asm_SetGdtr	Endp

Asm_SetIdtr		Proc
	push	rcx
	shl	rdx, 16
	push	rdx
	lidt	fword ptr [rsp+2]
	pop	rax
	pop	rax
	ret
Asm_SetIdtr	Endp

Asm_Rdtsc Proc
	mov rbx,rcx
	mov rsi,rdx

	xor rax,rax
	xor rdx,rdx

	rdtsc
	mov [rbx],rax
	mov [rsi],rdx
	ret
Asm_Rdtsc Endp

Asm_GetTSC PROC
;	rdtscp
	rdtsc
	shl		rdx, 32
	or		rax, rdx
	ret
Asm_GetTSC ENDP


Vmx_VmxOn Proc
	push rcx
	Vmxon qword ptr [rsp]
	add rsp,8
	ret
Vmx_VmxOn Endp

Vmx_VmxOff Proc
	Vmxoff
	ret
Vmx_VmxOff Endp

Vmx_VmPtrld Proc
	push rcx
	vmptrld qword ptr [rsp]
	add rsp,8
	ret
Vmx_VmPtrld endp

Vmx_VmClear Proc
	push rcx
	vmclear qword ptr [rsp]
	add rsp,8
	ret
Vmx_VmClear endp

Vmx_VmRead Proc
	mov rax,rcx
	vmread rcx,rax
	mov rax,rcx
	ret
Vmx_VmRead endp

Vmx_VmWrite Proc
	mov rax,rcx
	mov rcx,rdx
	vmwrite rax,rcx
	ret
Vmx_VmWrite endp

; CmInvept (PVOID Ep4ta(rcx), ULONG inval (rdx) );
Vmx_Invept proc
     push	 rbp
	 mov	 rbp, rsp
	 push    rsi
	 mov     rsi, rcx
	 mov     rax, rdx
	 invept  rax, xmmword ptr [rsi]
	 pop     rsi
	 mov	 rsp, rbp
	 pop	 rbp
	 ret
Vmx_Invept endp

Vmx_VmCall Proc
	push rax
	push rcx
	push rdx
	push rbx
	push rsp     ;HOST_RSP
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15 ;pushaq
	
	
	pushfq
	mov rax,rcx
	vmcall
	
	popfq
	
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rsp
	pop rbx
	pop rdx
	pop rcx
	pop rax ;popaq
	
	ret
Vmx_VmCall endp

Vmx_VmLaunch Proc
	vmlaunch
	ret
Vmx_VmLaunch endp

Vmx_VmResume Proc
	vmresume
	ret
Vmx_VmResume endp

Asm_GetGuestRSP Proc
	mov rax,GuestRSP
	ret
Asm_GetGuestRSP Endp

Asm_GetGuestReturn Proc
	mov rax,GuestReturn
	ret
Asm_GetGuestReturn Endp

Asm_AfterVMXOff Proc
	mov rsp,rcx
	jmp rdx
	ret
Asm_AfterVMXOff Endp

Asm_RunToVMCS Proc
	mov rax,[rsp]
	mov GuestReturn,rax ;获取返回地址，让vmlaunch后客户机继续执行驱动加载的代码
	
	call SetupVMCS
	ret
Asm_RunToVMCS Endp

Asm_SetupVMCS Proc
	cli
	mov GuestRSP,rsp
	
	mov EntryRAX,rax
	mov EntryRCX,rcx
	mov EntryRDX,rdx
	mov EntryRBX,rbx
	mov EntryRSP,rsp
	mov EntryEBP,rbp
	mov EntryESI,rsi
	mov EntryRDI,rdi
	mov EntryR8,r8
	mov EntryR9,r9
	mov EntryR10,r10
	mov EntryR11,r11
	mov EntryR12,r12
	mov EntryR13,r13
	mov EntryR14,r14
	mov EntryR15,r15
	
	pushfq
	pop EntryRflags
	
	call Asm_RunToVMCS
	
	push EntryRflags
	popfq
	mov rax,EntryRAX
	mov rcx,EntryRCX
	mov rdx,EntryRDX
	mov rbx,EntryRBX
	mov rsp,EntryRSP
	mov rbp,EntryEBP
	mov rsi,EntryESI
	mov rdi,EntryRDI
	mov r8,EntryR8
	mov r9,EntryR9
	mov r10,EntryR10
	mov r11,EntryR11
	mov r12,EntryR12
	mov r13,EntryR13
	mov r14,EntryR14
	mov r15,EntryR15
	
	mov rsp,GuestRSP
	sti
	ret
Asm_SetupVMCS Endp

Asm_VMMEntryPoint Proc
	cli
	push rax
	push rcx
	push rdx
	push rbx
	push rsp     ;HOST_RSP
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	
	mov [rsp-1280h],rax
	mov [rsp-1288h],rbx
	mov [rsp-1290h],rcx
	call GetGuestRegsAddress
	mov rcx,[rsp-1290h]
	mov [rax+8h],rcx
	mov [rax+10h],rdx
	mov [rax+18h],rbx
	mov [rax+20h],rsp
	mov [rax+28h],rbp
	mov [rax+30h],rsi
	mov [rax+38h],rdi
	mov [rax+40h],r8
	mov [rax+48h],r9
	mov [rax+50h],r10
	mov [rax+58h],r11
	mov [rax+60h],r12
	mov [rax+68h],r13
	mov [rax+70h],r14
	mov [rax+78h],r15
	mov rbx,[rsp-1280h]
	mov [rax],rbx
	mov rax,[rsp-1280h]
	mov rbx,[rsp-1288h]
	
	call VMMEntryPoint
	
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rsp
	pop rbx
	pop rdx
	pop rcx
	pop rax
	
	call GetGuestRegsAddress
	mov rcx,[rax+8h]
	mov rdx,[rax+10h]
	mov rbx,[rax+18h]
	mov rsp,[rax+20h]
	mov rbp,[rax+28h]
	mov rsi,[rax+30h]
	mov rdi,[rax+38h]
	mov r8,[rax+40h]
	mov r9,[rax+48h]
	mov r10,[rax+50h]
	mov r11,[rax+58h]
	mov r12,[rax+60h]
	mov r13,[rax+68h]
	mov r14,[rax+70h]
	mov r15,[rax+78h]
	mov rax,[rax]
	sti
	vmresume
Asm_VMMEntryPoint Endp

END