# Since this is the first (or even zeroth) form of the compiler for a subset of the language
# adhere to all compiler implementation, since doing so in assembly would be cumbersome and ineficient.
# as an example of that, the the syntax and semantics analyzers are tightly coupled, which is not recommended practice.
.section .data 
.set nl, '\n'
.set sp, ' '
.set tb, '\t'
.set dq, '"'
.set fs, '\\'
# Keywords
program: .ascii "program" #7
proc: .ascii "proc" #4
if:   .ascii "if" #2
for:  .ascii "for" #3
load: .ascii "load" #4
# end of Keywords

err_message: .asciz "error reading\n" #17
error_invalid_symbol_str: .asciz "invalid symbol or unexpected null char\n" #40
error_program_str: .asciz "program shoud start with program keyword\n" #42
error_parse_string: .asciz "expected string, but it must be enclosed into double quotes\n" #61
temp: .ascii "program mamad\n" #5
match: .asciz "match\n" #6
.set buff_size, 4096

.section .bss
buffer: .skip buff_size

.text 
.global _start

# check for white spaces
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
	
# skip white spaces (it is actually more efficient to rewrite the code instead of calling the fuction above)
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
# parse string TODO: figure out a way to escape characters.
_parse_string:
	cmpb $dq, %al
	jne error_parse_string

	pstr_loop:
		lodsb
		# if you've found the second double qoute, we've got outselves a string!
		cmpb $dq, %al
		je pstr_done

		cmpb $fs, %al
		je pstr_skip_escape

		#lodsb 
		#cmpb $dq, %al
		#jne 
		#stosb
		#stosw


		jmp pstr_loop

	pstr_done:
		ret	
	error_parse_string:
		movq $1, %rax
		movq $1, %rdi
		leaq error_parse_string(%rip), %rsi
		movq $61, %rdx
		call _exit
	

# parses symbols. it'a equivalent to this regex: `[a-zA-Z][a-zA-Z0-9]*`. it uses range cheking
_parse_symbol:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	
	first_part:
		cmpb $'A', %al
		jb error_invalid_symbol
		cmpb $'Z', %al
		jbe second_part

		cmpb $'a', %al
		jb  error_invalid_symbol
		cmpb $'z', %al
		jbe second_part

		jmp error_invalid_symbol
		
	second_part:
		ps_sp_loop:
			#stosb 
			lodsb
			cmpq $0, %rax
			je error_invalid_symbol

			call _white_space
			cmpq $0, %rax
			je symbol_match
			
			cmpb $'0', %al
			jb error_invalid_symbol
			cmpb $'9', %al
			jbe second_part

			cmpb $'A', %al
			jb error_invalid_symbol
			cmpb $'Z', %al
			jbe second_part

			cmpb $'a', %al
			jb  error_invalid_symbol
			cmpb $'z', %al
			jbe second_part


	symbol_match:
		popq %rbx 
		movq %rbp, %rsp
		popq %rbp
		
		movq $1, %rax
		movq $1, %rdi
		leaq match(%rip), %rsi
		movq $7, %rdx
		syscall
		
		ret
	error_invalid_symbol:
		movq $1, %rax
		movq $1, %rdi
		leaq error_invalid_symbol_str(%rip), %rsi
		movq $40, %rdx
		syscall

		call _exit

# it is used to parse the keyword "program"	and its binded name. TODO: add the name to the global table.
_parse_program:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	
	leaq program(%rip), %rbx
	# since program is 7 letters long
	movq $7, %rcx
	
	# check if the char is as expected.
	pp_loop:
		lodsb
		cmpb (%rbx), %al
		# if not, the program does not abide syntax rules and an error is generated.
		jne error_program

		inc %rbx
		loop pp_loop
	
	# skip the white spaces in between the keyword and the given name.
	call _skip_white_space
	# now check for the symbol.
	call _parse_symbol

	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	ret

	error_program:
		movq $1, %rax
		movq $1, %rdi
		leaq error_program_str(%rip), %rsi
		movq $42, %rdx
		syscall

		call _exit
		

#_parse:


#_open_file:
#	# Get the pointer to the filename as an argument
#	movq 24(%rsp), %rdi
#
#	movq $2, %rax
#	movq $0, %rsi 
#	xorq %rdx, %rdx
#	syscall
#	testq %rax, %rax
#	
#	movq %rax, %r10
#loop:
#	movq $0, %rax
#	movq %r10, %rdi
#	leaq buffer(%rip), %rsi
#	movq $buff_size, %rdx
#	syscall
#	
#	testq %rax, %rax
#	js read_error
#	jz done
#
#	call _parse
#
#	movq %rax, %rdx
#	movq $1, %rax
#	movq $1, %rdi
#	leaq buffer(%rip), %rsi
#	syscall
#	
#	jmp loop
#done:
#	ret
#
#read_error:
#	movq $1, %rax
#	movq $1, %rdi
#	leaq err_message(%rip), %rsi
#	movq $14, %rdx
#	ret
#	
_exit:
	movq $60, %rax
	movq $0, %rdi
	syscall

_start:
	#cmpq $2, (%rsp)
	#jl _exit
	leaq buffer(%rip), %rdi
	leaq temp(%rip), %rsi
	call _parse_program

	call _exit

