#include "exception.h"

void exception_handler(cpu_state_t *state) {
    calamity_panic(state);
}
