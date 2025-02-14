#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include "vga.h"
#include "klib.h"
#include "cpu.h"

void enable_interrupts() {
    __asm__ __volatile__("sti");  // Set Interrupt Flag (enable interrupts)
}

void disable_interrupts() {
    __asm__ __volatile__("cli");  // Set Interrupt Flag (enable interrupts)
}

struct GDTEntry {
    uint16_t limit_low;  // Lower 16 bits of the segment limit
    uint16_t base_low;   // Lower 16 bits of the base address
    uint8_t base_mid;    // Next 8 bits of the base address
    uint8_t access;      // Access flags
    uint8_t granularity; // Granularity and limit high 4 bits
    uint8_t base_high;   // Last 8 bits of the base address
} __attribute__((packed));

// GDTR structure
struct GDTPointer {
    uint16_t limit;      // Limit of the GDT
    uint32_t base;       // Base address of the GDT
} __attribute__((packed));

// GDT and GDTR
struct GDTEntry gdt[3]; // Null, Code, Data segments
struct GDTPointer gdt_ptr;

// Assembly function to load GDT
void load_gdt(struct GDTPointer *gdt_ptr)
{
     __asm__ __volatile__(
        "lgdt (%0)            \n" // Load the GDT pointer
        "mov $0x10, %%ax      \n" // Data segment selector (index 2)
        "mov %%ax, %%ds       \n" // Load DS
        "mov %%ax, %%es       \n" // Load ES
        "mov %%ax, %%fs       \n" // Load FS
        "mov %%ax, %%gs       \n" // Load GS
        //"mov $0x10, %%ax      \n" // Load stack segment (SS)
        "mov %%ax, %%ss       \n" // Load SS
        //"mov $0xC000, %%esp  \n" // Initialize ESP (stack pointer) to a location in the stack
        "jmp $0x08, $.flush   \n" // Code segment selector (index 1) and jump to the flush label
        ".flush:              \n" // Flush pipeline
        :
        : "r" (gdt_ptr)            // Input: GDT pointer
        : "memory", "%eax"          // Clobbers: memory and EAX
     );
}

void setup_gdt() {
    // Null segment
    gdt[0] = (struct GDTEntry){0};

    // Code segment
    gdt[1] = (struct GDTEntry){
        .limit_low = 0xFFFF,
        .base_low = 0,
        .base_mid = 0,
        .access = 0x9A,    // Code segment, present, ring 0
        .granularity = 0xCF,
        .base_high = 0
    };

    // Data segment
    gdt[2] = (struct GDTEntry){
        .limit_low = 0xFFFF,
        .base_low = 0,
        .base_mid = 0,
        .access = 0x92,    // Data segment, present, ring 0
        .granularity = 0xCF,
        .base_high = 0
    };
 
    //Stack segment (descriptor) attributes
    gdt[3] = (struct GDTEntry){
        .limit_low = 0xFFFF,  // The limit (segment size) for the stack segment.
        .base_low = 0x0,       // Base address (stack base).
        .base_mid = 0x0,
        .access = 0xF,         // Present, DPL=0 (kernel mode), read/write
        .granularity = 0xCF,   //
        .base_high = 0x0,
    };
   

    // GDTR setup
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint32_t)&gdt;

    // Load GDT
    load_gdt(&gdt_ptr);
}


//Multiboot 1 Memory info structure
struct multiboot_mmap_entry
{
    uint32_t flags;          // Flags to indicate which information is available
    uint32_t mem_lower;      // Lower memory size in KB
    uint32_t mem_upper;      // Upper memory size in KB
    uint32_t boot_device;    // Boot device (as specified by the bootloader)
    uint32_t cmdline;        // Command line passed to the kernel (if any)
    uint32_t mods_count;     // Number of modules loaded by the bootloader
    uint32_t mods_addr;      // Address of the modules (if mods_count > 0)
    // Optional fields depending on flags:
    uint32_t num_boot_modules;  // Number of boot modules (only available if flags contain the appropriate bit)
    uint32_t mmap_length;     // Length of the memory map (if provided)
    uint32_t mmap_addr;       // Memory map address (if provided)
    uint32_t drives_length;   // Length of the drives (if provided)
    uint32_t drives_addr;     // Drives address (if provided)
    uint32_t config_table;    // Configuration table address (if provided)
    uint32_t boot_loader_name; // Boot loader name string address (if provided)
    uint32_t apm_table;       // APM table address (if available)
    uint32_t vbe_control_info; // VBE control info address (if provided)
    uint32_t vbe_mode_info;   // VBE mode info address (if provided)
    uint32_t vbe_mode;        // VBE mode (if available)
    uint32_t vbe_interface_seg; // VBE interface segment (if available)
    uint32_t vbe_interface_off; // VBE interface offset (if available)
    uint32_t vbe_interface_len; // VBE interface length (if available)
} __attribute__((packed));

typedef struct multiboot_mmap_entry multiboot_memory_map_t;

extern multiboot_memory_map_t *boot_info;

//Calculate Physical Memory in bytes.
uint32_t get_physical_ram() {
    uint32_t lower_memory = boot_info->mem_lower;  // Lower memory (KB)
    uint32_t upper_memory = boot_info->mem_upper;  // Upper memory (KB)

    // Convert to bytes
    uint32_t lower_memory_bytes = lower_memory * 1024;  // KB to bytes
    uint32_t upper_memory_bytes = upper_memory * 1024;  // KB to bytes

    // Total memory in bytes
    uint32_t total_memory_bytes = lower_memory_bytes + upper_memory_bytes;

    return total_memory_bytes;
}

typedef struct mem_size_t
{
    uint32_t size;
    char qualifier[3];
} mem_size;

mem_size format_memory(uint32_t size)
{
    mem_size ram={0};
    if (size >= (1024 * 1024 * 1024)) {  // GB
        ram.size = size / (1024 * 1024 * 1024);
        ram.qualifier[0] = 'G';
        ram.qualifier[1] = 'B';
        ram.qualifier[2] = '\0';
    } else if (size >= (1024 * 1024)) {  // MB
        ram.size = size / (1024 * 1024);
        ram.qualifier[0] = 'M';
        ram.qualifier[1] = 'B';
        ram.qualifier[2] = '\0';
    } else if (size >= 1024) {  // KB
        ram.size = size / 1024;
        ram.qualifier[0] = 'K';
        ram.qualifier[1] = 'B';
        ram.qualifier[2] = '\0';
    } else {  // Bytes
        ram.size = size;
        ram.qualifier[0] = 'B';
        ram.qualifier[1] = '\0';
    }

    ram.size += 1;

    return ram;
}

//Setup paging
#define PAGE_SIZE 4096
#define NUM_PAGE_TABLE_ENTRIES 1024
#define NUM_PAGE_DIR_ENTRIES   1024
#define NUM_PAGE_TABLES        ((4 * 1024) / 4)  // 4096 MB / 4 MB per table = 4 tables

__attribute__((aligned(PAGE_SIZE))) uint32_t page_directory[NUM_PAGE_DIR_ENTRIES];
__attribute__((aligned(PAGE_SIZE))) uint32_t page_tables[NUM_PAGE_TABLES][NUM_PAGE_TABLE_ENTRIES];

void load_page_directory_and_enable_paging(uint32_t *page_directory) {
    __asm__ __volatile__(
        "movl %0, %%cr3\n\t"
        "movl %%cr0, %%eax\n\t"
        "orl $0x80000001, %%eax\n\t"
        "movl %%eax, %%cr0\n\t"
        :
        : "r"(page_directory)
        : "eax"
    );
}

void setup_paging() {
    // Create 4 page tables to map the first 16 MB
    for (uint32_t table = 0; table < NUM_PAGE_TABLES; table++) {
        for (uint32_t i = 0; i < NUM_PAGE_TABLE_ENTRIES; i++) {
            page_tables[table][i] = (table * 4 * 1024 * 1024 + i * PAGE_SIZE) | 3; // Address + Present + RW
        }
        // Point the corresponding page directory entry to the page table
        page_directory[table] = ((uint32_t)page_tables[table]) | 3; // Address + Present + RW
    }

    // Clear the rest of the page directory
    for (uint32_t i = NUM_PAGE_TABLES; i < NUM_PAGE_DIR_ENTRIES; i++) {
        page_directory[i] = 0;
    }

    // Load the page directory and enable paging
    load_page_directory_and_enable_paging(page_directory);
}


// Function to map a virtual address to a physical address
void map_page(uint32_t virtual_address, uint32_t physical_address, uint32_t flags) {
    uint32_t pd_index = (virtual_address >> 22) & 0x3FF; // Top 10 bits
    uint32_t pt_index = (virtual_address >> 12) & 0x3FF; // Next 10 bits

    // Ensure the page table exists
    if (!(page_directory[pd_index] & 1)) {
        // Allocate a new page table (aligned to 4 KB)
        uint32_t new_page_table = (uint32_t)&page_tables[pd_index];
        page_directory[pd_index] = new_page_table | flags | 1; // Present + RW
    }

    // Get the address of the page table
    uint32_t *page_table = (uint32_t *)(page_directory[pd_index] & 0xFFFFF000);

    // Map the page
    page_table[pt_index] = (physical_address & 0xFFFFF000) | (flags & 0xFFF) | 1; // Present + RW
}

// Example usage: Map 0x1000 (virtual) to 0x2000 (physical)
void setup_identity_mapping() {
    for (uint32_t i = 0; i < 16 * 1024 * 1024; i += PAGE_SIZE) { // Map first 16 MB
        map_page(i, i, 0x3); // Present + RW
    }
}

#define HEAP_START 0xC0000000  // Example heap start address
#define HEAP_SIZE  0x10000    // 1 MB heap size
#define HEAP_PHY_START 0x001E8480  // 16 MB physical address for heap start

// Memory block header
typedef struct mem_block {
    size_t size;             // Size of the block (excluding header)
    bool free;               // Is this block free?
    struct mem_block *next;  // Next block in the list
} mem_block_t;

static mem_block_t *heap_start = (mem_block_t *)HEAP_START;
static mem_block_t *heap_end = (mem_block_t *)(HEAP_START + HEAP_SIZE);
static bool heap_initialized = false;

void init_heap() {
    if (heap_initialized) return;

    // Map the heap region using paging
    // for (uintptr_t addr = HEAP_START; addr < HEAP_START + HEAP_SIZE; addr += PAGE_SIZE) {
    //     map_page(addr,addr,0x3);  // Ensure all heap pages are mapped
    // }

    for (uintptr_t addr = HEAP_START, phys_addr = HEAP_PHY_START;
         addr < HEAP_START + HEAP_SIZE;
         addr += PAGE_SIZE, phys_addr += PAGE_SIZE) {
        map_page(addr, phys_addr, 0x3);  // Map virtual to physical (Present + RW)
    }

    // Initialize the first memory block
    heap_start->size = HEAP_SIZE - sizeof(mem_block_t);
    heap_start->free = true;
    heap_start->next = NULL;

    heap_initialized = true;
}

void *malloc(size_t size) {
    if (!heap_initialized) init_heap();

    mem_block_t *current = heap_start;

    // Align size to 8 bytes for performance
    size = (size + 7) & ~7;

    // First-fit strategy
    while (current) {
        if (current->free && current->size >= size) {
            // Split the block if there's extra space
            if (current->size >= size + sizeof(mem_block_t) + 8) {
                mem_block_t *new_block = (mem_block_t *)((uintptr_t)current + sizeof(mem_block_t) + size);
                new_block->size = current->size - size - sizeof(mem_block_t);
                new_block->free = true;
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;
            }

            current->free = false;
            return (void *)((uintptr_t)current + sizeof(mem_block_t));
        }
        current = current->next;
    }

    // No suitable block found
    return NULL;
}

void free(void *ptr) {
    if (!ptr) return;

    mem_block_t *block = (mem_block_t *)((uintptr_t)ptr - sizeof(mem_block_t));
    block->free = true;

    // Coalesce adjacent free blocks
    mem_block_t *current = heap_start;
    while (current) {
        if (current->free && current->next && current->next->free) {
            current->size += sizeof(mem_block_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}


// IDT entry structure
struct IDTEntry {
    uint16_t offset_low;  // Lower 16 bits of ISR address
    uint16_t selector;    // Code segment selector
    uint8_t zero;         // Reserved
    uint8_t attributes;   // Type and attributes
    uint16_t offset_high; // Upper 16 bits of ISR address
} __attribute__((packed));

// IDTR structure
struct IDTPointer {
    uint16_t limit;       // Limit of the IDT
    uint32_t base;        // Base address of the IDT
} __attribute__((packed));

// IDT and IDTR
struct IDTEntry idt[256];
struct IDTPointer idt_ptr;

// Assembly function to load IDT
void load_idt(struct IDTPointer *idt_ptr)
{
     __asm__ __volatile__(
         "lidt (%0)    \n"  // Load the IDT pointer
        :
        : "r" (idt_ptr)    // Input: IDT pointer
        : "memory"         // Clobbers: memory 
     );
}

struct interrupt_frame;
// Dummy ISR
#pragma GCC target("general-regs-only")
__attribute__((interrupt)) void isr_dummy(struct interrupt_frame* frame) {
    //puts(" Interrupt\n");
    outb(0x20, 0x20);  // Send EOI to PIC1 (Primary PIC)
    outb(0xA0, 0x20); // // Send EOI to PIC2 (Secondary PIC), if applicable
}
#pragma GCC reset_options

#pragma GCC target("general-regs-only")
__attribute__((interrupt)) void isr80_handler(struct interrupt_frame* frame) {
   puts("Interrupt 0x80 Handler.............................................done\n");
   outb(0x20, 0x20);  
   outb(0xA0, 0x20); 
}
#pragma GCC reset_options

#pragma GCC target("general-regs-only")
__attribute__((interrupt)) void PIT_handler(struct interrupt_frame* frame) {
   //
    //printf("PIT interrupt occurred!\n");

        if (current_process == NULL || current_process->state != RUNNING) {
        outb(0x20, 0x20); // Acknowledge EOI before returning
        return; // No process to switch
    }

    // Save the current process's state
    asm volatile (
        "pusha\n\t"                      // Save all general-purpose registers
        "movl %%esp, %0\n\t"             // Save ESP to current_process->stack_pointer
        : "=m"(current_process->stack_pointer)
        :
        : "memory"
    );

    // Find the next process in the ready queue
    process_t *next_process = current_process->next;
    if (next_process == NULL) {
        next_process = ready_queue; // Loop back to the start of the queue
    }

    // Switch to the next process
    current_process = next_process;
    if (current_process) {
        current_process->state = RUNNING;

        // Restore the next process's state
        asm volatile (
            "movl %0, %%esp\n\t"         // Load ESP with next_process->stack_pointer
            "popa\n\t"                   // Restore all general-purpose registers
            :
            : "m"(current_process->stack_pointer)
            : "memory"
        );
    }

    // Acknowledge the End of Interrupt
    outb(0x20, 0x20); // Send EOI to the master PIC

    printf("Returning from interrupt after ACK\n");

    // Return from interrupt
    asm volatile ("iret");
}
#pragma GCC reset_options

#define KEYBOARD_DATA_PORT 0x60

void init_keyboard() {
    outb(0x60, 0xF4); // Enable scanning
}

// Basic US QWERTY keymap for scancodes
// Define your scancode_to_ascii mapping as before.
char scancode_to_ascii[128] = {
    0,    27,    '1',    '2',    '3',    '4',    '5',    '6',    '7',    '8',    '9',    '0',    '-',    '=',    '\b', 
    '\t', 'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',    'o',    'p',    '[',    ']',    '\n',    0,  
    'a',    's',    'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',    '\'',   '`',    0,    '\\',   'z',  
    'x',    'c',    'v',    'b',    'n',    'm',    ',',    '.',    '/',    0,    '*',    0,    ' ',    0,    // Space is mapped here
};

// Define shifted scancodes for uppercase and special symbols.
char scancode_to_ascii_shift[] = {
    0,    27,    '!',    '@',    '#',    '$',    '%',    '^',    '&',    '*',    '(',    ')',    '_',    '+',    '\b', 
    '\t', 'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I',    'O',    'P',    '{',    '}',    '\n',    0,  
    'A',    'S',    'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':',    '"',   '~',    0,    '|',   'Z',  
    'X',    'C',    'V',    'B',    'N',    'M',    '<',    '>',    '?',    0,    '*',    0,    ' ',    0,    // Space is mapped here
};

#pragma GCC target("general-regs-only")
__attribute__((interrupt)) void keyboard_handler(struct interrupt_frame* frame) {
   uint8_t scancode = inb(KEYBOARD_DATA_PORT);  // Read scancode from data port
 
    //char str[100];
    //puts( itoa(scancode,str,16) );
    //puts("\n");
    static int shift_pressed = 0;

    if (scancode == 0x2A || scancode == 0x36) {  // Left or Right Shift press
        shift_pressed = 1;
        // No printing, just return to avoid processing Shift itself
    } else if (scancode == 0xAA || scancode == 0xB6) {  // Left or Right Shift released
        shift_pressed = 0;
        // No printing, just return to avoid processing Shift itself
    } else if (scancode < 128) {
        // If the key is pressed (not released) and valid
        char key = (shift_pressed) ? scancode_to_ascii_shift[scancode] : scancode_to_ascii[scancode];
        if (key) {
            putchar(key);  // Print the corresponding ASCII character
        }
    }

    // Send End of Interrupt (EOI) to the PIC
    outb(0x20, 0x20);  // EOI to Master PIC
    outb(0xA0, 0x20);  // EOI to Slave PIC (if applicable)
}
#pragma GCC reset_options


void set_idt_entry(int vector, void (*handler)()) {
    uint32_t handler_addr = (uint32_t)handler;
    idt[vector].offset_low = handler_addr & 0xFFFF;
    idt[vector].offset_high = (handler_addr >> 16) & 0xFFFF;
    idt[vector].selector = 0x08; // Code segment selector in the GDT (assuming 0x08)
    idt[vector].zero = 0;
    idt[vector].attributes = 0x8E; // Interrupt gate with DPL 0 (kernel privilege)
}


void setup_idt() {
    // Initialize IDT with dummy ISR
    for (int i = 0; i < 256; i++) {
        uint32_t isr_address = (uint32_t)isr_dummy;
        idt[i].offset_low = isr_address & 0xFFFF;
        idt[i].selector = 0x08;   // Code segment selector
        idt[i].zero = 0;
        idt[i].attributes = 0x8E; // Interrupt gate, present, ring 0
        idt[i].offset_high = (isr_address >> 16) & 0xFFFF;
    }

    set_idt_entry(0x80, isr80_handler);
    set_idt_entry(0x20,PIT_handler);
    set_idt_entry(0x21,keyboard_handler);
    // IDTR setup
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt;

    // Load IDT
    load_idt(&idt_ptr);
}


void trigger_interrupt() {
    asm volatile (
        "int $0x80"  // Trigger interrupt 0x80
    );
}

#define interrupt(NUM) \
    asm volatile ( \
        "int %0" \
        : \
        : "i"(NUM) \
        : "memory" \
    )

// void interrupt(int interrupt_number) {
//     asm volatile (
//         "int %0"  // Use the provided interrupt number
//         :         // No output operands
//         : "r"(interrupt_number)  // Input operand
//         : "memory"  // Clobbered list to indicate memory might be affected
//     );
// }


void setup_PIC()
{
    // ICW1: Start initialization sequence
    outb(0x20, 0x11); // Start initialization for Master PIC
    outb(0xA0, 0x11); // Start initialization for Slave PIC

    // ICW2: Set vector offset
    outb(0x21, 0x20); // Master PIC offset (IRQ0–IRQ7 -> IVT 0x20–0x27)
    outb(0xA1, 0x28); // Slave PIC offset (IRQ8–IRQ15 -> IVT 0x28–0x2F)

    // ICW3: Configure PIC cascade
    outb(0x21, 0x04); // Master PIC: IRQ2 is connected to Slave PIC
    outb(0xA1, 0x02); // Slave PIC: Cascade identity is 2

    // ICW4: Set 8086/88 mode
    outb(0x21, 0x01); // Master PIC in 8086 mode
    outb(0xA1, 0x01); // Slave PIC in 8086 mode

    // Mask interrupts
    outb(0x21, 0xFC); // Unmask IRQ0 and IRQ1, mask others
    outb(0xA1, 0xFF); // Mask all interrupts on Slave PIC
}

void setup_PIT()
{
    //PIT IRQ0 IVT number 0x20 (32d)
    //IRQ1 IVT number 0x21 (33d)
    uint16_t divisor = 1193180 / 100;
    // Set PIT to rate generator mode on channel 0.
    outb(0x43,0x36); // 00 11 0110: Channel 0, lobyte/hibyte, rate generator.
    //Set the divider for 100 Hz. Divider 11932 = 1193182/frequncy
    outb(0x40,divisor & 0xFF); // Low byte of frequency divider 11932 .
    outb(0x40,(divisor >> 8) & 0xFF); // High byte of frequency divider 11932.
}

#define MAX_VENDOR_ID_LEN 13

// Structure to store hardware info
typedef struct {
    uint32_t total_ram_kb;        // Total RAM in kilobytes (this can be set via other methods)
    char vendor_id[MAX_VENDOR_ID_LEN]; // CPU Vendor ID string
    uint32_t processor_flags;     // Processor features flags (from CPUID)
    uint32_t processor_signature; // Processor signature
    uint32_t cache_size_kb;       // Cache size (in KB)
} hardware_info_t;

// Function to invoke CPUID instruction
static void cpuid(uint32_t eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx, uint32_t *eax_out) {
    __asm__ (
        "cpuid"
        : "=a"(*eax_out), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(eax)
    );
}

// Function to retrieve hardware info
hardware_info_t hardware_info() {
    hardware_info_t info = {0};

    // Get vendor ID and processor information
    uint32_t eax, ebx, ecx, edx;

    // Query CPUID with eax = 0 to get the highest function supported and vendor ID
    cpuid(0, &ebx, &ecx, &edx, &eax);
    
    // Extract the vendor ID string
    // *((uint32_t *)info.vendor_id) = ebx;
    // *((uint32_t *)(info.vendor_id + 4)) = edx;
    // *((uint32_t *)(info.vendor_id + 8)) = ecx;
    // info.vendor_id[12] = '\0'; // Null-terminate the vendor ID string

    // Query CPUID with eax = 1 for processor flags and features
    // cpuid(1, &ebx, &ecx, &edx, &eax);
    // info.processor_flags = edx;      // Processor features are in edx
    // info.processor_signature = eax; // Processor signature is in eax

    // // Query CPUID for cache size (eax = 4 to get cache info)
    // cpuid(4, &ebx, &ecx, &edx, &eax);
    // info.cache_size_kb = ((ebx >> 22) + 1) * (ecx + 1);  // Cache size in KB (EBX bit 22 gives the cache line size)

    // For RAM size, we can use another method like e820 or other system calls to retrieve the RAM size.
    // For this example, we'll just set it to 8192MB (8GB) for demonstration purposes.
    info.total_ram_kb = 8192 * 1024; // Assume 8GB of RAM for this example

    return info;
}



#endif