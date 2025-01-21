.section .data
.set byte, 1
.set bool, 2
.set int, 3
.set uint, 4
.set float, 5
.set string, 6
inv_type_str: .asciz "undefined type number"#22
noftypes: .quad 6
type_size: 
	.byte 1, 1, 8, 8, 4, 8
	.fill 506, 1, 0

.section .text
.extern _err_exit
.global _add_type
.global _resolve_type
.global type_system_init
#type_system_init:
	

_add_type:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r12
	movq %rax, %r12
	movq noftypes(%rip), %rbx
	leaq type_size(%rip), %rax
	movq %r12, (%rax, %rbx)
	movq noftypes(%rip), %rax
	incq noftypes(%rip)
	popq  %r12
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
_resolve_type:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	cmpq noftypes(%rip), %rax
	jge invalid_type
	leaq type_size(%rip), %rbx
	movb (%rbx, %rax), %al
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	ret
	invalid_type:
	movq inv_type_str(%rip), %rax
	movq $22, %rdi
	call _err_exit
