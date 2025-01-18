.section .data
.set byte 1
.set bool 2
.set int 3
.set uint 4
.set float 5
.set string 6
type_size: 
	.word $byte, $bool, $int, $uint, $float, $string
	.fill 500, 1, 0

.section .text
.global _add_type
.global _resolve_type

#_add_type:
_resolve_type:

