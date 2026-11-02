#include "CommunicationBusController.h"

/* Testing-sized unsafe RAM region controlled by this assist. */
static volatile uint8_t communication_bus_ram
    [COMMUNICATION_BUS_TEST_MINIMUM_BYTES]
    __attribute__((used));

static communication_channel_t communication_bus_channels
    [COMMUNICATION_BUS_CHANNEL_COUNT];

static volatile uint32_t communication_bus_sequence;

static void communication_bus_zero(void *destination, size_t length) {
    volatile uint8_t *bytes = destination;
    for (size_t i = 0; i < length; i++) {
        bytes[i] = 0;
    }
}

static void communication_bus_copy(void *destination, const void *source,
                                   size_t length) {
    volatile uint8_t *output = destination;
    const uint8_t *input = source;
    for (size_t i = 0; i < length; i++) {
        output[i] = input[i];
    }
}

void communication_bus_init(void) {
    uint8_t *ram = (uint8_t *)(uintptr_t)communication_bus_ram;

    for (size_t i = 0; i < COMMUNICATION_BUS_TEST_MINIMUM_BYTES; i++) {
        ram[i] = 0;
    }

    for (size_t i = 0; i < COMMUNICATION_BUS_CHANNEL_COUNT; i++) {
        communication_bus_zero((void *)&communication_bus_channels[i],
                               sizeof(communication_bus_channels[i]));
    }

    communication_bus_sequence = 0;
}

communication_channel_t *communication_bus_channel_for_target(
    communication_target_t target) {
    if ((unsigned)target >= COMMUNICATION_BUS_CHANNEL_COUNT) {
        return NULL;
    }
    return &communication_bus_channels[target];
}

bool communication_bus_send(communication_target_t target,
                            communication_status_t status,
                            const void *data,
                            size_t length) {
    communication_channel_t *channel =
        communication_bus_channel_for_target(target);

    if (channel == NULL || length > COMMUNICATION_BUS_PAYLOAD_SIZE) {
        return false;
    }

    channel->sequence = ++communication_bus_sequence;
    channel->length = (uint16_t)length;
    for (size_t i = 0; i < COMMUNICATION_BUS_PAYLOAD_SIZE; i++) {
        channel->payload[i] = 0;
    }
    if (data != NULL && length > 0) {
        communication_bus_copy((void *)(uintptr_t)channel->payload,
                               data, length);
    }

    for (size_t i = COMMUNICATION_BUS_STATUS_SIZE; i > 1; i--) {
        channel->status[i - 1] = channel->status[i - 2];
    }
    channel->status[0] = status;
    return true;
}

bool communication_bus_read(communication_target_t target,
                            communication_channel_t *out) {
    communication_channel_t *channel =
        communication_bus_channel_for_target(target);

    if (channel == NULL || out == NULL) {
        return false;
    }

    communication_bus_copy(out, (void *)channel, sizeof(*out));
    return true;
}

void communication_bus_cleanup(communication_target_t target) {
    communication_channel_t *channel =
        communication_bus_channel_for_target(target);

    if (channel != NULL) {
        communication_bus_zero((void *)channel, sizeof(*channel));
    }
}

void communication_bus_cleanup_all(void) {
    for (size_t i = 0; i < COMMUNICATION_BUS_CHANNEL_COUNT; i++) {
        communication_bus_zero((void *)&communication_bus_channels[i],
                               sizeof(communication_bus_channels[i]));
    }
}

bool communication_bus_self_test(void) {
    const char marker[] = "BUS-OK";
    if (!communication_bus_send(COMMUNICATION_TARGET_BUS,
                                COMMUNICATION_STATUS_RESPONSE,
                                marker, sizeof(marker))) {
        return false;
    }

    communication_channel_t snapshot;
    if (!communication_bus_read(COMMUNICATION_TARGET_BUS, &snapshot)) {
        return false;
    }

    return snapshot.status[0] == COMMUNICATION_STATUS_RESPONSE;
}

void communication_bus_wait_seconds(uint32_t seconds) {
    /* Replaced by scheduler timer support when the scheduler is available. */
    volatile uint64_t iterations =
        (uint64_t)seconds * 100000000ULL;
    while (iterations-- > 0) {
        __asm__ volatile ("pause");
    }
}
