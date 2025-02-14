#ifndef CPU_H
#define CPU_H
#include <stddef.h>
#include "memory.h"

struct interrupt_frame {
    unsigned int edi;       // General-purpose registers
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;       // The stack pointer at the time of interrupt
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

    unsigned int eip;       // Instruction pointer (where to continue execution)
    unsigned int cs;        // Code segment
    unsigned int eflags;    // Flags register

    unsigned int useresp;   // User stack pointer (if applicable)
    unsigned int ss;        // Stack segment (if applicable)

    unsigned int error_code; // Error code (optional, used by some exceptions)
};

typedef enum {
    RUNNING,
    READY,
    WAITING,
    TERMINATED
} process_state_t;

typedef struct process {
    void (*func)();             // Pointer to the function to be executed
    process_state_t state;      // State of the process (RUNNING, READY, etc.)
    unsigned int stack_pointer; // Pointer to the stack for this process
    unsigned int stack_size;    // Size of the stack
    struct process *next;       // Pointer to the next process in the list
} process_t;

process_t *ready_queue = NULL; // A linked list of processes in the READY state
process_t *current_process = NULL; // The currently running process

unsigned int scheduler_stack[1024]; // A stack for the scheduler
unsigned int scheduler_stack_pointer; // Pointer to the scheduler stack

extern void *malloc(size_t size);
extern void printf(const char *format, ...);
extern void free(void *ptr);
extern void outb(uint16_t port, uint8_t value);
extern void disable_interrupts();
extern void enable_interrupts();

void perform_context_switch(unsigned int* old_esp, unsigned int new_esp) {
    __asm__ __volatile__ (
        "pusha\n\t"
        "mov %%esp, %0\n\t"         // Save current ESP to *old_esp
        "mov %1, %%esp\n\t"         // Load new ESP
        "popa\n\t"                  // Restore general-purpose registers
        "iret\n\t"                  // Return from interrupt
        : "=m" (*old_esp)           // Output constraint for saving old ESP
        : "r" (new_esp)             // Input constraint for new ESP
        : "memory"                  // Memory clobber
    );

    // This line will never execute due to `iret`
}

void swtch(process_t *old_process, process_t *new_process) {
    asm volatile (
        "pusha\n\t"                      // Save all general-purpose registers
        "movl %%esp, %0\n\t"             // Save ESP to old_process->stack_pointer (if old_process is not NULL)
        "movl %1, %%esp\n\t"             // Load ESP with new_process->stack_pointer
        "popa\n\t"                       // Restore all general-purpose registers
        "ret\n\t"                        // Return to the new process
        : "=m"(old_process->stack_pointer)
        : "m"(new_process->stack_pointer)
        : "memory"
    );
}

uint32_t *label_address;

void terminate_process() {
    // Mark the current process as TERMINATED
    current_process->state = TERMINATED;

    // Remove the current process from the ready queue
    if (ready_queue == current_process) {
        ready_queue = current_process->next;
    } else {
        process_t *prev = ready_queue;
        while (prev && prev->next != current_process) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = current_process->next;
        }
    }

    // Switch back to the scheduler
    current_process = ready_queue;
    if (current_process) {
        swtch(NULL, current_process);
    } else {
        // No more processes to run, return to kmain
        asm volatile ("jmp *%0" : : "r"(label_address));
    }
}

void add_process(void (*func)(), unsigned int stack_size) {
    process_t *new_process = (process_t*)malloc(sizeof(process_t));
    if (new_process == NULL) return;

    new_process->func = func;
    new_process->state = READY;
    new_process->stack_size = stack_size;
    
    // Allocate stack
    void* stack_mem = malloc(stack_size);
    if (stack_mem == NULL) {
        free(new_process);
        return;
    }

    // Point to top of stack (stack grows down)
    unsigned int *stack = (unsigned int*)((unsigned int)stack_mem + stack_size);

    // Set up initial interrupt frame
    // *(--stack) = 0x202;               // EFLAGS with interrupts enabled
    // *(--stack) = 0x08;                // CS - assuming this is your code segment
    // *(--stack) = (unsigned int)func;   // EIP - starting point

    // // Save initial registers
    // *(--stack) = 0;    // EAX
    // *(--stack) = 0;    // ECX
    // *(--stack) = 0;    // EDX
    // *(--stack) = 0;    // EBX
    // *(--stack) = (unsigned int)stack_mem + stack_size;  // ESP
    // *(--stack) = 0;    // EBP
    // *(--stack) = 0;    // ESI
    // *(--stack) = 0;    // EDI

    new_process->stack_pointer = (unsigned int)stack;
    new_process->next = NULL;

    // Add to ready queue
    if (ready_queue == NULL) {
        ready_queue = new_process;
    } else {
        process_t *last = ready_queue;
        while (last->next != NULL) last = last->next;
        last->next = new_process;
    }
}

process_t * create_process(uint32_t pc, unsigned int stack_size)
{
    process_t *new_process = (process_t*)malloc(sizeof(process_t));
    if (new_process == NULL) return NULL;

    // Allocate stack memory
    void* stack_mem = malloc(stack_size);
    unsigned int *stack = (unsigned int*)((unsigned int)stack_mem + stack_size);

    // Push return address (termination handler)
    *(--stack) = (unsigned int) terminate_process;  // Replace with an appropriate termination function

    // Push EIP (function entry point)
    *(--stack) = pc;

    // Push initial registers
    *(--stack) = 0;    // EAX
    *(--stack) = 0;    // ECX
    *(--stack) = 0;    // EDX
    *(--stack) = 0;    // EBX
    *(--stack) = 0;    // ESP (will be overwritten)
    *(--stack) = 0;    // EBP
    *(--stack) = 0;    // ESI
    *(--stack) = 0;    // EDI

    // Set up the process struct
    new_process->stack_pointer = (unsigned int)stack;
    new_process->next = NULL;
    new_process->func = (void*)pc;
    new_process->state = READY;
    new_process->stack_size = stack_size;

    return new_process;
}

// void switch_context(struct interrupt_frame* frame) {
//     if (current_process == NULL) {
//         if (ready_queue != NULL) {
//             current_process = ready_queue;
//             ready_queue = ready_queue->next;
//             current_process->state = RUNNING;
//             current_process->next = NULL;

//             __asm__ __volatile__ (
//                 "mov %0, %%esp\n\t"
//                 "popa\n\t"
//                 "iret\n\t"
//                 : : "r" (current_process->stack_pointer)
//             );
//         }
//         return;
//     }

//     // Save the current process state
//     current_process->stack_pointer = frame->esp;
//     current_process->state = READY;

//     // Move current process to end of ready queue
//     if (ready_queue == NULL) {
//         ready_queue = current_process;
//     } else {
//         process_t *last = ready_queue;
//         while (last->next != NULL) last = last->next;
//         last->next = current_process;
//     }
//     current_process->next = NULL;

//     // Get the next process from the ready queue
//     if (ready_queue != NULL) {
//         process_t *next = ready_queue;
//         ready_queue = ready_queue->next;
//         next->state = RUNNING;
//         next->next = NULL;

//         process_t *old = current_process;
//         current_process = next;
//         // printf("  Process at %x (func: %x)\n", 
//         //        (unsigned int)next, (unsigned int)next->func);
//         disable_interrupts();
//         perform_context_switch(&old->stack_pointer, next->stack_pointer);
//         enable_interrupts();
//     }

//     __asm__ __volatile__ (
//         "popa\n\t"
//         "iret\n\t"
//     );
// }


void switch_context(struct interrupt_frame* frame) {
    if (current_process == NULL) {
        if (ready_queue != NULL) {
            current_process = ready_queue;
            ready_queue = ready_queue->next;
            current_process->state = RUNNING;
            current_process->next = NULL;

            // Set the stack pointer for the new process
            __asm__ __volatile__ (
                "mov %0, %%esp\n\t"
                "popa\n\t"
                "iret\n\t"
                : : "r" (current_process->stack_pointer)
            );
        }
        return;
    }

    // Save the current process state
    current_process->stack_pointer = (unsigned int)frame->esp;
    current_process->state = READY;

    // Move current process to end of ready queue
    if (ready_queue == NULL) {
        ready_queue = current_process;
    } else {
        process_t *last = ready_queue;
        while (last->next != NULL) last = last->next;
        last->next = current_process;
    }
    current_process->next = NULL;

    // Get the next process from the ready queue
    if (ready_queue != NULL) {
        process_t *next = ready_queue;
        ready_queue = ready_queue->next;
        next->state = RUNNING;
        next->next = NULL;

        // Switch to the next process
        process_t *old = current_process;
        current_process = next;

        // Perform the context switch by saving the old process' stack and loading the new one
        disable_interrupts();
        perform_context_switch(&old->stack_pointer, next->stack_pointer);
        enable_interrupts();
    }
}

// Helper function to print the state of the queue (add this for debugging)
void print_queue_state() {
    printf("Current process: %x\n", (unsigned int)current_process);
    printf("Ready queue:\n");
    process_t *temp = ready_queue;
    while (temp != NULL) {
        printf("  Process at %x (func: %x)\n", 
               (unsigned int)temp, (unsigned int)temp->func);
        temp = temp->next;
    }
}

#endif