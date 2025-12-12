/* linker.ld - flat binary output with 'entry' as entry point */

OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(entry)

SECTIONS
{
    /* base output address (adjust with -Ttext or change here as needed) */
    . = 0x7c00;

    .text : {
        /* ensure an 'entry' symbol in .text will be used as entry */
        *(.text.entry)
        *(.text*)
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }
}