.section .text
.global _exit
.global _integer_to_ascii
_exit:
	movq $60, %rax
	movq $0, %rdi
	syscall

# BEWARE: prints gibberish unless given integer number
_digit_to_ascii:
	addb $48, %al
	ret
_integer_to_ascii:
	pushq %rbx 
	pushq %r12 
	movq %rdi, %rbx

	movq $10, %rcx

	cmpq $0, %rax
	jge itoa_zero
	movb $'-', (%rdi)
	incq %rdi
	negq %rax

	itoa_zero:
	test %rax, %rax
	jne itoa_loop
	movb $'0', (%rdi)
	incq %rdi
	jmp itoa_done
	
	itoa_loop:
	testq %rax, %rax
	jz itoa_reverse	
	xorq %rdx, %rdx
	divq %rcx
	
	addb $48, %dl
	movb %dl, (%rdi)	
	incq %rdi
	jmp itoa_loop	

	itoa_reverse:
	pushq %rdi
	itoa_reverse_loop:
	decq %rdi
	movb (%rbx), %r12b
	xchgb (%rdi), %r12b
	movb %r12b, (%rbx)
	incq %rbx
	decq %rdi
	cmpq %rbx, %rdi
	jl itoa_reverse_loop
	popq %rdi
	movq $0, (%rdi)

	itoa_done:
	popq %r12
	popq %rbx
	ret
