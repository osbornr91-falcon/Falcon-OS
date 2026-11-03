#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    COMMAND_LINE_TEST_NONE = 0,
    COMMAND_LINE_TEST_KERNEL = 1 << 0,
    COMMAND_LINE_TEST_BUS = 1 << 1,
    COMMAND_LINE_TEST_COMMAND_LINE = 1 << 2,
    COMMAND_LINE_TEST_ALL =
        COMMAND_LINE_TEST_KERNEL |
        COMMAND_LINE_TEST_BUS |
        COMMAND_LINE_TEST_COMMAND_LINE
} command_line_test_mask_t;

void command_line_init(void);
void command_line_open(void);
void command_line_set_framebuffer(void *address, uint64_t width,
                                  uint64_t height, uint64_t pitch,
                                  uint16_t bpp, uint8_t red_shift,
                                  uint8_t green_shift, uint8_t blue_shift);
void command_line_append_output(const char *text);
const char *command_line_running_output(void);
bool command_line_run(const char *command);
bool command_line_test_systems(command_line_test_mask_t systems);
bool command_line_self_test(void);
void command_line_request_allstop(bool confirmed);
void command_line_request_shutdown(void);
void command_line_poll_keyboard(void);

#endif
