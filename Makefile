#TrollfaceOS Makefile

AS := i686-elf-as
CC := i686-elf-gcc
LD := i686-elf-ld

CFLAGS :=
ASMFLAGS :=

LINKERSCRIPT := linkerscript.ld

OUTDIR := bin
SRCDIR := src

BOOTSECTORSRC := src/boot/boot.s
BOOTSECTOROBJ := $(BOOTSECTORSRC:.s=.o)
BOOTSECTOROUT := bootsector.bin

OBJTARGETS :=

OUT_IMAGE := bin/trollfaceos.img

%.o: %.s
	$(AS) -o $@ -c $< $(ASMFLAGS)

#$(OUTDIR)/%.o: $(SRCDIR)/%.s
#	$(AS) -o $@ -c $< $(ASMFLAGS)

#$(OUTDIR)/%.o: $(SRCDIR)/%.c
#	$(CC) -c -o $@ $< $(CFLAGS)

all: dirs boot img

dirs:
	@mkdir -p $(OUTDIR)

boot: $(BOOTSECTOROBJ)
	$(LD) -o ./$(OUTDIR)/$(BOOTSECTOROUT) $^ -Ttext 0x7C00 --oformat=binary

img: dirs boot
	dd if=/dev/zero of=$(OUT_IMAGE) bs=512 count=2880
	dd if=./$(OUTDIR)/$(BOOTSECTOROUT) of=$(OUT_IMAGE) conv=notrunc bs=512 seek=0 count=1