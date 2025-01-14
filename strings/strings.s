.section .data
.set max_length, 512
.set total_length, 528
.set string_type_number, 6
.set dq, '"'
.set fs, '\\'
mmap_err_string: .asciz "mmap error" #11
long_string_err: .asciz "string too long" #16
error_validate_string_str:   .asciz "expected string, but it must be enclosed into double quotes\n" #61

.section .text
.extern _exit
.global _create_string

_validate_string:
	cmpb $dq, %al
	jne error_validate_string
	movq $max_length, %rcx

	pstr_loop:
		lodsb
		# if you've found the second double qoute, we've got outselves a string!
		cmpb $dq, %al
		je pstr_done
		stosb
		loop pstr_loop
		
	movq $1, %rax
	movq $1, %rdi
	leaq long_string_err(%rip), %rsi
	movq $16, %rdx
	call _exit
	
	pstr_done:
		movq $max_length, %rax
		subq %rcx, %rax
		ret	
	error_validate_string:
		movq $1, %rax
		movq $1, %rdi
		leaq error_validate_string_str(%rip), %rsi
		movq $6, %rdx
		call _exit
	
_create_string:
	pushq %rbp
	movq %rsp, %rbp 
	pushq %rbx
	pushq %r12
	pushq %r13
	movq %rax, %r12
	movq %rsi, %r13

	movq $9, %rax
	xorq %rdi, %rdi
	movq $total_length, %rsi
	movq $0x03, %rdx
	movq $0x22, %r10
	xorq %r8, %r8
	decq %r8
	xorq %r9, %r9
	syscall
	
	test %rax, %rax
	jz mmap_error
	
	pushq %rax
	movq $string_type_number, (%rax)
	addq $16, %rax
	movq %rax, %rdi
	movq %r12, %rax
	movq %r13, %rsi
	call _validate_string
	popq %rbx
	movq %rax, 8(%rbx)
	movq %rbx, %rax

	popq %r13
	popq %r12
	movq %rbp, %rsp
	popq %rbp
	ret
	
	mmap_error:
		movq $1, %rax
		movq $1, %rdi
		movq $mmap_err_string, %rsi
		movq $11, %rdx
		syscall
		#replace with _exit
		movq $60, %rax
		movq $1, %rdi
		syscall
		ret
