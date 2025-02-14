# Ashken Operating System Kernel

## Introduction
This is a hobbyist operating system kernel designed for the Intel/AMD x86-32 architecture. It is built from scratch to explore low-level system programming concepts, including bootstrapping, memory management, multitasking, and hardware interaction. The goal of this project is to learn and experiment with OS development while gradually adding new features.

## Features
- **Multiboot-compliant Bootloader**: Compatible with GRUB for easy loading.
- **Protected Mode**: Runs in x86-32 protected mode with paging enabled.
- **Basic Memory Management**: Implements a simple memory allocator.
- **Interrupt Handling**: Support for handling hardware and software interrupts.
- **Multitasking (Planned)**: Future implementation of task switching and process management.
- **File System (Planned)**: Basic support for a simple filesystem for storing and retrieving files.

## Architecture
The kernel follows a monolithic design with modular components for different subsystems:
- **Bootloader**: Responsible for setting up the environment and jumping to kernel entry.
- **Kernel Core**: Manages memory, tasks, and interrupt handling.
- **Drivers**: Basic drivers for keyboard and screen output.
- **User Mode (Planned)**: Separation of kernel and user space for executing user applications.

## Development Setup
### Requirements
- **Compiler**: GCC (cross-compiled for x86-elf)
- **Assembler**: NASM for bootloader and low-level assembly routines
- **Linker**: GNU LD with a custom linker script
- **Emulator**: QEMU for testing

### Building and Running
1. Clone the repository:
   ```sh
   git clone https://github.com/vinnieashken/ashkenos.git
   cd ashkenos
   ```
2. Clean the project:
   ```sh
   make clean
   ```   
3. Build the kernel:
   ```sh
   make
   ```
4. Run using QEMU:
   ```sh
   make run
   ```

## Roadmap
- [x] Basic bootloader
- [x] Entering protected mode
- [x] Basic VGA text output
- [x] Basic Keyboard driver
- [x] Memory management 
- [ ] Interrupt handling refinements
- [ ] Multitasking implementation
- [ ] User-mode support
- [ ] File system support

## Contributing
This is a personal learning project, but contributions, suggestions, and feedback are welcome. Feel free to fork and experiment with the code!

## License
This project is released under the MIT License. See `LICENSE` for details.

