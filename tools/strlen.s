; might need tweaking inorder to work
.section .text
.global _get_length

_get_length:
	#calculate string length
	xorq %rax, %rax
	movq $-1, %rcx

	repne scasb

	notq %rcx
	decq %rcx
	   
	movq %rcx, %rax
	ret


