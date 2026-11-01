; Read and echo bytes until EOF. Input/output instructions ignore C.
; Input stores bytes as 0-255, or -1 at EOF.
loop:
    subleq -1, character, 0
    subleq minus_one, character, done ; Add 1: only EOF becomes zero.
    subleq one, character, output     ; Restore the original byte.
output:
    subleq character, -1, 0
    subleq zero, zero, loop
done:
    subleq zero, zero, -1

character: .word 0
minus_one: .word -1
one:       .word 1
zero:      .word 0
