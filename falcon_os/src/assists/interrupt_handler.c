#include "exception.h"
#include "pic.h"
#include "ps2_keyboard.h"

static volatile uint64_t irq1_interrupt_count;
static volatile uint64_t irq12_interrupt_count;

void irq_handler(uint8_t irq) {
    if (irq == 1) {
        irq1_interrupt_count++;
        ps2_keyboard_handle_irq1();
    } else if (irq == 12) {
        irq12_interrupt_count++;
    }

    pic_send_eoi(irq);
}

uint64_t irq_handler_get_irq1_count(void) {
    return irq1_interrupt_count;
}

uint64_t irq_handler_get_irq12_count(void) {
    return irq12_interrupt_count;
}

void exception_handler(cpu_state_t *state) {
    calamity_panic(state); //in the event that we hit a bad panic that we have no hope of getting out of.
}
