#include <stdint.h>
#include <alloca.h>

#include <tg-console.h>
#include <tg-remap.h>
#include <xstg-map.h>

#include "tgr-ce.h"
#include "tgr-elf.h"
#include "util.h"
#include "mem-util.h"

//
// XXX Totally out of date text below
// Static and PIE linked XE ELF executable loader
// This loader detects the static/PIE format and loads appropriately.
// The entry point is in tg_load_xe_elf() at the bottom of this file.
//
// General process
//  1. The 'master' CE validates the ELF image and creates a segment table
//     of SegmentEntry structures that describe the memory to be copied.
//     The 'slave' CEs wait on a barrier until the 'master' has created the
//     segment table and copied it up to the TgKrnlInfo structure in the CfgArea.
//
//     For PIE executables, the 'master' also allocates memory (L1, L2, DRAM
//     as appropriate) for the segments and parses the .pie_note section (marked
//     by a section flag) to get the per segment base register numbers (RARs)
//     which are placed in their segment table entries.
//
//  2. All CEs process the segment table built in step 1, doing:
//     a. copying the L2 destined ELF segments to their block L2
//     b. zeroing out any difference between segment memory and ELF file size.
//     c. creating xe-loader table entries for L1 destined segments
//     d. creating table entries for zero'd segment memory (as in b.)
//     e. copying the xe-loader image and loader table to each XE's L1.
//     The CEs rendezvous at the end of this process via another barrier.
//
//  3. All CEs initialize each XE's xe-loader and RAR registers as necessary.
//     All XEs are taken out of power gate state and allowed to start running.
//
// Notes:
//  The PIE allocation code makes assumptions about the destinations of the
//  ELF segments based on current usage. This should be fixed if/when the memory
//  'class' is encoded into the RAR info in the .pie_note section.
//
//  Since the basic model for openmp (and the tgr in general) on TG  is that each block
//  is a separate application, then each block gets its own copy of ELF global data
//  The ELF text may be shared and there are hooks in the block_info structure that
//  indicate whether a block should load the text segment or not. Global data is always
//  loaded per-block.
//  ** Although a block may not load text, the text segment RAR for a PIE ELF MUST be
//     set for all XEs in all blocks using the segment.
//
// Segment structure used by CEs to copy the XE executable segments to their
// appropriate address.
//
typedef struct {
    int index;      // Segment index in ELF
    int type;       // Segment type - SEG_*
    bool isPIE;     // Whether this is a segment in a PIE ELF
    u64 reg;        // RAR register - used by PIE only
    u64 src_addr;   // Segment src address (in ELF image in CE private memory)
    u64 src_tgaddr; // Segment src address converted to DMA usable address
    u64 dst_addr;   // Segment dst address (DRAM, IPM, L3, L2, L1)
    u32 src_len;    // Length in bytes of the src region
    u32 mem_len;    // Length in bytes of the segment (zero fill remainder)
} SegmentEntry;

//
// These are 1-1 with the PIE ELF segment order
//
enum {
    SEG_TEXT   = 0, // IPM (text, rodata)
    SEG_LOCAL  = 1, // L1 (or L2 if PIE and promoted)
    SEG_BLOCK  = 2, // L2
    SEG_GLOBAL = 3  // IPM (data)
};

static const char * mem_type_strs[] = {
    "text",
    "local",
    "block",
    "global"
};

#define MEM_STR(t)  (mem_type_strs[(t)])

//
// Forwards
//

//
// Find and parse the .pie_note section of a PIE ELF executable
// 'rars is a pointer to eh->phnum ints for holding the per segment RAR values.
// Returns 0 on success, -1 on failure.
//
static int tg_parse_pie( u64 imgAddr, Elf64_Ehdr * eh, int * rars )
{
    int rar_size = 0;
    u64 shoff = imgAddr + eh->e_shoff;
    Elf64_Shdr * sh = NULL;
    //
    // search for the .pie_note section - marked by section flags
    //
    for( int i = 0 ; i < eh->e_shnum ; i++ ) {
        sh = (Elf64_Shdr *) (shoff + i * eh->e_shentsize);

        if( (sh->sh_flags & SHF_MASKOS) == SHF_XSTG_RAR ) {
            rar_size = sh->sh_size;
            break;
        }
    }
    if( rar_size == 0 ) {
        ce_error( "ELF","Mal-formed XE PIE executable - no .pie_note section\n" );
        return -1;
    }
    //
    // Retrieve and parse the RAR note data
    //
    uint8_t * rar_data = alloca( rar_size );
    memcpy( rar_data, (void *)(imgAddr + sh->sh_offset), rar_size );
    //
    // extract the table
    // Used to overlay the note header
    //
    typedef struct {
        uint32_t namesize;
        uint32_t descsize;
        uint32_t type;
    } Rar_header ;
    #define RAR_HEADER_SIZE    12  // can't use sizeof(Rar_header)

    const Rar_header * rh = (const Rar_header *) rar_data;

    if( rar_size < sizeof(Rar_header) ||
        rar_size != RAR_HEADER_SIZE + rh->namesize + rh->descsize ) {

        ce_error( "ELF","ill formed RAR note section!\n");
        return 1;
    }

    if( rh->descsize / 8 != eh->e_phnum ) {
        ce_error( "ELF","PHDR and RAR count mismatch (%d != %d)!\n",
                    eh->e_phnum, rh->descsize/8 );
        return 1;
    }
    //
    // Copy the RAR entries into our table
    // We're going to cheat here because we know that a register number
    // is only 2 bytes max and never negative, don't bother with all 8 bytes.
    //
    rar_data = rar_data + RAR_HEADER_SIZE + rh->namesize;

    for( int i = 0 ; i < eh->e_phnum ; i++ ) {
        *rars = 0;
        *(u8 *) rars = * rar_data;
        *((u8 *)rars + 1) = * (rar_data + 1);
        rar_data += sizeof(uint64_t);
        rars++;
    }
    return 0;
}

//
// If segment in-memory length is more than in the ELF file zero the remainder
//
__attribute__((unused))
static int zero_bss( SegmentEntry * seg, uint64_t addr )
{
    if( (int)seg->mem_len - seg->src_len ) {
        uint64_t bss_len   = seg->mem_len - seg->src_len;
        uint64_t bss_start = addr + seg->src_len;

        ce_print("ELF", " -   Zeroing %d bytes at %p\n", bss_len, bss_start );
        tg_zero( bss_start, (int) bss_len );
    }
    return 0;
}

//
// If we've got a PIE ELF, set all the block's XE's RAR associated with
// this segment.
//
static void set_pie_rar( block_info *bi, SegmentEntry * seg )
{
    //
    // setup the segment register for all XEs
    //
    if( seg->isPIE && seg->type != SEG_LOCAL ) {
        //
        // Since XEs use MR format addresses for the PC, we convert our TEXT
        // segment RARs into MR from SR (assuming policy of all text in global)
        //
        uint64_t dst_addr = seg->dst_addr;

        if( seg->type == SEG_TEXT ) {
            dst_addr = SR_TO_MR(bi->id.rack, bi->id.cube, bi->id.socket, dst_addr);
        }
        for( xe_info *xei = NULL ; (xei = xe_get_next_info( bi, xei )) ; ) {
            xe_set_reg( xei, seg->reg, dst_addr ); // set base addr
            ce_vprint("ELF", " -   XE %d : R%d 0x%llx\n",
                    xei->id.agent, seg->reg, dst_addr );
        }
    }
}

//
// Copy a local segment into the block's XE complement.
// If we're PIE, we also allocate the memory we need.
// If not, we just reserve the assigned address in the XE's L1 region.
//
static int load_local_seg( block_info * bi, SegmentEntry * seg )
{
    uint64_t addr;

    ce_print("ELF", "   - Copying local segment into XE L1 memory\n", seg->index );
    //
    // Copy in the L1 segments
    //
    for( xe_info *xei = NULL ; (xei = xe_get_next_info( bi, xei )) ; ) {
        //
        // Find memory for this segment
        //
        if( seg->isPIE ) {
            //
            // Try to allocate for L1 and if a no-go, try for L2
            // Note, if L2 then each XE gets a different address
            //
            mem_seg * mseg = xe_alloc_mem( xei, Mem_L1, seg->mem_len );

            if( mseg == NULL ) {
                mseg = block_alloc_mem( bi, Mem_L2, seg->mem_len, 0, xei->id.agent );
                if( mseg == NULL ) {
                    ce_error("ELF","PIE alloc: can't get %d bytes of local mem for XE %d\n",
                            seg->mem_len, XEI_NUM(xei));
                    return 1;
                }
            }
            ce_print("ELF", "   - PIE allocated XE %d segment at %p\n",
                    XEI_NUM(xei), mseg->va );
            addr = seg->dst_addr = mseg->va;
            //
            // Could we have an entry point in local? XXX
            //

            //
            // setup the RAR here because each XE may have gotten a different
            // address if we promoted to L2
            //
            xe_set_reg( xei, seg->reg, addr ); // set base addr
            ce_print("ELF", "   - RAR R%d set to 0x%llx\n", seg->reg, addr );
        //
        // Static linked ELF - Reserve this address range
        //
        } else {
            //
            // We setup the L1 memory using CR address, convert dst_addr to CR here
            // and reserve. We DON'T modify sed->dst_addr as we need that on the
            // subsequent iterations.
            //
            addr = validate_xe_addr( xei, seg->dst_addr, seg->mem_len );

            if( xe_alloc_mem_at( xei, mem_type_of(addr), addr, seg->mem_len ) == NULL ) {
                ce_error("ELF","xe-elf64: memory reservation error!\n");
                return 1;
            }
        }
        if( seg->src_len )
            (void) tg_memcpy( addr, seg->src_tgaddr, seg->src_len);
#ifdef CLEAR_BSS
        zero_bss( seg, addr );
#endif // CLEAR_BSS
    }
    return 0;
}

//
// Load a Block segment
//
static int load_block_seg( block_info * bi, SegmentEntry * seg )
{
    uint64_t addr;

    ce_print("ELF", "   - Copying block segment into XE L2 memory\n", seg->index );

    //
    // Allocate space for PIE if necessary
    //
    if( seg->isPIE ) {
        mem_seg * mseg = block_alloc_mem( bi, Mem_L2, seg->mem_len, 0, CE );

        if( mseg == NULL ) {
            ce_error("ELF","Can't allocate %d bytes of block mem for PIE\n",
                    seg->mem_len);
            return 1;
        }
        ce_print("ELF", "   - PIE allocated segment at %p\n", mseg->va );
        addr = seg->dst_addr = mseg->va;
    //
    // Reserve this address range
    //
    } else {
        addr = validate_addr( bi, seg->dst_addr, seg->mem_len );

        if( block_alloc_mem_at( bi, mem_type_of(addr), addr, seg->mem_len ) == NULL ) {
            ce_error("ELF","xe-elf64: memory reservation conflict!\n");
            return 1;
        }
    }
    //
    // Copy it to memory
    //
    if( seg->src_len )
        (void) tg_memcpy( addr, seg->src_tgaddr, seg->src_len);

#ifdef CLEAR_BSS
    zero_bss( seg, addr );
#endif // CLEAR_BSS
    return 0;
}

//
// Text and Global
//
static int load_global_seg( block_info * bi, SegmentEntry * seg )
{
    uint64_t addr;
    ce_print("ELF", "   - Copying global segment (type %s) into XE IPM memory\n",
            MEM_STR(seg->type) );
    //
    // in the case that the policy is to have only 1 block load text,
    // we check the block policy
    //
    // XXX we assume that there are no global RO data segments, and that RO data
    // is merged into the text segment.
    //
    if( seg->type == SEG_TEXT && bi->load_text == 0 )
        return 0;
    //
    // Allocate space for PIE if necessary
    //
    if( seg->isPIE ) {
        mem_seg * mseg = global_alloc_mem( bi->ce, seg->mem_len, 0, CE );
        if( mseg == NULL ) {
            ce_error("ELF","Can't allocate %d bytes of IPM mem for PIE\n", seg->mem_len);
            return 1;
        }
        ce_print("ELF", "   - PIE allocated segment at %p\n", mseg->va );
        addr = seg->dst_addr = mseg->va;

        if( seg->type == SEG_TEXT )
            bi->entry += addr;
    //
    // Reserve this address range
    //
    } else {
        addr = validate_addr( bi, seg->dst_addr, seg->mem_len );
        ce_print("ELF", "    - allocating %p -> %p, 0x%lx bytes\n",
                seg->dst_addr, addr, seg->mem_len);

        if( block_alloc_mem_at( bi, mem_type_of(addr), addr, seg->mem_len ) == NULL ) {
            ce_error("ELF","xe-elf64: memory reservation conflict!\n");
            return 1;
        }
    }
    if( seg->src_len )
        (void) tg_memcpy( addr, seg->src_tgaddr, seg->src_len);
#ifdef CLEAR_BSS
    zero_bss( seg, addr );
#endif // CLEAR_BSS

    return 0;
}

//
// imgAddr is the address of the first byte of the XE ELF image
//
// This is largely copied from tgkrnl, but with PIE support added and
// all image loading is done via CE - no XE loader.
//
int tgr_load_xe_elf_image( block_info * bi, uint64_t imgAddr )
{
    //
    // Start with the ELF64 header
    //
    Elf64_Ehdr * eh = (void *) imgAddr;

    // Elf?
    if(eh->e_ident[EI_MAG0] != ELFMAG0 ||
       eh->e_ident[EI_MAG1] != ELFMAG1 ||
       eh->e_ident[EI_MAG2] != ELFMAG2 ||
       eh->e_ident[EI_MAG3] != ELFMAG3) {
        ce_error( "ELF","Not an ELF image!\n");
        return -1;
    }

    // Elf64?
    if(eh->e_ident[EI_CLASS] != ELFCLASS64) {
        ce_error( "ELF","ELF, but not an ELF64 image!\n");
        return -1;
    }

    // Elf64 LSB?
    if(eh->e_ident[EI_DATA] != ELFDATA2LSB) {
        ce_error( "ELF","ELF64, but not LSB image!\n");
        return -1;
    }

    // Elf64 LSB Current Version?
    if(eh->e_ident[EI_VERSION] != EV_CURRENT) {
        ce_error( "ELF","ELF64 LSB, but not current version image!\n");
        return -1;
    }

    // EXEC?
    if(eh->e_type != ET_EXEC) {
        ce_error( "ELF","Not an executable image!\n");
        return -1;
    }

    // Traleika Glacier?
    if(eh->e_machine != EM_XSTG) {
        ce_error( "ELF","Not an XSTG image!\n");
        return -1;
    }
    //
    // PIE or statically linked?
    //
    int isPIE = eh->e_flags & EF_XSTG_PIE;

    // Well-structured program header(s) exist?
    if(!eh->e_phnum || eh->e_phentsize != sizeof(Elf64_Phdr)) {
        ce_error( "ELF","No program headers?\n");
        return -1;
    }

    // Well-structured section header(s) exist?
    if(!eh->e_shnum || eh->e_shentsize != sizeof(Elf64_Shdr)) {
        ce_error( "ELF","No section headers?\n");
        return -1;
    }

    ce_print("ELF", " - Valid XE ELF64 header%s found\n", isPIE ? " (PIE format)":"");

    int * rars = alloca( sizeof(int) *  eh->e_phnum );

    for( int i = 0; i < eh->e_phnum; i++ )
        rars[i] = 0;

    if( isPIE ) {
        if( tg_parse_pie( imgAddr, eh, rars )) {
            return -1;
        }
    }
    //
    // Remember entry addr
    //
    bi->entry = eh->e_entry;
    //
    // Init a segment then load that segment, while iterating through the
    // program headers, ignoring uninteresting entries.
    //
    uint64_t phstart = imgAddr + eh->e_phoff;

    for( int i = 0; i < eh->e_phnum; i++ ) {
        Elf64_Phdr * ph = (Elf64_Phdr *)(phstart + i * eh->e_phentsize);
        SegmentEntry seg;
        //
        // Skip uninteresting header entries
        //
        if(ph->p_type != PT_LOAD || ph->p_memsz == 0) {
            ce_print("ELF", " - Segment %d : type 0x%x (0x%x bytes) not loaded\n",
                    i, ph->p_type, ph->p_memsz );
            continue;
        }
        if(ph->p_filesz & 0x7 || ph->p_memsz & 0x7) {
            ce_print( "ELF", "Note: XE ELF Segment size not 64bit aligned\n");
        }
        //
        // populate the segment structure
        //
        seg.index = i;
        seg.type = i;
        seg.isPIE = isPIE;
        seg.src_addr   = imgAddr + ph->p_offset;
        seg.src_tgaddr = (uint64_t) x86_to_tg( (void *) seg.src_addr );
        seg.src_len = ph->p_filesz;
        seg.mem_len = ph->p_memsz;
        seg.reg     = rars[i];

        if( ! isPIE ) {
            seg.dst_addr = ph->p_vaddr & ~CACHED;

            // Validate alignment
            if(ph->p_align > 1 &&
               ((seg.dst_addr % ph->p_align ) != (ph->p_offset % ph->p_align))) {
                ce_error("ELF","Badly aligned ELF64 image! Aborting...\n");
                return -1;
            }
            //
            // Set the segment type
            //
            switch( mem_type_of( seg.dst_addr ) ) {
              case Mem_L1: seg.type = SEG_LOCAL; break;
              case Mem_L2: seg.type = SEG_BLOCK; break;
              case Mem_IPM:
                seg.type = ( ph->p_flags & PF_X ) ? SEG_TEXT : SEG_GLOBAL;
                break;
              default:
                ce_error("ELF","segment %d bad memory type %d\n",
                        i, mem_type_of( seg.dst_addr ) );
                return -1;
            }
        }
        ce_print("ELF", " - Segment %d : type %s, 0x%x bytes at %p\n",
                i, MEM_STR(seg.type), ph->p_memsz, ph->p_vaddr );
        //
        // load the seg into memory
        //
        switch( seg.type ) {
          case SEG_LOCAL :
            if( load_local_seg( bi, & seg ) )
                return 1;
            break;

          case SEG_BLOCK :
            if( load_block_seg( bi, & seg ) )
                return 1;
            break;

          case SEG_TEXT :
          case SEG_GLOBAL :
            if( load_global_seg( bi, & seg ) )
                return 1;
            break;

          default:
            ce_error("ELF","Unknown segment type !!!\n");
            return 1;
        }
        set_pie_rar( bi, & seg );
    }
    return 0;
}
