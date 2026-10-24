/* Tell the linker where execution starts */
ENTRY(_start)

SECTIONS
{
    /* Standard starting virtual address for higher-half kernels (2MB mark) */
    . = 0xffffffff80200000;

    /* Executable Code Section */
    .text : ALIGN(4K) {
        *(.text*)
    }

    /* Read-Only Data (Constants, String Literals) */
    .rodata : ALIGN(4K) {
        *(.rodata*)
    }
    
    /* Limine protocol request structures */
    .requests : ALIGN(4K) {
        KEEP(*(.requests_start_marker))
        KEEP(*(.requests))
        KEEP(*(.requests_end_marker))
    }

    /* Read-Write Data (Initialized Global/Static Variables) */
    .data : ALIGN(4K) {
        *(.data*)
    }

    /* Uninitialized Data (BSS - Global Variables initialized to zero) */
    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss*)
    }

    /* Discard unwanted metadata sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
