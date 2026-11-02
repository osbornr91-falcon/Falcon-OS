#ifndef COMMUNICATION_BUS_CONTROLLER_H
#define COMMUNICATION_BUS_CONTROLLER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Minimum protected requirement for the test build: one KiB. */
#define COMMUNICATION_BUS_TEST_MINIMUM_BYTES 1024U
#define COMMUNICATION_BUS_CHANNEL_COUNT 16U
#define COMMUNICATION_BUS_PAYLOAD_SIZE 32U
#define COMMUNICATION_BUS_STATUS_SIZE 16U

typedef enum {
    COMMUNICATION_TARGET_KERNEL = 0,
    COMMUNICATION_TARGET_BUS = 1,
    COMMUNICATION_TARGET_COMMAND_LINE = 2,
    COMMUNICATION_TARGET_BROADCAST = 15
} communication_target_t;

typedef enum {
    COMMUNICATION_STATUS_EMPTY = 0,
    COMMUNICATION_STATUS_READY = 1,
    COMMUNICATION_STATUS_OK = 2,
    COMMUNICATION_STATUS_PENDING = 3,
    COMMUNICATION_STATUS_RESPONSE = 4,
    COMMUNICATION_STATUS_ERROR = 5,
    COMMUNICATION_STATUS_SHUTDOWN = 6
} communication_status_t;

typedef struct {
    volatile uint16_t length;
    volatile uint8_t payload[COMMUNICATION_BUS_PAYLOAD_SIZE];
    volatile uint32_t sequence;
    volatile communication_status_t status[COMMUNICATION_BUS_STATUS_SIZE];
} communication_channel_t;

void communication_bus_init(void);
bool communication_bus_send(communication_target_t target,
                            communication_status_t status,
                            const void *data,
                            size_t length);
bool communication_bus_read(communication_target_t target,
                            communication_channel_t *out);
void communication_bus_cleanup(communication_target_t target);
void communication_bus_cleanup_all(void);
bool communication_bus_self_test(void);
communication_channel_t *communication_bus_channel_for_target(
    communication_target_t target);

/* Busy wait is used only until Falcon OS has a scheduler-backed timer. */
void communication_bus_wait_seconds(uint32_t seconds);

#endif
