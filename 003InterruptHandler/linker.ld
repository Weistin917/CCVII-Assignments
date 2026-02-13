ENTRY(_start)

MEMORY
{
    ram : ORIGIN = 0x82000000, LENGTH = 256K
}

SECTIONS
{
    . = ORIGIN(ram);

    .text :
    {
        *(.text*)
        *(.rodata*)
    } > ram

    .data : AT(ADDR(.text) + SIZEOF(.text))
    {
        __data_start__ = .;
        *(.data*)
        __data_end__ = .;
    } > ram

    .bss :
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;

        . = ALIGN(8);
        _irq_stack_bottom = .;
        . += 0x1000;
        _irq_stack_top = .;

        . = ALIGN(8);
        _stack_bottom = .;
        . += 0x2000;
        _stack_top = .;
    } > ram
}
