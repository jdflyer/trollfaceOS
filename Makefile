CC=i386-elf-gcc
AS=i386-elf-as
LD=i386-elf-gcc
CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS= -ffreestanding -O2 -nostdlib -lgcc
MKRESCUE=grub-mkrescue

BUILD=build
OUTPUTISO=$(BUILD)/trollfaceOS.iso
KERNEL=$(BUILD)/trollfaceOS.bin
OBJDIR=$(BUILD)/obj
FSDIR=$(BUILD)/fs

SRCDIR=src
GRUBCFG=$(SRCDIR)/grub.cfg
LDSCRIPT=$(SRCDIR)/linker.ld
SOURCES=$(patsubst $(SRCDIR)/%,%,$(wildcard $(SRCDIR)/*.s) $(wildcard $(SRCDIR)/*.c))
OBJS=$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(SOURCES)))
FULLOBJS=$(patsubst %,$(OBJDIR)/%,$(OBJS))

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	$(AS) $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

dirs:
	@mkdir -p $(OBJDIR)

$(KERNEL): $(FULLOBJS)
	$(LD) -T $(LDSCRIPT) -o $(KERNEL) $(LDFLAGS) $(FULLOBJS)

kernel: $(KERNEL)

$(OUTPUTISO): $(KERNEL)
	@mkdir -p $(FSDIR)/boot/grub
	@cp $(KERNEL) $(FSDIR)/boot/
	@cp $(GRUBCFG) $(FSDIR)/boot/grub/
	$(MKRESCUE) -o $(OUTPUTISO) $(FSDIR)

iso: dirs $(OUTPUTISO)

all: dirs kernel iso

run: all
	qemu-system-i386 -cdrom $(OUTPUTISO)

.PHONY: clean run

clean:
	@rm -rfv build