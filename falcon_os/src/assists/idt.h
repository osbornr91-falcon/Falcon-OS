#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// 64-bit IDT Gate Descriptor
typedef struct {
    uint16_t offset_low;      // Lower 16 bits of handler function address
    uint16_t selector;        // Kernel code segment selector (usually 0x08)
    uint8_t  ist;             // Interrupt Stack Table offset (0 for now)
    uint8_t  type_attr;       // Gate type, DPL, and present flags (e.g., 0x8E for 32/64-bit interrupt gate)
    uint16_t offset_middle;   // Middle 16 bits of handler address
    uint32_t offset_high;     // Higher 32 bits of handler address
    uint32_t reserved;        // Set to zero
} __attribute__((packed)) idt_entry_t;

// IDTR register structure for LIDT instruction
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;

void idt_init(void);
void idt_set_gate(uint8_t vector, void* handler, uint16_t selector, uint8_t flags);

#endif