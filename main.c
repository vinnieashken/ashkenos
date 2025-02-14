__asm__ (
        // Define constants for the multiboot header
        ".set ALIGN, 1<<0\n"
        ".set MEMINFO, 1<<1\n"
        ".set FLAGS, ALIGN | MEMINFO\n"
        ".set MAGIC, 0x1BADB002\n"
        ".set CHECKSUM, -(MAGIC + FLAGS)\n"

        // Multiboot header
        ".section .multiboot\n"
        ".align 4\n"
        ".long MAGIC\n"
        ".long FLAGS\n"
        ".long CHECKSUM\n"
        ".long 0x100000\n"
        ".long 0x1000\n"
        
        // Stack section
        ".section .bss\n"
        ".align 16\n"
        "stack_bottom:\n"
        ".skip 16384\n"  // Reserve 16 KiB for stack
        "stack_top:\n"

        // Kernel entry point
        ".section .text\n"
        ".global _start\n"
        ".type _start, @function\n"
        "_start:\n"

        // Set the stack pointer to the top of the stack
        "mov $stack_top, %esp\n"


        "push %ebx\n"
        // Call the kernel main function
        "call kmain\n"

        // Disable interrupts
        "cli\n"

        // Infinite loop to halt the processor (safe stopping point)
        "1: hlt\n"
        "jmp 1b\n"

        // Set the size of the _start symbol
        ".size _start, . - _start\n"
    );


#include "vga.h"
#include "memory.h"
#include "klib.h"

// void task1() {

//     printf("This is PROCESS 1\n");

//     while (1)
//     {
//         /* code */
//     }
// }

// void task2() {
//     printf("This is PROCESS 2\n");
//     while (1)
//     {
//         /* code */
//     }
    
// }

// void task3(){
//     printf("This is PROCESS 3\n");
//     while (1)
//     {
//         /* code */
//     }
// }

void task1() {
    printf("Task 1 started\n");
    int counter = 0;
    // while (counter <=5) {
    //     printf("Task 1 running (iteration %d)\n", counter++);
    //     // Add a delay
    //     //for(volatile int i = 0; i < 100; i++) {}
    // }
}

void task2() {
    printf("Task 2 started\n");
    int counter = 0;
    // while (counter <=5) {
    //     printf("Task 2 running (iteration %d)\n", counter++);
    //     // Add a delay
    //     //for(volatile int i = 0; i < 100; i++) {}
    // }
}

void process1_func() {
    printf("Process 1 is running\n");
    int i=0;
    // while(1)
    // {
    //     if(i <10)
    //     {
    //         i++;
    //         printf("Process 1: %d\n", i);
    //     }
        
    // }
}

void process2_func() {
    printf("Process 2 is running\n");
   int i=0;
    // while(1)
    // {
    //     if(i <10)
    //     {
    //         i++;
    //         printf("Process 2: %d\n", i);
    //     }
        
    // }
}

process_t process1 = {0};
process_t process2 = {0};

// Updated scheduler
process_t *schedule() {
    if (current_process == &process1 && process2.state == READY) {
        return &process2;
    } else if (current_process == &process2 && process1.state == READY) {
        return &process1;
    }
    return current_process; // No switch if both processes are not READY
}

// void process1_func() {
//     while (1) {
//         printf("Process 1 is running\n");
//         for (volatile int i = 0; i < 1000000; ++i); // Simulate work
//         current_process->state = READY; // Yield back to scheduler
//     }
// }

// void process2_func() {
//     while (1) {
//         printf("Process 2 is running\n");
//         for (volatile int i = 0; i < 1000000; ++i); // Simulate work
//         current_process->state = READY; // Yield back to scheduler
//     }
// }



multiboot_memory_map_t *boot_info;
// Main kernel function
void kmain(multiboot_memory_map_t *info) {
    boot_info = info;
    clear();
    char str[20];
    puts("                               WELCOME TO ASHKEN OS                           \n\n\n");
    setup_gdt();
    puts("Setting up Global Descriptor Tables ...............................done\n");
    setup_paging();
    puts("Setting up Paging Tables Identity Mapping .........................done\n");
    setup_idt();
    puts("Setting up Interrupt Descriptor Tables ............................done\n");
    setup_PIC();
    puts("Setting up PIC ....................................................done\n");
    setup_PIT();
    puts("Setting up PIT ....................................................done\n");
    enable_interrupts();
    puts("Enabling Hardware Interrupts.......................................done\n");
    puts("Testing interrupts.................................................\n");
    trigger_interrupt();
    init_keyboard();
    printf("Initializing Heap memory...........................................done\n");
    init_heap();
    //disable_interrupts();
    
    //hardware_info_t info = hardware_info();

    uint32_t eax, ebx, ecx, edx;

    // Query CPUID with eax = 0 to get the highest function supported and vendor ID
    cpuid(0, &ebx, &ecx, &edx, &eax);

    uint32_t ven[4];
    ven[0] = ebx;
    ven[1] = edx;
    ven[2] = ecx;
    ven[3] = '\0'; 

    //unsigned long ram = info->mem_upper;
    unsigned long ram = get_physical_ram();
    mem_size memory = format_memory(ram);
    printf("CPU VENDOR: %s\nSYSTEM RAM: %d%s\n",(char*)ven,memory.size,memory.qualifier);
    char buffer[128];

    // sprintf(buffer, "Integer: %d, Unsigned: %u, Hex: %x, Float: %f, Char: %c, String: %s", 
    //         -42, 42, 255, 3.14159, 'A', "Test");
    // printf("Result: %s\n", buffer);
    //setup_identity_mapping();

    char *p = malloc(20);

    strcpy(p,"hello world\n\n\0");
    printf(p);
    free(p);

    struct interrupt_frame frame;

    // enable_interrupts();
    // add_process(task1, 1024);  // Task 1 with a stack size of 1024 bytes
    // add_process(task2, 1024);  // Task 2 with a stack size of 1024 bytes
    //add_process(task3, 1024);  // Task 1 with a stack size of 1024 bytes
    //print_queue_state();
    // switch_context(&frame);
    // switch_context(&frame);
    // process_t *new_process = (process_t*) malloc(sizeof(process_t));
    // new_process->state = READY;
    // if (new_process == NULL) {
    //     printf("Failed to allocate memory for new process size %d!\n",sizeof(process_t));
    //     return;
    // }
    // else{
    //     printf("New process state = %d\n",new_process->state);
    // }

    //print_queue_state();
    int stack_size = 1024;
    process_t * proc1 = create_process((uint32_t)process1_func,stack_size);
    process_t * proc2 = create_process((uint32_t)process2_func,stack_size);

    // Set up the ready queue
    proc1->next = proc2;
    proc2->next = NULL; // Circular queue
    ready_queue = proc1;
    proc1->state = RUNNING;
    //printf("Ready queue setup: Process 1 -> %x, Process 2 -> %x\n  , Process 1 real %x\n", proc1->next, proc2->next,proc1);

    label_address = &&EXIT;

    // Scheduler loop
    // current_process = ready_queue;
    // process_t *next_process = current_process;
    // swtch(NULL, next_process);

    // Start the scheduler loop
    current_process = ready_queue;

    printf("FIRST SP %x,   FP %x\n",current_process->stack_pointer,current_process->func);
    printf("NEXT SP %x,  FP %x\n",current_process->next->stack_pointer,current_process->next->func);
  
    if (current_process) {
        current_process->state = RUNNING; // Mark the first process as RUNNING
        swtch(NULL, current_process);
    }

    // while (current_process) {
    //     next_process = current_process->next;
    //     swtch(NULL, current_process);
    //     current_process = next_process;
    // }

    EXIT:
    printf("All processes terminated. Returning to kmain.\n");
    //print_queue_state();
    for(;;);
}


