/* Default linker script, for normal executables */
OUTPUT_FORMAT("elf32-rmd", "elf32-bigrmd", "elf32-rmd")
ENTRY (_start)
MEMORY
{
    /*
    * Local RAM in self-map
    */
    LOCAL_RAM     : ORIGIN = 0x00000000,  LENGTH = 64K
    /*
    * Block RAM in self-map
    */
    BLOCK0_RAM    : ORIGIN = 0x00200000, LENGTH = 1280K
    BLOCK1_RAM    : ORIGIN = 0x00400000, LENGTH = 1280K
    /*
    * DRAM in self-map
    */
    CHIP_DRAM     : ORIGIN = 0x200000000, LENGTH = 524288K /* 512MB */
}

SECTIONS
{
    /* Local RAM */
    .xe_tls : {
        _fxetls         = ABSOLUTE(.);
        xe_tls.o(.bss_local);
        _exetls         = ABSOLUTE(.);
    } > LOCAL_RAM

    .text : {
        . = ALIGN(8);
        _start          = ABSOLUTE(.);
        _ftext          = ABSOLUTE(.);
        xe_header.o(.text);    
        einit           = ABSOLUTE(.);
        *(.init)
        eprol           = ABSOLUTE(.);
        *(.text)
        *(.fini)
        etext           = ABSOLUTE(.);
        _etext          = ABSOLUTE(.);
    } > LOCAL_RAM

    .rodata_local : {
        *(.rodata_local)
    } > LOCAL_RAM

    .data_local : {
        . = ALIGN(8);
        _fdata_local    = ABSOLUTE(.);
        *(.data_local)
        edata_local     = ABSOLUTE(.);
        _edata_local    = ABSOLUTE(.);
    } > LOCAL_RAM

    .bss_local : {
        . = ALIGN(8);
        _fbss_local     = ABSOLUTE(.);
        *(.bss_local)
        ebss_local      = ABSOLUTE(.);
        _ebss_local     = ABSOLUTE(.);
        end_local       = ABSOLUTE(.);
        _end_local      = ABSOLUTE(.);
    } > LOCAL_RAM

    /* Block RAM */
    .rodata_block : {
        *(.rodata_block)
    } > BLOCK0_RAM

    .data_block : {
        . = ALIGN(8);
        _fdata_block    = ABSOLUTE(.);
        *(.data_block)
        edata_block     = ABSOLUTE(.);
        _edata_block    = ABSOLUTE(.);
    } > BLOCK0_RAM

    .bss_block : {
        . = ALIGN(8);
        _fbss_block     = ABSOLUTE(.);
        *(.bss_block)
        ebss_block      = ABSOLUTE(.);
        _ebss_block     = ABSOLUTE(.);
        end_block       = ABSOLUTE(.);
        _end_block      = ABSOLUTE(.);
    } > BLOCK0_RAM

    /* DRAM */
    .rodata : {
        *(.rodata)
    } > CHIP_DRAM

    .data : {
        . = ALIGN(8);
        _fdata          = ABSOLUTE(.);
        *(.data)
        edata           = ABSOLUTE(.);
        _edata          = ABSOLUTE(.);
    } > CHIP_DRAM

    .bss : {
        . = ALIGN(8);
        _fbss           = ABSOLUTE(.);
        *(.bss)
        *(COMMON)
        ebss            = ABSOLUTE(.);
        _ebss           = ABSOLUTE(.);
        end             = ABSOLUTE(.);
        _end            = ABSOLUTE(.);
    } > CHIP_DRAM
}
