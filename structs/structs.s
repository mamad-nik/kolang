.section .data
.section .bss
.section .text
.global _create_struct
.set struct_header_size, 8
#rdi: size
_create_struct:
	pushq %rbp 
	movq %rsp, %rbp

	movq $9, %rax
	movq %rdi, %rsi
	xorq %rdi, %rdi
	movq $0x3, %rdx 
	movq $0x22, %r10 
	xorq %r8, %r8 
	decq %r8 
	xorq %r9, %r9
	syscall
	
	.create_struct_exit:
	popq %rbp
	ret
	
#rdi: address, rsi: size
_destroy_struct:
	pushq %rbp 
	movq %rsp, %rbp

	movq $11, %rax
	syscall

	popq %rbp
	ret
