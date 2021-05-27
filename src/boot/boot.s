.code16
.org 0

.text

.global _start
_start:
    cli #Clears interrupt flag

    mov %cs, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    movw $0x3000, %sp #Sets stack pointer

    mov %dl, drive_num

    sti

read_drive_params:
    mov $8, %ah #int 0x13 param for getting drive paramaters
    movb drive_num, %dl #puts drive number in DL
    int $0x13 #Executes

    movb %dh, num_heads

    andb $0x3f, %cl
    movb %cl, sectors_per_track

setup_drive_read:
    movw $0x100, %cx #Set how many sectors to read
    movw $0x1000, segment
    movw $0x0000, offset
    movw $1, sector
    movw $2, chs_sector
load_sector:
    push %cx #Push onto the stack

    movb $0x42, %ah #Sets up int 0x13 for extended read sectors
    movb drive_num, %dl #Sets the drive number to read from
    movw $int_0x13_struct, %si #Moves the pointer for the read data
    int $0x13 #Executes
    jnc read_success #If CF == 0 than read was a success
    #If fails attempt to use int 0x13 with param 0x2

    mov $extended_read_sector_error_string, %si
    call print

    movb $0x2, %ah #Sets up int 0x13 for read sectors
    movb $0x1, %al #Number of sectors to read
    movb chs_cylinder, %ch #Sets the cylinder to read from
    movb chs_sector, %cl #Sets the sector to read from
    movb drive_num, %dl #Sets the drive number to read from
    movb chs_head, %dh #Sets the head to read from
    movw segment, %es #Sets the segment to put the results into
    movw offset, %bx #Sets the offset to put the results into
    int $0x13 #Execute
    jnc read_success #Jump to read success if read was a success, else print disk error message
disk_error:
    movw $disk_error_string, %si #Move disk error string to SI
    call print #Prints disk error string
    jmp inf_loop #Jumps to inf loop


read_success:
    incb sector #Increase sector to read from

    incb chs_sector #Increase chs sector

    movb sectors_per_track, %al
    cmp %al, chs_sector #Checks if the current sector has reached the limit
    jna sector_same_head_cylinder #Jump if the chs sector is less than the limit

    #Sector limit broken
    movb $1, chs_sector
    incb chs_head
    mov num_heads, %al
    cmp %al, chs_head
    jna sector_same_head_cylinder

    #Head count broken; reset to 0 and increase cylinder

    movb $0, chs_head
    incb chs_cylinder

sector_same_head_cylinder:
    addw $0x200, offset #Increase offset by 512 bytes (1 sector)
    jnc sector_same_segment

    addw $0x1000, segment #Increase segment and reset offset if on a different segment
    movw $0x0, offset
sector_same_segment:
    pop %cx
    loop load_sector
drive_read_done:
    call print_welcome_string
set_video_mode:
    movb $0x00, %ah
    movb $0x13, %al
    int $0x10
enable_A20:
    cli #Clear interrupt flag

    /* read and save state */
    call enable_a20_wait0
    movb $0xD0, %al
    outb $0x64
    call enable_a20_wait1
    xorw %ax, %ax
    inb $0x60

    /* write new state with A20 bit set (0x2) */
    pushw %ax
    call enable_a20_wait0
    movb $0xD1, %al
    outb $0x64
    call enable_a20_wait0
    popw %ax
    orw $0x2, %ax
    outb $0x60

    /* enable PE flag */
    movl %cr0, %eax
    orl $0x1, %eax
    movl %eax, %cr0

    jmp flush #flush

enable_a20_wait0:
    xorw %ax, %ax
    inb $0x64
    btw $1, %ax
    jc enable_a20_wait0
    ret

enable_a20_wait1:
    xorw %ax, %ax
    inb $0x64
    btw $0, %ax
    jnc enable_a20_wait1
    ret

flush:
    lidt idt
    lgdt gdtp

    movw $(gdt_data_segment - gdt_start), %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
jump_to_32bit:
    movl $0x3000, %esp #Sets top of the stack
    ljmp $0x8, $entry32

.code32
entry32:
    movl $0x10000, %eax
    jmpl *%eax #Jump to kernel start

_32bit_loop:
    jmp _32bit_loop
.code16


inf_loop:
    #call print_welcome_string #Loop prints U MAD BRO
    jmp inf_loop #Infinite loop (like a boss)

print_newline:
    movw $newline_string, %si
    call print
    ret
print_welcome_string:
    movw $welcome_string, %si
    call print
    ret

print: #Expects pointer to the string to be in si
    #xorb %bh, %bh #xor the bh register (probably not needed)
    #movb $0b0100, %bl #sets color to red (needs to be on correct graphics mode to work)

    movb $0xE, %ah #sets teletype mode for interrupt 0x10 (Only needed once?)
print_loop:

    lodsb #Loads byte at si into al and increments si

    cmpb $0, %al #Checks if current character is NULL
    je print_ret #Jumps to return if the character is NULL

    int $0x10 #Prints character to the screen
    jmp print_loop #Loops to beginning

print_ret:
    ret

print_num: #Input number is in al
    addb %al, 48 #Adds 0x30 to the number, turning it into the ascii equivilent
    mov $0xe, %ah #Setup teletype mode
    int $0x10 #Execute char print
    ret


welcome_string:
    .asciz "U MAD BRO?\n\r"
disk_error_string:
    .asciz "Normal Read Failed!\n\r"

extended_read_sector_error_string:
    .asciz "Extended read failed!\n\r"

newline_string:
    .asciz "\n\r"

chs_sector:
    .byte 0x1
chs_cylinder:
    .byte 0x0
chs_head:
    .byte 0x0


drive_num:
    .byte 0x0
num_heads:
    .byte 0x0
sectors_per_track:
    .byte 0x0

int_0x13_struct:
    .byte 0x10 #Disk Address Packet Size
    .byte 0x0 #Unused
num_sectors:
    .word 0x1 #Number of sectors to be read (2 Bytes)
offset:
    .word 0x0 #Offset to put result (Segment:Offset pointer, but Offset is defined before segment because x86 is little endian)
segment:
    .word 0x1000 #Segment to put result
sector:
    .quad 0x0 #Segment to read from (Uses logical block addressing)

#GDT
.align 16
gdtp:
    .word gdt_end - gdt_start - 1
    /* .long (0x07C0 << 4) + gdt */
    .long gdt_start

.align 16
gdt_start:
gdt_null:
    .quad 0
gdt_code_segment:
    .word 0xffff
    .word 0x0000
    .byte 0x00
    .byte 0b10011010
    .byte 0b11001111
    .byte 0x00
gdt_data_segment:
    .word 0xffff
    .word 0x0000
    .byte 0x00
    .byte 0b10010010
    .byte 0b11001111
    .byte 0x00
gdt_end:

/* IDT */
idt:
    .word 0
    .long 0


.fill 510-(.-_start), 1, 0
.word 0xAA55
