#include "exception.h"

void exception_handler(cpu_state_t *state) {
    calamity_panic(state); //in the event that we hit a bad panic that we have no hope of getting out of.
}
