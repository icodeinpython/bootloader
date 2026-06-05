ENTRY(lm_init)
OUTPUT_FORMAT("binary")
SECTIONS
{
    . = 0xf400;
    .text : {
        *(.text*)
    }

    /DISCARD/ : {
        *(.data*)
        *(.bss*)
    }
}