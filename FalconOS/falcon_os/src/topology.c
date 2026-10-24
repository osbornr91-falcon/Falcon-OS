#include "topology.h"
#include <stddef.h>

static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile ("cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}

void topology_detect_cpuid(system_topology_t *topo) {
    uint32_t eax, ebx, ecx, edx;

    // 1. Get Vendor String (Leaf 0x0)
    cpuid(0, &eax, &ebx, &ecx, &edx);
    ((uint32_t*)topo->vendor_string)[0] = ebx;
    ((uint32_t*)topo->vendor_string)[1] = edx;
    ((uint32_t*)topo->vendor_string)[2] = ecx;
    topo->vendor_string[12] = '\0';

    // 2. Get Family / Model / Core Count info (Leaf 0x1)
    cpuid(1, &eax, &ebx, &ecx, &edx);
    topo->family = ((eax >> 8) & 0xF) + ((eax >> 20) & 0xFF);
    topo->model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);

    // Initial default BSP core entry
    topo->total_cores = 1;
    topo->cores[0].lapic_id = 0;
    topo->cores[0].processor_id = 0;
    topo->cores[0].is_enabled = true;
    topo->cores[0].is_bsp = true;
}
