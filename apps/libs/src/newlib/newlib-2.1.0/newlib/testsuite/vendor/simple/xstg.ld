/* Linker script for Traleika Glacier systems. */
ENTRY(_start)
OUTPUT_FORMAT(elf64-xstg)
TARGET(elf64-xstg)
SEARCH_DIR(.)

MEMORY
{
  /*
   * Agent L1 in self-map
   */
  AGENT_L1     : ORIGIN = 0x000000030000, LENGTH = 64K

  /*
   * Block L2 in self-map
   */
  BLOCK_L2     : ORIGIN = 0x000000c00000, LENGTH = 2048K /* 2MB */

  /*
   * Cluster L3 in self-map
   */
  CLUSTER_L3   : ORIGIN = 0x000018000000, LENGTH = 8192K /* 8MB */

  /*
   * Socket IPM in self-map
   */
  SOCKET_IPM   : ORIGIN = 0x240000000000, LENGTH = 134217728K /* 128GB */

  /*
   * Socket DRAM in self-map
   */
  SOCKET_DRAM  : ORIGIN = 0x280000000000, LENGTH = 536870912K /* 512GB */

  /*
   * Socket NVM in self-map
   */
  SOCKET_NVM   : ORIGIN = 0x300000000000, LENGTH = 4294967296K /* 4TB */
}

SECTIONS
{
  .text : {
    . = ALIGN(8);
    _ftext          = ABSOLUTE(.);
    _start          = ABSOLUTE(.);
    einit           = ABSOLUTE(.);
    *(.init)
    eprol           = ABSOLUTE(.);
    *(.text)
    *(.fini)
    etext           = ABSOLUTE(.);
    _etext          = ABSOLUTE(.);
  } > SOCKET_DRAM


  /* Agent L1 */
  .rodata_local : {
    . = ALIGN(8);
    *(.rodata_local)
  } > AGENT_L1

  .data_local : {
    . = ALIGN(8);
    _fdata_local    = ABSOLUTE(.);
    *(.data_local)
    edata_local     = ABSOLUTE(.);
    _edata_local    = ABSOLUTE(.);
  } > AGENT_L1

  .bss_local : {
    . = ALIGN(8);
    _fbss_local     = ABSOLUTE(.);
    *(.bss_local)
    ebss_local      = ABSOLUTE(.);
    _ebss_local     = ABSOLUTE(.);
    end_local       = ABSOLUTE(.);
    _end_local      = ABSOLUTE(.);
  } > AGENT_L1


  /* Block L2 */
  .rodata_block : {
    . = ALIGN(8);
    *(.rodata_block)
  } > BLOCK_L2

  .data_block : {
    . = ALIGN(8);
    _fdata_block    = ABSOLUTE(.);
    *(.data_block)
    edata_block     = ABSOLUTE(.);
    _edata_block    = ABSOLUTE(.);
  } > BLOCK_L2

  .bss_block : {
    . = ALIGN(8);
    _fbss_block     = ABSOLUTE(.);
    *(.bss_block)
    ebss_block      = ABSOLUTE(.);
    _ebss_block     = ABSOLUTE(.);
    end_block       = ABSOLUTE(.);
    _end_block      = ABSOLUTE(.);
  } > BLOCK_L2


  /* Socket DRAM */
  .rodata : {
    . = ALIGN(8);
    *(.rodata)
  } > SOCKET_DRAM

  .data : {
    . = ALIGN(8);
    _fdata          = ABSOLUTE(.);
    *(.data)
    edata           = ABSOLUTE(.);
    _edata          = ABSOLUTE(.);
  } > SOCKET_DRAM

  .bss : {
    . = ALIGN(8);
    _fbss           = ABSOLUTE(.);
    *(.bss)
    *(COMMON)
    ebss            = ABSOLUTE(.);
    _ebss           = ABSOLUTE(.);
    end             = ABSOLUTE(.);
    _end            = ABSOLUTE(.);
  } > SOCKET_DRAM
}

