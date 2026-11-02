#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_CORES 256

// Read-only pass-up structure for individual CPU cores
typedef struct {
    uint32_t lapic_id;
    uint32_t processor_id;
    bool is_enabled;
    bool is_bsp;             // Bootstrap Processor
} cpu_core_info_t;

// Master read-only system topology pass-up struct
typedef struct {
    char vendor_string[13];  // e.g., "GenuineIntel" or "AuthenticAMD"
    uint32_t family;
    uint32_t model;
    uint32_t total_cores;
    uint64_t usable_ram_bytes;
    uint64_t total_ram_bytes;
    cpu_core_info_t cores[MAX_CORES];
} system_topology_t;

// Query CPUID for basic vendor and feature data
void topology_detect_cpuid(system_topology_t *topo);

#endif
