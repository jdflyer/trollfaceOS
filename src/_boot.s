.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .data
gdtr:
.word gdt_start - gdt_end - 1 #size
.long gdt_start #offset

gdt_start:
.quad 0 #blank entry
.word 0xffff #gdt code entry
.word 0x0000
.byte 0x00
.byte 0b10011010
.byte 0b11001111
.byte 0x00
.word 0xffff #gdt data entry
.word 0x0000
.byte 0x00
.byte 0b10011010
.byte 0b11001111
.byte 0x00
gdt_end:

.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    cli

    #pre-kernel init
    lgdt gdtr

    mov $stack_top, %esp #setup stack


    call kmain
loop:
    hlt
    jmp loop


.size _start, . - _start
