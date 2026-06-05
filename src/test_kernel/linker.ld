ENTRY(_start)

SECTIONS
{
    /* base output address (adjust with -Ttext or change here as needed) */
    . = 1M;
    .text : {
        /* ensure an 'entry' symbol in .text will be used as entry */
        *(.text*)
    }

    .rodata : {
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

    /DISCARD/ : {
        *(.comment*)
        *(.note*)
    }
}