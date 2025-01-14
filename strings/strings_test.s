.section .data
temp: .ascii "\"string\"" 
.section .text
.global _start
.extern _create_string
.extern _exit
_start:
	#movq %rsp, %rbp
	leaq temp(%rip), %rsi
	lodsb
	call _create_string

	movq %rax, %rbx
	movq $1, %rax
	movq $1, %rdi
	movq %rbx, %rsi
	addq $16, %rsi
	movq 8(%rbx), %rdx
	addq $2, %rdx
	syscall

	call _exit

