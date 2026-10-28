#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "topology.h"
#include "idt.h"

// Mandate Limine Protocol Version
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// Request start/end marker tags required by Limine
__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// Request graphical framebuffer
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Request Limine Memory Map
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

// Global read-only pass-up structure for higher modules/scheduler
static system_topology_t sys_topology;

static void halt(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

void kmain(void) {
    // 1. Ensure base revision is supported
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        halt();
    }

    // 2. Ensure we received a valid framebuffer response
    if (framebuffer_request.response == NULL 
        || framebuffer_request.response->framebuffer_count < 1) {
        halt();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    if (fb == NULL || fb->address == NULL) {
        halt();
    }

    // Use volatile pointer to prevent GCC vectorization optimizations
    volatile uint32_t *pixel_buffer = (volatile uint32_t *)fb->address;
    size_t pitch_pixels = fb->pitch / 4;

    // Render white 100x100 pixel square
    for (size_t y = 0; y < 100; y++) {
        for (size_t x = 0; x < 100; x++) {
            pixel_buffer[y * pitch_pixels + x] = 0x00FFFFFF;
        }
    }

    // 3. Detect CPU Vendor & Core Information (Pass-Up)
    topology_detect_cpuid(&sys_topology);

    // 4. Parse Memory Map (Pass-Up)
    if (memmap_request.response != NULL) {
        for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
            struct limine_memmap_entry *entry = memmap_request.response->entries[i];
            sys_topology.total_ram_bytes += entry->length;
            if (entry->type == LIMINE_MEMMAP_USABLE) {
                sys_topology.usable_ram_bytes += entry->length;
            }
        }
    }

    // Pass-up 'sys_topology' is now populated and ready for the scheduler!

    // 5. Install exception handlers and enable maskable interrupts
    idt_init();
    __asm__ volatile ("sti");

    // 6. Final Halt Loop
    halt();
}