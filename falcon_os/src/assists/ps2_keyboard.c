#include "ps2_keyboard.h"
#include "CommunicationBusController.h"

#define PS2_KEYBOARD_DATA_PORT 0x60
#define PS2_KEYBOARD_STATUS_PORT 0x64
#define PS2_KEYBOARD_QUEUE_SIZE 32

static ps2_key_event_t ps2_keyboard_queue[PS2_KEYBOARD_QUEUE_SIZE];
static volatile uint32_t ps2_keyboard_head;
static volatile uint32_t ps2_keyboard_tail;
static bool ps2_shift_pressed;
static bool ps2_caps_lock;
static bool ps2_extended_scan_code;

static uint8_t ps2_inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void ps2_outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void ps2_io_wait(void) {
    for (uint32_t i = 0; i < 10000; i++) {
        __asm__ volatile ("pause");
    }
}

static bool ps2_wait_input_clear(void) {
    for (uint32_t i = 0; i < 100000; i++) {
        if ((ps2_inb(PS2_KEYBOARD_STATUS_PORT) & 0x02) == 0) {
            return true;
        }
        __asm__ volatile ("pause");
    }
    return false;
}

static bool ps2_wait_output_full(void) {
    for (uint32_t i = 0; i < 100000; i++) {
        if ((ps2_inb(PS2_KEYBOARD_STATUS_PORT) & 0x01) != 0) {
            return true;
        }
        __asm__ volatile ("pause");
    }
    return false;
}

static bool ps2_read_controller_byte(uint8_t *value) {
    if (!ps2_wait_output_full()) {
        return false;
    }
    *value = ps2_inb(PS2_KEYBOARD_DATA_PORT);
    return true;
}

static char ps2_unshifted_ascii(uint8_t scan_code) {
    static const char unshifted[128] = {
        [0x01] = 27, [0x02] = '1', [0x03] = '2', [0x04] = '3',
        [0x05] = '4', [0x06] = '5', [0x07] = '6', [0x08] = '7',
        [0x09] = '8', [0x0A] = '9', [0x0B] = '0', [0x0C] = '-',
        [0x0D] = '=', [0x0E] = 8, [0x0F] = 9, [0x10] = 'q',
        [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
        [0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o',
        [0x19] = 'p', [0x1A] = '[', [0x1B] = ']', [0x1C] = '\n',
        [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f',
        [0x22] = 'g', [0x23] = 'h', [0x24] = 'j', [0x25] = 'k',
        [0x26] = 'l', [0x27] = ';', [0x28] = '\'', [0x29] = '`',
        [0x2B] = '\\', [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c',
        [0x2F] = 'v', [0x30] = 'b', [0x31] = 'n', [0x32] = 'm',
        [0x33] = ',', [0x34] = '.', [0x35] = '/', [0x39] = ' '
    };
    if (scan_code >= 128) {
        return 0;
    }
    return unshifted[scan_code];
}

static char ps2_shifted_ascii(uint8_t scan_code) {
    static const char shifted[128] = {
        [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',
        [0x06] = '%', [0x07] = '^', [0x08] = '&', [0x09] = '*',
        [0x0A] = '(', [0x0B] = ')', [0x0C] = '_', [0x0D] = '+',
        [0x0E] = 8, [0x1A] = '{', [0x1B] = '}', [0x1C] = '\n',
        [0x27] = ':', [0x28] = '"', [0x29] = '~', [0x2B] = '|',
        [0x33] = '<', [0x34] = '>', [0x35] = '?'
    };
    if (scan_code >= 128) {
        return 0;
    }
    char value = shifted[scan_code];
    if (value != 0) {
        return value;
    }
    value = ps2_unshifted_ascii(scan_code);
    if (value >= 'a' && value <= 'z') {
        return (char)(value - 'a' + 'A');
    }
    return value;
}

static void ps2_queue_event(ps2_key_event_t event) {
    uint32_t next = (ps2_keyboard_tail + 1) % PS2_KEYBOARD_QUEUE_SIZE;
    if (next == ps2_keyboard_head) {
        return;
    }
    ps2_keyboard_queue[ps2_keyboard_tail] = event;
    ps2_keyboard_tail = next;
}

void ps2_keyboard_init(void) {
    ps2_keyboard_head = 0;
    ps2_keyboard_tail = 0;
    ps2_shift_pressed = false;
    ps2_caps_lock = false;
    ps2_extended_scan_code = false;

    /* Disable the first PS/2 port and drain stale controller output. */
    if (!ps2_wait_input_clear()) {
        return;
    }
    ps2_outb(PS2_KEYBOARD_STATUS_PORT, 0xAD);
    ps2_io_wait();

    while ((ps2_inb(PS2_KEYBOARD_STATUS_PORT) & 0x01) != 0) {
        (void)ps2_inb(PS2_KEYBOARD_DATA_PORT);
        ps2_io_wait();
    }

    /* Read the controller configuration byte and enable IRQ 1. */
    if (!ps2_wait_input_clear()) {
        return;
    }
    ps2_outb(PS2_KEYBOARD_STATUS_PORT, 0x20);
    uint8_t configuration;
    if (!ps2_read_controller_byte(&configuration)) {
        return;
    }

    configuration |= 0x01;
    configuration &= (uint8_t)~0x40;

    if (!ps2_wait_input_clear()) {
        return;
    }
    ps2_outb(PS2_KEYBOARD_STATUS_PORT, 0x60);
    if (!ps2_wait_input_clear()) {
        return;
    }
    ps2_outb(PS2_KEYBOARD_DATA_PORT, configuration);
    ps2_io_wait();

    /* Re-enable the first PS/2 port and keyboard scanning. */
    if (!ps2_wait_input_clear()) {
        return;
    }
    ps2_outb(PS2_KEYBOARD_STATUS_PORT, 0xAE);
    ps2_io_wait();

    if (!ps2_wait_input_clear()) {
        return;
    }
    ps2_outb(PS2_KEYBOARD_DATA_PORT, 0xF4);
    if (!ps2_wait_output_full()) {
        return;
    }
    (void)ps2_inb(PS2_KEYBOARD_DATA_PORT);
}

void ps2_keyboard_handle_irq1(void) {
    if ((ps2_inb(PS2_KEYBOARD_STATUS_PORT) & 0x01) == 0) {
        return;
    }

    uint8_t scan_code = ps2_inb(PS2_KEYBOARD_DATA_PORT);
    if (scan_code == 0xE0) {
        ps2_extended_scan_code = true;
        return;
    }

    bool is_pressed = (scan_code & 0x80) == 0;
    uint8_t make_code = (uint8_t)(scan_code & 0x7F);

    if (!is_pressed && make_code == 0x2A) {
        ps2_shift_pressed = false;
    } else if (is_pressed && make_code == 0x2A) {
        ps2_shift_pressed = true;
    } else if (!is_pressed && make_code == 0x36) {
        ps2_shift_pressed = false;
    } else if (is_pressed && make_code == 0x36) {
        ps2_shift_pressed = true;
    } else if (is_pressed && make_code == 0x3A) {
        ps2_caps_lock = !ps2_caps_lock;
    }

    ps2_key_event_t event;
    event.scancode = make_code;
    event.is_pressed = is_pressed;
    event.ascii = 0;

    if (is_pressed) {
        char value = ps2_shift_pressed
            ? ps2_shifted_ascii(make_code)
            : ps2_unshifted_ascii(make_code);
        if (!ps2_shift_pressed && ps2_caps_lock &&
            value >= 'a' && value <= 'z') {
            value = (char)(value - 'a' + 'A');
        }
        event.ascii = value;
    }

    if (!ps2_extended_scan_code) {
        ps2_queue_event(event);
    }
    ps2_extended_scan_code = false;
}

bool ps2_keyboard_poll_event(ps2_key_event_t *event) {
    if (event == NULL || ps2_keyboard_head == ps2_keyboard_tail) {
        return false;
    }

    *event = ps2_keyboard_queue[ps2_keyboard_head];
    ps2_keyboard_head =
        (ps2_keyboard_head + 1) % PS2_KEYBOARD_QUEUE_SIZE;

    return communication_bus_send(COMMUNICATION_TARGET_COMMAND_LINE,
                                  COMMUNICATION_STATUS_READY,
                                  event,
                                  sizeof(*event));
}
