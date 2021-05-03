.code16
.org 0

.text

.global _start
_start:
    cli #Clears interrupt flag

    movw $0x3000, %sp #Sets stack pointer

    sti

    call print_welcome_string

    jmp end

print_welcome_string:
    movw $welcome_string, %si
    call print
    ret
print: #Expects pointer to the string to be in si
    xorb %bh, %bh #xor the bh register
    movb $0xE, %ah #sets teletype mode for interrupt 0x10

    lodsb

    cmpb $0, %al #Checks if current character is NULL
    je print_ret #Jumps to return if the character is NULL

    int $0x10 #Prints character to the screen
    jmp print #Loops to beginning

print_ret:
    ret

end:
    call print_welcome_string #Loop prints U MAD BRO
    jmp end #Infinite loop (like a boss)


welcome_string:
    .asciz "U MAD BRO?"
welcome_string_end:


.fill 510-(.-_start), 1, 0
.word 0xAA55
