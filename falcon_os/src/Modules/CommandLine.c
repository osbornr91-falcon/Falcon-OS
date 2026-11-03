#include "CommandLine.h"
#include "CommunicationBusController.h"
#include "ps2_keyboard.h"

static char command_line_output[4096];
static size_t command_line_output_length;
static volatile uint32_t *command_line_framebuffer;
static uint64_t command_line_framebuffer_width;
static uint64_t command_line_framebuffer_height;
static uint64_t command_line_framebuffer_pitch_pixels;
static uint16_t command_line_framebuffer_bpp;
static uint8_t command_line_red_shift;
static uint8_t command_line_green_shift;
static uint8_t command_line_blue_shift;
static char command_line_input[128];
static size_t command_line_input_length;
static uint32_t command_line_last_bus_sequence;

static void command_line_set_pixel(uint64_t x, uint64_t y,
                                   uint32_t color) {
    if (command_line_framebuffer == NULL ||
        x >= command_line_framebuffer_width ||
        y >= command_line_framebuffer_height) {
        return;
    }

    if (command_line_framebuffer_bpp == 32) {
        volatile uint32_t *pixels =
            (volatile uint32_t *)command_line_framebuffer;
        pixels[y * command_line_framebuffer_pitch_pixels + x] = color;
    } else if (command_line_framebuffer_bpp == 16) {
        volatile uint16_t *pixels =
            (volatile uint16_t *)command_line_framebuffer;
        uint16_t rgb565 =
            (uint16_t)(((color >> (16 + command_line_red_shift)) & 0xff) << 11) |
            (uint16_t)(((color >> (8 + command_line_green_shift)) & 0xff) << 5) |
            (uint16_t)((color >> command_line_blue_shift) & 0xff);
        pixels[y * (command_line_framebuffer_pitch_pixels * 2 / 4) + x] =
            rgb565;
    }
}

static uint32_t command_line_text_color(void) {
    if (command_line_framebuffer_bpp == 32) {
        return ((uint32_t)0xff << command_line_red_shift) |
               ((uint32_t)0xff << command_line_green_shift) |
               ((uint32_t)0xff << command_line_blue_shift);
    }
    return 0xffffffff;
}

static void command_line_draw_columns(uint64_t x, uint64_t y,
                                      const char *text) {
    uint32_t foreground = command_line_text_color();
    uint64_t cursor = x;

    while (*text != '\0') {
        uint8_t pattern[7];
        char c = *text++;
        if (c == 'F') {
            uint8_t p[7] = {31,16,16,30,16,16,16};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 'a') {
            uint8_t p[7] = {0,0,14,1,15,17,15};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 'l') {
            uint8_t p[7] = {12,4,4,4,4,4,14};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 'c') {
            uint8_t p[7] = {0,0,14,16,16,16,14};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 'o') {
            uint8_t p[7] = {0,0,14,17,17,17,14};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 'n') {
            uint8_t p[7] = {0,0,22,25,17,17,17};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 'O') {
            uint8_t p[7] = {14,17,17,17,17,17,14};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 'S') {
            uint8_t p[7] = {15,16,16,14,1,1,30};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 'A') {
            uint8_t p[7] = {14,17,17,31,17,17,17};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 'd') {
            uint8_t p[7] = {1,1,15,17,17,17,15};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 'm') {
            uint8_t p[7] = {0,0,26,21,21,17,17};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 'i') {
            uint8_t p[7] = {4,0,12,4,4,4,14};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 's') {
            uint8_t p[7] = {0,0,15,16,14,1,30};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 't') {
            uint8_t p[7] = {4,4,30,4,4,4,3};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == 'e') {
            uint8_t p[7] = {0,0,14,17,31,16,15};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = p[row];
        } else if (c == ':') {
            uint8_t pattern_values[7] = {0,4,4,0,4,4,0};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = pattern_values[row];
        } else if (c == '>') {
            uint8_t pattern_values[7] = {16,8,4,2,4,8,16};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = pattern_values[row];
        } else if (c == '.') {
            uint8_t pattern_values[7] = {0,0,0,0,0,4,4};
            for (uint64_t row = 0; row < 7; row++) pattern[row] = pattern_values[row];
        } else {
            for (uint64_t row = 0; row < 7; row++) pattern[row] = 0;
        }

        for (uint64_t row = 0; row < 7; row++) {
            for (uint64_t column = 0; column < 5; column++) {
                if ((pattern[row] & (uint8_t)(1U << (4 - column))) != 0) {
                    for (uint64_t scale_y = 0; scale_y < 2; scale_y++) {
                        for (uint64_t scale_x = 0; scale_x < 2; scale_x++) {
                            command_line_set_pixel(cursor + column * 2 + scale_x,
                                                   y + row * 2 + scale_y,
                                                   foreground);
                        }
                    }
                }
            }
        }
        cursor += 12;
    }
}

void command_line_set_framebuffer(void *address, uint64_t width,
                                  uint64_t height, uint64_t pitch,
                                  uint16_t bpp, uint8_t red_shift,
                                  uint8_t green_shift, uint8_t blue_shift) {
    command_line_framebuffer = (volatile uint32_t *)address;
    command_line_framebuffer_width = width;
    command_line_framebuffer_height = height;
    command_line_framebuffer_pitch_pixels =
        bpp >= 32 ? pitch / 4 : pitch / 2;
    command_line_framebuffer_bpp = bpp;
    command_line_red_shift = red_shift;
    command_line_green_shift = green_shift;
    command_line_blue_shift = blue_shift;

    if (command_line_framebuffer == NULL) {
        return;
    }

    uint64_t prompt_y = height > 16 ? height - 16 : 0;
    for (uint64_t yy = prompt_y; yy < height; yy++) {
        for (uint64_t xx = 0; xx < width; xx++) {
            command_line_set_pixel(xx, yy, 0x000000);
        }
    }
    command_line_draw_columns(0, prompt_y, "FalconOS:Admin:>");
}

static size_t command_line_text_length(const char *text) {
    size_t length = 0;
    while (text[length] != '\0') {
        length++;
    }
    return length;
}

static bool command_line_text_equals(const char *left, const char *right) {
    size_t index = 0;
    while (left[index] != '\0' && right[index] != '\0') {
        if (left[index] != right[index]) {
            return false;
        }
        index++;
    }
    return left[index] == '\0' && right[index] == '\0';
}

static void append_literal(const char *text) {
    size_t text_length = command_line_text_length(text);
    if (text_length >= sizeof(command_line_output) -
                       command_line_output_length) {
        text_length = sizeof(command_line_output) -
                      command_line_output_length - 1;
    }
    if (text_length > 0) {
        for (size_t i = 0; i < text_length; i++) {
            command_line_output[command_line_output_length + i] = text[i];
        }
        command_line_output_length += text_length;
    }
    command_line_output[command_line_output_length] = '\0';
}

void command_line_append_output(const char *text) {
    if (command_line_output_length > 0) {
        append_literal("\n");
    }
    append_literal(text);
}

void command_line_init(void) {
    command_line_output[0] = '\0';
    command_line_output_length = 0;
    command_line_input[0] = '\0';
    command_line_input_length = 0;
    command_line_last_bus_sequence = 0;
    command_line_append_output("Falcon OS command line module started.");
    command_line_append_output("Running output log:");
}

void command_line_open(void) {
    command_line_append_output("Command line opened.");
}

const char *command_line_running_output(void) {
    return command_line_output;
}

bool command_line_self_test(void) {
    return communication_bus_send(COMMUNICATION_TARGET_COMMAND_LINE,
                                  COMMUNICATION_STATUS_OK,
                                  "CLI-OK",
                                  sizeof("CLI-OK"));
}

extern bool kernel_self_test(void);

bool command_line_test_systems(command_line_test_mask_t systems) {
    bool kernel_ok = true;
    bool bus_ok = true;
    bool cli_ok = true;
    bool selected = false;

    if ((systems & COMMAND_LINE_TEST_KERNEL) != 0) {
        selected = true;
        kernel_ok = kernel_self_test();
    }
    if ((systems & COMMAND_LINE_TEST_BUS) != 0) {
        selected = true;
        bus_ok = communication_bus_self_test();
    }
    if ((systems & COMMAND_LINE_TEST_COMMAND_LINE) != 0) {
        selected = true;
        cli_ok = command_line_self_test();
    }

    if (!selected) {
        command_line_append_output("Test failed: no systems selected.");
        return false;
    }

    bool all_ok = true;
    if ((systems & COMMAND_LINE_TEST_KERNEL) != 0 && !kernel_ok) {
        all_ok = false;
    }
    if ((systems & COMMAND_LINE_TEST_BUS) != 0 && !bus_ok) {
        all_ok = false;
    }
    if ((systems & COMMAND_LINE_TEST_COMMAND_LINE) != 0 && !cli_ok) {
        all_ok = false;
    }
    if (!all_ok) {
        communication_bus_wait_seconds(5);
    }

    if (all_ok) {
        command_line_append_output("Test successful.");
        if ((systems & COMMAND_LINE_TEST_KERNEL) != 0) {
            command_line_append_output("Kernel response: OK");
        }
        if ((systems & COMMAND_LINE_TEST_BUS) != 0) {
            command_line_append_output("Communication Bus Controller response: OK");
        }
        if ((systems & COMMAND_LINE_TEST_COMMAND_LINE) != 0) {
            command_line_append_output("Command Line response: OK");
        }
    } else {
        command_line_append_output("Test failed.");
        if ((systems & COMMAND_LINE_TEST_KERNEL) != 0) {
            command_line_append_output(kernel_ok
                ? "Kernel response: OK"
                : "Kernel response: NO RESPONSE");
        }
        if ((systems & COMMAND_LINE_TEST_BUS) != 0) {
            command_line_append_output(bus_ok
                ? "Communication Bus Controller response: OK"
                : "Communication Bus Controller response: NO RESPONSE");
        }
        if ((systems & COMMAND_LINE_TEST_COMMAND_LINE) != 0) {
            command_line_append_output(cli_ok
                ? "Command Line response: OK"
                : "Command Line response: NO RESPONSE");
        }
    }

    return all_ok;
}

extern void kernel_request_allstop(void);
extern void kernel_request_shutdown(void);

void command_line_request_allstop(bool confirmed) {
    if (confirmed) {
        command_line_append_output("ALLSTOP confirmed. Requesting hard halt.");
        kernel_request_allstop();
    } else {
        command_line_append_output("ALLSTOP cancelled.");
    }
}

void command_line_request_shutdown(void) {
    communication_bus_send(COMMUNICATION_TARGET_BROADCAST,
                           COMMUNICATION_STATUS_SHUTDOWN,
                           "shutdown", 8);
    command_line_append_output(
        "Shutdown message sent to all running modules.");
    kernel_request_shutdown();
}

bool command_line_run(const char *command) {
    if (command == NULL) {
        return false;
    }
    if (command_line_text_equals(command, "test")) {
        return command_line_test_systems(COMMAND_LINE_TEST_ALL);
    }
    if (command_line_text_equals(command, "ALLSTOP")) {
        command_line_append_output(
            "ALLSTOP requires confirmation. Call with y/n confirmation.");
        return true;
    }
    if (command_line_text_equals(command, "shutdown")) {
        command_line_request_shutdown();
        return true;
    }
    command_line_append_output("Unknown command.");
    return false;
}

void command_line_poll_keyboard(void) {
    communication_channel_t channel;
    if (!communication_bus_read(COMMUNICATION_TARGET_COMMAND_LINE, &channel) ||
        channel.sequence == command_line_last_bus_sequence ||
        channel.status[0] != COMMUNICATION_STATUS_READY ||
        channel.length != sizeof(ps2_key_event_t)) {
        return;
    }

    command_line_last_bus_sequence = channel.sequence;
    ps2_key_event_t event;
    if (channel.length != sizeof(event)) {
        return;
    }
    uint8_t *event_bytes = (uint8_t *)&event;
    for (size_t i = 0; i < sizeof(event); i++) {
        event_bytes[i] = channel.payload[i];
    }

    if (!event.is_pressed) {
        return;
    }

    if (event.ascii == '\n') {
        command_line_input[command_line_input_length] = '\0';
        command_line_append_output("System:>");
        command_line_run(command_line_input);
        command_line_input_length = 0;
        command_line_input[0] = '\0';
    } else if (event.ascii == 8) {
        if (command_line_input_length > 0) {
            command_line_input_length--;
            command_line_input[command_line_input_length] = '\0';
        }
    } else if (event.ascii >= 32 && event.ascii <= 126 &&
               command_line_input_length < sizeof(command_line_input) - 1) {
        command_line_input[command_line_input_length++] = event.ascii;
        command_line_input[command_line_input_length] = '\0';
    }

    if (command_line_framebuffer != NULL &&
        command_line_framebuffer_height >= 16) {
        uint64_t prompt_y = command_line_framebuffer_height - 16;
        for (uint64_t yy = prompt_y; yy < command_line_framebuffer_height; yy++) {
            for (uint64_t xx = 0; xx < command_line_framebuffer_width; xx++) {
                command_line_set_pixel(xx, yy, 0x000000);
            }
        }
        command_line_draw_columns(0, prompt_y, "FalconOS:Admin:>");
        command_line_draw_columns(17 * 12, prompt_y, command_line_input);
    }
}
