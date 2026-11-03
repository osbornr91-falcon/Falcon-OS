#include "pic.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

static uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void pic_wait(void) {
    outb(0x80, 0);
}

void pic_init(void) {
    uint8_t master_mask = inb(PIC1_DATA);
    uint8_t slave_mask = inb(PIC2_DATA);

    outb(PIC1_COMMAND, 0x11);
    pic_wait();
    outb(PIC2_COMMAND, 0x11);
    pic_wait();

    outb(PIC1_DATA, PIC_MASTER_OFFSET);
    pic_wait();
    outb(PIC2_DATA, PIC_SLAVE_OFFSET);
    pic_wait();

    outb(PIC1_DATA, 0x04);
    pic_wait();
    outb(PIC2_DATA, 0x02);
    pic_wait();

    outb(PIC1_DATA, 0x01);
    pic_wait();
    outb(PIC2_DATA, 0x01);
    pic_wait();

    (void)master_mask;
    (void)slave_mask;

    /* Keep non-tested IRQs masked. IRQ 1 and IRQ 12 are enabled for plumbing. */
    outb(PIC1_DATA, 0xFB);
    outb(PIC2_DATA, 0xEF);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_mask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t bit;

    if (irq < 8) {
        port = PIC1_DATA;
        bit = (uint8_t)(1U << irq);
    } else {
        port = PIC2_DATA;
        bit = (uint8_t)(1U << (irq - 8));
    }

    outb(port, (uint8_t)(inb(port) | bit));
}

void pic_unmask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t bit;

    if (irq < 8) {
        port = PIC1_DATA;
        bit = (uint8_t)(1U << irq);
    } else {
        port = PIC2_DATA;
        bit = (uint8_t)(1U << (irq - 8));
    }

    outb(port, (uint8_t)(inb(port) & (uint8_t)~bit));
}
