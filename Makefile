#TrollfaceOS Makefile

AS := i686-elf-as
CC := i686-elf-gcc
LD := i686-elf-ld

CFLAGS := -Wno-builtin-declaration-mismatch
ASMFLAGS :=

LINKERSCRIPT := linkerscript.ld

OUTDIR := bin
SRCDIR := src

BOOTSECTORSRC := src/boot/boot.s
BOOTSECTOROBJ := $(BOOTSECTORSRC:.s=.o)
BOOTSECTOROUT := bootsector.bin

KERNEL_C_SRCS := $(wildcard src/kernel/*.c)
KERNEL_S_SRCS := $(wildcard src/kernel/*.s)
KERNEL_OBJS := $(KERNEL_C_SRCS:.c=.o) $(KERNEL_S_SRCS:.s=.o)

KERNELOUT := kernel.bin

OUT_IMAGE := bin/trollfaceos.img

%.o: %.s
	$(AS) -o $@ -c $< $(ASMFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

all: dirs boot img

dirs:
	@mkdir -p $(OUTDIR)

boot: $(BOOTSECTOROBJ)
	@$(LD) -o ./$(OUTDIR)/$(BOOTSECTOROUT) $^ -Ttext 0x7C00 --oformat=binary

kernel: $(KERNEL_OBJS)
	$(LD) -o ./bin/$(KERNELOUT) $^ $(LDFLAGS) -T$(LINKERSCRIPT)


img: dirs boot kernel
	@dd if=/dev/zero of=$(OUT_IMAGE) bs=512 count=2880 status=none
	@dd if=./$(OUTDIR)/$(BOOTSECTOROUT) of=$(OUT_IMAGE) conv=notrunc bs=512 seek=0 count=1 status=none
	@dd if=./bin/$(KERNELOUT) of=$(OUT_IMAGE) conv=notrunc bs=512 seek=1 count=2048 status=none