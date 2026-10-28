#include "idt.h"
#include <stddef.h>

extern void isr0(void);
extern void isr6(void);
extern void isr14(void);

static idt_entry_t idt[256];
static idtr_t idtr;

void idt_set_gate(uint8_t vector, void *handler, uint16_t selector, uint8_t flags) {
    uint64_t address = (uint64_t)handler;

    idt[vector].offset_low = (uint16_t)address;
    idt[vector].selector = selector;
    idt[vector].ist = 0;
    idt[vector].type_attr = flags;
    idt[vector].offset_middle = (uint16_t)(address >> 16);
    idt[vector].offset_high = (uint32_t)(address >> 32);
    idt[vector].reserved = 0;
}

void idt_init(void) {
    idtr.limit = (uint16_t)(sizeof(idt) - 1);
    idtr.base = (uint64_t)&idt[0];

    idt_set_gate(0, isr0, 0x08, 0x8E);
    idt_set_gate(6, isr6, 0x08, 0x8E);
    idt_set_gate(14, isr14, 0x08, 0x8E);

    __asm__ volatile ("lidt %0" : : "m" (idtr));
}
