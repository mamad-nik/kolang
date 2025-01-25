.section .data
par_error_str: .asciz "expected a closing parenthesis" #31
.section .text
.global _parse_arith_exp
.extern _ascii_to_integer
.extern _err_exit	
_parse_arith_exp:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r12

	xorq %rbx, %rbx
	call _parse_exp
	movq %r12, %rax

	popq %r12
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	ret
_parse_exp:
	call _arith_skip_white_space
	call _parse_term
	exp_loop:
	cmpb $'\n', %al
	je _parse_done
	cmpb $'+', %al
	jne exp_sub
	call _handle_add
	jmp exp_loop
	exp_sub:
	cmpb $'-', %al
	jne exp_done
	call _handle_sub
	jmp exp_loop
	exp_done:
	ret
_handle_add:
	lodsb 
	call _arith_skip_white_space
	pushq %r12
	call _parse_term
	popq %rbx
	addq %rbx, %r12
	ret
_handle_sub:
	lodsb
	call _arith_skip_white_space
	pushq %r12
	call _parse_term
	popq %rbx
	subq %r12, %rbx
	movq %rbx, %r12
	ret
_parse_term:
	call _parse_factor
	pt_loop:
	call _arith_skip_white_space
	cmpb $'\n', %al
	je _parse_done

	cmpb $'*', %al
	jne pt_div
	call _handle_mul
	jmp pt_loop
	pt_div:
	cmpb $'/', %al
	jne pt_done
	call _handle_div
	jmp pt_loop
	pt_done:
	ret
_handle_mul:
	lodsb 
	call _arith_skip_white_space
	pushq %r12
	call _parse_factor
	popq %rbx
	push %rax
	movq %r12, %rax
	mulq %rbx
	movq %rax, %r12
	popq %rax
	ret
_handle_div:
	lodsb 
	call _arith_skip_white_space
	pushq %r12
	call _parse_factor
	movq %r12, %rax
	popq %rbx
	xchgq %rbx, %rax
	cqto
	idivq %rbx
	movq %rax, %r12
	ret
_parse_factor:
	cmpb $'(', %al
	jne pf_cont
	call _handle_par
	ret
	pf_cont:
	call _ascii_to_integer
	movq %rax, %r12
	movb -1(%rsi), %al
	ret

_handle_par:
	lodsb
	call _parse_exp
	call _arith_skip_white_space
	cmpb $')', %al
	jne par_error
	lodsb
	ret

	par_error:
	leaq par_error_str(%rip), %rax
	movq $31, %rdi
	call _err_exit
_parse_done:
	ret

_arith_skip_white_space:
	sws_loop:
		cmpb $' ', %al
		je act_loop

		cmpb $'\t', %al 
		je act_loop
		jmp sws_done
		act_loop:
		lodsb
		jmp sws_loop
	sws_done:
	ret
