/* Linker script for Virtual Boy with C++ support. */

OUTPUT_FORMAT("elf32-v810", "elf32-v810", "elf32-v810")
OUTPUT_ARCH(v810)
TARGET(elf32-v810)
ENTRY(start)

MEMORY
{
    /*wram    (!r)    : ORIGIN =  0x05000000, LENGTH = 64k*/
    wram    (!r)    : ORIGIN =  0x04000000, LENGTH = 16M  /* Put it in expansion */
    rom     (rx)    : ORIGIN =  0x07000000, LENGTH = 16M
}

__text_vma__ = ORIGIN(rom);
__data_vma__ = ORIGIN(wram);
__gp__ = __data_vma__ + 0x8000;
__gp = __data_vma__ + 0x8000; /* Used by GCC */
__stack__ = ORIGIN(wram) + LENGTH(wram) - 4;

SECTIONS
{
    .text __text_vma__ :
    {
        *(.text*)
        . = ALIGN(4);
        __ctor_list__ = .;
        KEEP (*(.ctor*))
        KEEP (*(.init_array))
        __ctor_list_end__ = .;
        . = ALIGN(4);
    } >rom = 0

    .rodata ALIGN(4): SUBALIGN(4)
    {
        *(.rodata*)
        *all.rodata*(*)
        /*KEEP(*(.rosdata.*))*/
        . = ALIGN(4);
    } >rom = 0

    __data_lma = .;
    .data __data_vma__ : AT(__data_lma)
    {
      PROVIDE (__data_start__ = .);
        *(.data*)
        *(.sdata*)
      PROVIDE (__data_end__ = .);
    } >wram = 0

    .bss (NOLOAD): AT(0)
    {
      PROVIDE (__bss_start__ = .);
        *(.sbss*)
        *(.scommon*)
        *(.bss*)
        *(COMMON)
      PROVIDE (__bss_end__ = .);
      _end = ABSOLUTE(.); /* Heap start */
    } >wram = 0

    end_size = 0x21F;
    end_min = ORIGIN(rom) + SIZEOF(.text) + SIZEOF(.rodata) + SIZEOF(.data) + end_size + 8; /* Enough room for BSS markers. */
    end_alignment = (1 << (LOG2CEIL(end_min - __text_vma__))) - end_size + __text_vma__ - 1;

    .rom_header end_alignment :
    {
        KEEP (*(.rom_header))
    } >rom = 0

    .interrupt_handlers :
    {
        KEEP (*(.interrupt_handlers))
    } >rom = 0
}
