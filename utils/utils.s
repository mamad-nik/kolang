.section .data
.set nl, '\n'
.set sp, ' '
.set tb, '\t'
.section .text
.global _exit
.global _err_exit
.global _integer_to_ascii
.global _skip_white_space
.global _white_space
_exit:
	movq $60, %rax
	movq $0, %rdi
	syscall
_err_exit:
	movq %rax, %rsi
	movq %rdi, %rdx
	movq $1, %rax
	movq $1, %rdi
	syscall

	movq $60, %rax
	movq $1, %rdi
	syscall

_white_space:
	cmpb $sp, %al
	je ws_ret

	cmpb $tb, %al 
	je ws_ret

	cmpb $nl, %al
	je ws_ret

	ret
	ws_ret:
	xorq %rax, %rax
	ret

_skip_white_space:
	sws_loop:
		lodsb
		cmpb $sp, %al
		je sws_loop

		cmpb $tb, %al 
		je sws_loop

		cmpb $nl, %al
		je sws_loop

	ret

# BEWARE: prints gibberish unless given integer number
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
	pushq %rbx
	pushq %rdi
	decq %rdi
	itoa_reverse_loop:
	movb (%rbx), %r12b
	xchgb (%rdi), %r12b
	movb %r12b, (%rbx)
	incq %rbx
	decq %rdi
	cmpq %rbx, %rdi
	jge itoa_reverse_loop

	popq %rdi
	movq $0, (%rdi)

	popq %rax

	itoa_done:
	popq %r12
	popq %rbx
	ret
