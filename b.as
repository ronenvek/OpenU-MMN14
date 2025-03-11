; error detection example

; error - can't use a missing label as an entry
.entry MISSING

.extern EXISTS

; error - can't define conserved words
.define hlt = 1

; error - wrong define syntax
.define len, 9

; error - invalid number
.define size = 2a

; warning - labeling a define statement
label: .define len = 12


; error - label longer than 31 chars
AVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryLongLabel: mov r0, r1

; error - wrong number of arguments
L0: dec r1, #2

; error - wrong argument types
L1: jmp #100

; error - missing comma
L2: sub EXISTS r3

; error - undefined arguments
L3: inc MISSING

; error - unknown operation
L4: abc #12

END: hlt


; error - can't make an extern and a label with the same name
EXISTS: .data 1

; error - cant redefine the label
END: .string "aaabbbccc"

; error - the label and the : must be attached
   LIST1 :   .string "abc"
   
