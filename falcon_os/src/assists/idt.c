#include "idt.h"
#include "pic.h"
#include <stddef.h>

extern void isr0(void);
extern void isr6(void);
extern void isr14(void);
extern void irq32(void);
extern void irq33(void);
extern void irq34(void);
extern void irq35(void);
extern void irq36(void);
extern void irq37(void);
extern void irq38(void);
extern void irq39(void);
extern void irq40(void);
extern void irq41(void);
extern void irq42(void);
extern void irq43(void);
extern void irq44(void);
extern void irq45(void);
extern void irq46(void);
extern void irq47(void);

extern uint64_t irq_handler_get_irq1_count(void);
extern uint64_t irq_handler_get_irq12_count(void);

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

    idt_set_gate(0x20, irq32, 0x08, 0x8E);
    idt_set_gate(0x21, irq33, 0x08, 0x8E);
    idt_set_gate(0x22, irq34, 0x08, 0x8E);
    idt_set_gate(0x23, irq35, 0x08, 0x8E);
    idt_set_gate(0x24, irq36, 0x08, 0x8E);
    idt_set_gate(0x25, irq37, 0x08, 0x8E);
    idt_set_gate(0x26, irq38, 0x08, 0x8E);
    idt_set_gate(0x27, irq39, 0x08, 0x8E);
    idt_set_gate(0x28, irq40, 0x08, 0x8E);
    idt_set_gate(0x29, irq41, 0x08, 0x8E);
    idt_set_gate(0x2A, irq42, 0x08, 0x8E);
    idt_set_gate(0x2B, irq43, 0x08, 0x8E);
    idt_set_gate(0x2C, irq44, 0x08, 0x8E);
    idt_set_gate(0x2D, irq45, 0x08, 0x8E);
    idt_set_gate(0x2E, irq46, 0x08, 0x8E);
    idt_set_gate(0x2F, irq47, 0x08, 0x8E);

    pic_init();
    __asm__ volatile ("lidt %0" : : "m" (idtr));
}

uint64_t idt_get_irq1_count(void) {
    return irq_handler_get_irq1_count();
}

uint64_t idt_get_irq12_count(void) {
    return irq_handler_get_irq12_count();
}
