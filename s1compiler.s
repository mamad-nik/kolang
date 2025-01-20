# Since this is the first (or even zeroth) form of the compiler for a subset of the language, it does not
# adhere to all compiler implementation best practices, since doing so in assembly would be cumbersome and ineficient.
# as an example of that, the the syntax and semantics analyzers are tightly coupled, which is not recommended practice.
.section .data 
.set nl, '\n'
.set sp, ' '
.set tb, '\t'
.set dq, '"'
.set fs, '\\'
.set buff_size, 4096

# Keywords
program: .ascii "program" #7
inline:  .ascii "inline" #6
proc: 	 .ascii "proc" #4
load: 	 .ascii "load" #4
ret: 	 .ascii "ret" #3
for: 	 .ascii "for" #3
var: 	 .ascii "var" #3
if: 	 .ascii "if" #2
# end of Keywords
# symbol table form: name | type
error_program_str: 		  .asciz "program shoud start with the program keyword\n" #46
error_invalid_symbol_str: .asciz "invalid symbol or unexpected null char\n" #40
_error_create_file_str: .asciz "error calling openat(), retry\n" #31

temp:  .ascii "program mamad\n" #5
match: .asciz "match\n" #6

.section .bss
buffer: .skip buff_size
program_name: .fill 64
.text 
.extern _create_symbol_table
.extern _destroy_symbol_table
.extern _update_symbol_table
.extern _exit
.extern _err_exit
.extern _skip_white_space
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
# parse string TODO: figure out a way to escape characters.
	

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
			stosb 
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
		movq $0, (%rdi)
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
	leaq program_name(%rip), %rdi
	call _parse_symbol

	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	ret

	error_program:
		movq $1, %rax
		movq $1, %rdi
		leaq error_program_str(%rip), %rsi
		movq $46, %rdx
		syscall

		call _exit
		

#_parse:


_create_file:
	movq $257, %rax
	movq $-100, %rdi
	leaq program_name(%rip), %rsi
	movq $0x41, %rdx
	movq $0444, %r10
	syscall

	cmpq $0, %rax
	jl _error_create_file
	ret
	_error_create_file:
		movq $1, %rax
		movq $1, %rdi
		movq _error_create_file_str(%rip), %rdi
		movq $31 , %rdx
		syscall

		call _err_exit
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
_start:
	#cmpq $2, (%rsp)
	#jl _exit
	leaq temp(%rip), %rsi
	call _parse_program

	call _exit

