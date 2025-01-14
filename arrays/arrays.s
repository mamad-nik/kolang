.set defualt_size, 10
.section .text
.general _create_array
.general _destroy_array
.general _resize_array

_create_array:
	pushq %rbp
	movq %rsp, %rbp 
	pushq %rbx
	pushq %rax 

	mulq $defualt_size
	movq %rax, %rbx

	movq $9, %rax
	xorq %rdi, %rdi
	movq %rbx, %rsi		
	movq $0x3, %rdx 
	movq $0x22, %r10 
	xorq %r8, %r8 
	decq %r8
	xorq %r9, %r9
	syscall

	movq %rbx, (%rax)
	popq 8(%rax)
	movq $24, %rax

	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	ret
