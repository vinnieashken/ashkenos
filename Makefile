# Compiler and linker tools
CC := x86_64-elf-gcc
LD := x86_64-elf-ld
AS := x86_64-elf-as

# Compiler flags
CFLAGS := -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -mno-red-zone -MMD -MP
LDFLAGS := -m elf_i386 -T linker.ld
ASFLAGS := --32

# Source files
C_SRC := $(wildcard *.c)
ASM_SRC := $(wildcard *.s)
OBJS := $(C_SRC:.c=.o) $(ASM_SRC:.s=.o)
DEPS := $(OBJS:.o=.d)

# Final output
OUTPUT := kernel.bin

# Phony targets
.PHONY: all clean run

# Default target
all: $(OUTPUT)

# Linking stage
$(OUTPUT): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Compilation stage
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# Compilation stage for assembly files
%.o: %.s
	$(AS)  $(ASFLAGS) -o $@ $<

# Clean up build artifacts
#rm -f $(OBJS) $(OUTPUT)
clean:
	del /f /q $(subst /,\,$(OBJS)) $(subst /,\,$(DEPS)) $(subst /,\,$(OUTPUT))

run:
	qemu-system-x86_64 -cpu qemu64 -m 256M -kernel kernel.bin

