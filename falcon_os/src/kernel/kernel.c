#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "topology.h"
#include "idt.h"
#include "CommunicationBusController.h"
#include "CommandLine.h"

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

static bool kernel_shutdown_requested;
static bool kernel_allstop_requested;

static void halt(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

bool kernel_self_test(void) {
    return communication_bus_send(COMMUNICATION_TARGET_KERNEL,
                                  COMMUNICATION_STATUS_RESPONSE,
                                  "KERNEL-OK",
                                  sizeof("KERNEL-OK"));
}

void kernel_request_allstop(void) {
    kernel_allstop_requested = true;
    halt();
}

void kernel_request_shutdown(void) {
    kernel_shutdown_requested = true;
    halt();
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
    command_line_set_framebuffer(fb->address, fb->width, fb->height,
                                 fb->pitch, fb->bpp, fb->red_mask_shift,
                                 fb->green_mask_shift, fb->blue_mask_shift);

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

    // 5. Start the Communication Bus Controller
    communication_bus_init();

    // 6. Start the first runnable module
    command_line_init();
    command_line_open();

    // 7. Install exception handlers and enable maskable interrupts
    idt_init();
    __asm__ volatile ("sti");

    // Kernel startup complete. Module shutdown requests return here.
    halt();
}