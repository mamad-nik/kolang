.section .data
.set max, 0x8000000000000000
.set min, 0x7FFFFFFFFFFFFFFF
negative: .byte 0
error_invalid_integer_str: .asciz "invalid integer"#16
signed_overflow_str: .asciz "too big an integer"#19
.section .text
.extern _err_exit
.extern _white_space
.global _parse_integer
_parse_integer:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r12
	xorq %rbx, %rbx
	xorq %r12, %r12

	cmpb $'-', %al
	jne parse_integer_loop
	incb negative(%rip)
	lodsb
	parse_integer_loop:
	movb %al, %r12b	
	call _white_space
	testq %rax, %rax
	jz parse_integer_negative
	movb %r12b, %al
	cmpb $48, %al
	jl error_invalid_integer
	cmpb $57, %al
	jg error_invalid_integer

	subb $48, %al	
	movb %al, %r12b
	
	movq $10, %rax
	mulq %rbx
	movq %rax, %rbx
	addq %r12, %rbx
	jo signed_overflow
	lodsb
	jmp parse_integer_loop
	parse_integer_negative:
	cmpb $0, negative(%rip)
	je parse_integer_done
	negq %rbx
	jo signed_overflow

	parse_integer_done:
	movq %rbx, %rax
	popq %r12
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	ret
	error_invalid_integer:
	leaq error_invalid_integer_str(%rip), %rax
	movq $16, %rdi
	call _err_exit
	signed_overflow:
	leaq signed_overflow_str(%rip), %rax
	movq $19, %rdi
	call _err_exit

#_ascii_to_integer:
#	pushq %rbp
#	movq %rsp, %rbp
#	pushq %rbx
#	popq %rbx
#	movq %rbp, %rsp
#	popq %rbp
#	ret
