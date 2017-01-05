/*
 * Test tgr static linker script, for normal executables
 * Note that this is useful ONLY for single block testing on FSIM
 * With more than one block all the blocks will be accessing the same
 * global data, breaking the 1 block 1 application rule for TGR.
 */
OUTPUT_FORMAT("elf64-xstg", "elf64-bigxstg", "elf64-xstg")
SEARCH_DIR("../../../../../tg/tg/install/xstg-linux-elf/lib");
ENTRY (_start)

/*
 * XSTG memory region definitions. For fsim test configurations we use
 * the full 2MB block L2 space available.
 *
 * NOTE: Any region that can contain executable code should be rendered
 *       as a machine relative address to avoid breaking the C++ libraries
 *       (exception handling) and gdb. Other addresses can be self-referential.
 */
MEMORY
{
  /*
   * Agent L1 in self-map (agent relative)
   */
  AGENT_L1     : ORIGIN = 0x000000030000, LENGTH = 64K
  /*
   * Block L2 in block relative
   */
  BLOCK_L2     : ORIGIN = 0x0000000000C00000, LENGTH = 4096K /* 2MB */
  /*
   * Socket IPM in socket relative
   */
  SOCKET_IPM   : ORIGIN = 0x0000240000000000, LENGTH = 134217728K /* 128GB */
}
/*
 * Region mapping for Fsim block-mode config.
 */
REGION_ALIAS( "REGION_PRIVATE", AGENT_L1 );
REGION_ALIAS( "REGION_PRIVATE_RO", AGENT_L1 );  /* change to LOCAL? */
REGION_ALIAS( "REGION_LOCAL", BLOCK_L2 );
REGION_ALIAS( "REGION_LOCAL_RO", BLOCK_L2 );
REGION_ALIAS( "REGION_GLOBAL", SOCKET_IPM );
REGION_ALIAS( "REGION_GLOBAL_RO", SOCKET_IPM );

/*
 * Section definitions
 */
SECTIONS
{
  .rodata_local : {
    . = ALIGN(8);
    *(.rodata_local)
  } > REGION_PRIVATE_RO

  .data_local : {
    . = ALIGN(8);
    _fdata_local    = ABSOLUTE(.);
    *(.data_local)
    edata_local     = ABSOLUTE(.);
    _edata_local    = ABSOLUTE(.);
  } > REGION_PRIVATE

  .bss_local : {
    . = ALIGN(8);
    _fbss_local     = ABSOLUTE(.);
    *(.bss_local)
    ebss_local      = ABSOLUTE(.);
    _ebss_local     = ABSOLUTE(.);
    end_local       = ABSOLUTE(.);
    _end_local      = ABSOLUTE(.);
  } > REGION_PRIVATE

  /* Block L2 */
  .rodata_block : {
    . = ALIGN(8);
    *(.rodata_block)
  } > REGION_LOCAL_RO

  .data_block : {
    . = ALIGN(8);
    _fdata_block    = ABSOLUTE(.);
    *(.data_block)
    edata_block     = ABSOLUTE(.);
    _edata_block    = ABSOLUTE(.);
  } > REGION_LOCAL

  .bss_block : {
    . = ALIGN(8);
    _fbss_block     = ABSOLUTE(.);
    *(.bss_block)
    ebss_block      = ABSOLUTE(.);
    _ebss_block     = ABSOLUTE(.);
    end_block       = ABSOLUTE(.);
    _end_block      = ABSOLUTE(.);
  } > REGION_LOCAL

  /* Global Read-only sections */
  .text : {
    . = ALIGN(8);
    _ftext          = ABSOLUTE(.);
    einit           = ABSOLUTE(.);
    *(.init)
    eprol           = ABSOLUTE(.);
    *(.text)
    *(.fini)
    etext           = ABSOLUTE(.);
    _etext          = ABSOLUTE(.);
  } > REGION_GLOBAL_RO

  .rodata : {
    . = ALIGN(8);
    *(.rodata)
  } > REGION_GLOBAL_RO

  .ctors :
  {
    /* we use crt0.o to find the start of
       the constructors, so we make sure it is first.
    */
    KEEP (crt0*.o(.ctors))
    /*KEEP (crt0?.o(.ctors))*/
    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  } > REGION_GLOBAL_RO

  .dtors :
  {
    KEEP (crt0*.o(.dtors))
    KEEP (*(EXCLUDE_FILE (crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  } > REGION_GLOBAL_RO

  .eh_frame_hdr :
  {
  	PROVIDE_HIDDEN( __eh_frame_hdr_start = . );
    *(.eh_frame_hdr)
  } > REGION_GLOBAL_RO
  PROVIDE_HIDDEN( __eh_frame_hdr_size = SIZEOF(.eh_frame_hdr) );

  .eh_frame :
  {
  	PROVIDE_HIDDEN( __eh_frame_start = . );
    KEEP (*(.eh_frame))
  } > REGION_GLOBAL_RO
  PROVIDE_HIDDEN( __eh_frame_size = SIZEOF(.eh_frame) );

  .gcc_except_table : ONLY_IF_RO
  {
  	*(.gcc_except_table .gcc_except_table.*)
  } > REGION_GLOBAL_RO

  .tdata :
  {
  	PROVIDE_HIDDEN( __tls_start = . );
    KEEP (*(.tdata))
  } > REGION_GLOBAL_RO

  .tbss :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN( _ftbss = . );
    KEEP (*(.tbss))
    PROVIDE_HIDDEN( _etbss = . );
  } > REGION_GLOBAL_RO
  PROVIDE_HIDDEN( __tls_size = SIZEOF(.tdata) + SIZEOF(.tbss) );

  /* Global Read/Write sections */
  .data : {
    . = ALIGN(8);
    _fdata          = ABSOLUTE(.);
    *(.data)
    edata           = ABSOLUTE(.);
    _edata          = ABSOLUTE(.);
  } > REGION_GLOBAL

  .bss : {
    . = ALIGN(8);
    _fbss           = ABSOLUTE(.);
    *(.bss)
    *(COMMON)
    ebss            = ABSOLUTE(.);
    _ebss           = ABSOLUTE(.);
    end             = ABSOLUTE(.);
    _end            = ABSOLUTE(.);
  } > REGION_GLOBAL
}
