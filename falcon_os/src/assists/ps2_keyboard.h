#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t scancode;
    bool is_pressed;
    char ascii;
} ps2_key_event_t;

void ps2_keyboard_init(void);
void ps2_keyboard_handle_irq1(void);
bool ps2_keyboard_poll_event(ps2_key_event_t *event);

#endif
