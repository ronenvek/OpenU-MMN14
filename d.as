; some program


.entry LIST 
.extern LIST2

.define size = 2 

MAIN:	mov #0, r0
		mcr macro
		prn #size
		mov #size, LIST[size]
		endmcr
		add LIST[0], r0
		add LIST[1], r0
L3:		dec r0
		macro
		cmp LIST2[0], r0
		bne LIST2
END:	hlt


STR: .string "banana" 
LIST: .data size, 70, -100

.entry L3
