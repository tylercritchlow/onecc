; Print Hello world, then halt.
; B = -1 prints the byte stored at A. C is ignored for output.
start:
    subleq H, -1, 0
    subleq e, -1, 0
    subleq l, -1, 0
    subleq l, -1, 0
    subleq o, -1, 0
    subleq space, -1, 0
    subleq w, -1, 0
    subleq o, -1, 0
    subleq r, -1, 0
    subleq l, -1, 0
    subleq d, -1, 0
    subleq newline, -1, 0
    subleq zero, zero, -1

; ASCII character codes.
H:       .word 72
e:       .word 101
l:       .word 108
o:       .word 111
space:   .word 32
w:       .word 119
r:       .word 114
d:       .word 100
newline: .word 10
zero:    .word 0
