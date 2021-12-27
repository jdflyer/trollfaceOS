CC=i386-elf-gcc
AS=i386-elf-as
LD=i386-elf-gcc
CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra -mgeneral-regs-only
LDFLAGS= -ffreestanding -O2 -s -nostdlib -lgcc -Xlinker
MKRESCUE=grub-mkrescue

BUILD=build
OUTPUTISO=$(BUILD)/trollfaceOS.iso
KERNEL=$(BUILD)/trollfaceOS.bin
MAPFILE=$(BUILD)/trollfaceOS.map
OBJDIR=$(BUILD)/obj
FSDIR=$(BUILD)/fs

OUTPUTIMG=$(BUILD)/trollfaceOS.img
ZERO=/dev/zero
IMGBLOCKS=100000
LOOPSTART=1048576
LOOPDEVICE=/dev/loop0
FSTYPE=vfat
MNTDIR=$(BUILD)/mnt

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
	$(LD) -T $(LDSCRIPT) -o $(KERNEL) $(LDFLAGS) -Map=$(MAPFILE) $(FULLOBJS)

kernel: $(KERNEL)

fs: dirs $(KERNEL)
	@mkdir -p $(FSDIR)/boot/grub
	@cp $(KERNEL) $(FSDIR)/boot/
	@cp $(GRUBCFG) $(FSDIR)/boot/grub/
	

$(OUTPUTISO): fs
	$(MKRESCUE) -o $(OUTPUTISO) $(FSDIR)

iso: dirs $(OUTPUTISO)

$(OUTPUTIMG): fs
	dd if=$(ZERO) of=$(OUTPUTIMG) count=$(IMGBLOCKS)
	parted --script $(OUTPUTIMG) mklabel msdos mkpart p fat32 1 50 set 1 boot on
	udisksctl loop-setup -f $(OUTPUTIMG)
	mkfs -t $(FSTYPE) -F 32 -n TROLLFACEOS --offset=2048 -v $(OUTPUTIMG)
	@mkdir -p $(MNTDIR)
	@echo "THE NEXT PART OF THE IMAGE CREATION WILL USE THE ROOT ACCOUNT TO MOUNT THE IMAGE AS AN FS AND INSTALL GRUB. PLEASE ONLY RUN IF YOU'VE READ THROUGH THE MAKEFILE"
	sudo mount $(LOOPDEVICE)p1 $(MNTDIR)
	sudo cp -vr $(FSDIR)/* $(MNTDIR)
	sudo grub-install --target=i386-pc --no-floppy --modules="biosdisk part_msdos fat configfile normal multiboot" --boot-directory=$(MNTDIR)/boot $(LOOPDEVICE)
	@du -sh $(MNTDIR)
	sudo umount $(MNTDIR)
	udisksctl loop-delete -b $(LOOPDEVICE)

img: dirs $(OUTPUTIMG)

all: dirs kernel iso img

runiso: iso
	qemu-system-i386 -cdrom $(OUTPUTISO)

runimg: img
	qemu-system-i386 -hda $(OUTPUTIMG)

.PHONY: clean run

clean:
	@rm -rfv build
