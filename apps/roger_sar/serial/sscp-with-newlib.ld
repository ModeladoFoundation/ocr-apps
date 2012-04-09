/* Default linker script, for normal executables */
OUTPUT_FORMAT("elf32-rmd", "elf32-bigrmd",
          "elf32-rmd")
ENTRY (_start)
MEMORY
{
  /*
   * RMD self-map view as per EAS 0.8.6
   *
   * The self-map hole is [0x000000000 - 0x200000000),
   * this describes its layout.
   */
  /*
   * Local RAM in self-map (accessible with load64.loc/store64.loc)
   */
  LOCAL_RAM     : ORIGIN = 0x000000000,  LENGTH = 2042K
  LOCAL_PRF     : ORIGIN = 0x000010000,  LENGTH = 4K
  LOCAL_MSR     : ORIGIN = 0x000011000,  LENGTH = 512
  LOCAL_PMU     : ORIGIN = 0x000011200,  LENGTH = 1K
  /*
   * Block RAM in self-map (accessible with load64.rem/store64.rem)
   */
  BLOCK0_MSRPMU : ORIGIN = 0x000000000, LENGTH = 2K
  BLOCK0_RAM    : ORIGIN = 0x000002000, LENGTH = 640K
  BLOCK1_MSRPMU : ORIGIN = 0x000100000, LENGTH = 2K
  BLOCK1_RAM    : ORIGIN = 0x000102000, LENGTH = 640K
  /*
   * DRAM in self-map (accessible with load64.rem/store64.rem)
   */
  CHIP_DRAM     : ORIGIN = 0x3030000, LENGTH = 8339264K
}
SECTIONS
{
  .text : {
        _ftext = .;
        crt0_lxe.o(.text);
        einit = .;
    *(.init)
    eprol = .;
    *(.text)
    *(.fini)
    etext  =  .;
    _etext  =  .;
  } > LOCAL_RAM
  . = .;
  .rodata : {
    *(.rodata*)
  } > LOCAL_RAM
   _fdata = ALIGN(16);
  .data : {
    *(.data)
    CONSTRUCTORS
  } > LOCAL_RAM /* Should be: CHIP_DRAM */ 
  .data_local : {  *(.data_local)  } > LOCAL_RAM
  .data_block : {  *(.data_block)  } > LOCAL_RAM /* Should be: BLOCK0_RAM or BLOCK1_RAM */
   _gp = ALIGN(16) + 0x8000;
   edata  =  .;
   _edata  =  .;
   _fbss = .;
  .bss : {
    *(.bss)
    *(COMMON)
  } > LOCAL_RAM /* Should be: CHIP_DRAM */
  .bss_local : {
    *(.bss_local)
    *(COMMON_LOCAL)
  } > LOCAL_RAM
  .bss_block : {
    *(.bss_block)
    *(COMMON_BLOCK)
  } > LOCAL_RAM /* Should be: BLOCK0_RAM */
   end = .;
   _end = .;
}
