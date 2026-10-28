# Compiler and Assembler Flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -pipe -Isrc -Isrc/kernel -Isrc/Modules -Isrc/assists \
         -ffreestanding -fno-stack-protector -fno-stack-check \
         -fno-lto -fno-PIE -mcmodel=kernel -m64 -march=x86-64 -mno-red-zone

NASM = nasm
NASMFLAGS = -f elf64

LD = ld
LDFLAGS = -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 -T linker.ld

# Directories and Targets
BUILD_DIR = build
ISO_DIR = iso_root
KERNEL = $(BUILD_DIR)/falcon_kernel
ISO = falcon_os.iso

# Source Files
C_SRCS = $(shell find src -type f -name '*.c' | sort)
ASM_SRCS = $(shell find src -type f -name '*.asm' | sort)
OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(C_SRCS)) \
       $(patsubst src/%.asm,$(BUILD_DIR)/%.o,$(ASM_SRCS))

.PHONY: all run clean limine

all: $(ISO)

# Compile C files
$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble Assembly files
$(BUILD_DIR)/%.o: src/%.asm | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(NASM) $(NASMFLAGS) $< -o $@

# Link Kernel Binary
$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

# Fetch Limine Bootloader Binaries
limine:
	@if [ ! -d "limine" ]; then \
		git clone https://github.com/limine-bootloader/limine.git --branch=v7.x-binary --depth=1; \
	fi
	$(MAKE) -C limine
	chmod +x limine/limine

# Package ISO
$(ISO): $(KERNEL) limine
	mkdir -p $(ISO_DIR)/boot
	cp $(KERNEL) $(ISO_DIR)/boot/falcon_kernel
	cp limine.conf $(ISO_DIR)/limine.conf
	cp limine.conf $(ISO_DIR)/limine.cfg
	cp limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin $(ISO_DIR)/
	xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		$(ISO_DIR) -o $(ISO)
	./limine/limine bios-install $(ISO)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Run in QEMU
run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO)

# Clean Build Artifacts
clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR) $(ISO)
