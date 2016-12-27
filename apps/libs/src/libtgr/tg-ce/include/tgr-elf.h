#ifndef __TGR_ELF_H__
#define __TGR_ELF_H__
//
// We don't have a ready source of elf.h in newlib or binutils
// so this is a stripped down Linux version that fits the need of tgr-ce
//
#include <stdint.h>

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;

/* Types for signed and unsigned 64-bit quantities.  */
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

/* Type of addresses.  */
typedef uint64_t Elf64_Addr;

/* Type of file offsets.  */
typedef uint64_t Elf64_Off;

/* Type for section indices, which are 16-bit quantities.  */
typedef uint16_t Elf64_Section;

/* Type for version symbol information.  */
typedef Elf64_Half Elf64_Versym;


/* The ELF file header.  This appears at the start of every ELF file.  */

#define EI_NIDENT (16)

typedef struct
{
  unsigned char e_ident[EI_NIDENT];  /* Magic number and other info */
  Elf64_Half    e_type;              /* Object file type */
  Elf64_Half    e_machine;           /* Architecture */
  Elf64_Word    e_version;           /* Object file version */
  Elf64_Addr    e_entry;             /* Entry point virtual address */
  Elf64_Off     e_phoff;             /* Program header table file offset */
  Elf64_Off     e_shoff;             /* Section header table file offset */
  Elf64_Word    e_flags;             /* Processor-specific flags */
  Elf64_Half    e_ehsize;            /* ELF header size in bytes */
  Elf64_Half    e_phentsize;         /* Program header table entry size */
  Elf64_Half    e_phnum;             /* Program header table entry count */
  Elf64_Half    e_shentsize;         /* Section header table entry size */
  Elf64_Half    e_shnum;             /* Section header table entry count */
  Elf64_Half    e_shstrndx;          /* Section header string table index */
} Elf64_Ehdr;

/* Fields in the e_ident array.  The EI_* macros are indices into the
   array.  The macros under each EI_* macro are the values the byte
   may have.  */

#define EI_MAG0        0        /* File identification byte 0 index */
#define ELFMAG0        0x7f     /* Magic number byte 0 */

#define EI_MAG1        1        /* File identification byte 1 index */
#define ELFMAG1        'E'      /* Magic number byte 1 */

#define EI_MAG2        2        /* File identification byte 2 index */
#define ELFMAG2        'L'      /* Magic number byte 2 */

#define EI_MAG3        3        /* File identification byte 3 index */
#define ELFMAG3        'F'      /* Magic number byte 3 */

/* Conglomeration of the identification bytes, for easy testing as a word.  */
#define    ELFMAG     "\177ELF"
#define    SELFMAG    4

#define EI_CLASS      4        /* File class byte index */
#define ELFCLASSNONE  0        /* Invalid class */
#define ELFCLASS32    1        /* 32-bit objects */
#define ELFCLASS64    2        /* 64-bit objects */
#define ELFCLASSNUM   3

#define EI_DATA       5        /* Data encoding byte index */
#define ELFDATANONE   0        /* Invalid data encoding */
#define ELFDATA2LSB   1        /* 2's complement, little endian */
#define ELFDATA2MSB   2        /* 2's complement, big endian */
#define ELFDATANUM    3

#define EI_VERSION    6        /* File version byte index */
                               /* Value must be EV_CURRENT */
/* Legal values for e_version (version).  */

#define EV_NONE       0        /* Invalid ELF version */
#define EV_CURRENT    1        /* Current version */
#define EV_NUM        2

#define EI_PAD        9        /* Byte index of padding bytes */

/* Legal values for e_type (object file type).  */

#define ET_NONE       0        /* No file type */
#define ET_REL        1        /* Relocatable file */
#define ET_EXEC       2        /* Executable file */
#define ET_DYN        3        /* Shared object file */
#define ET_CORE       4        /* Core file */
#define ET_NUM        5        /* Number of defined types */
#define ET_LOOS       0xfe00   /* OS-specific range start */
#define ET_HIOS       0xfeff   /* OS-specific range end */
#define ET_LOPROC     0xff00   /* Processor-specific range start */
#define ET_HIPROC     0xffff   /* Processor-specific range end */

/* Legal values for e_machine (architecture).  */

#define EM_NONE       0        /* No machine */
#define EM_TG         0x5555
#define EM_XSTG       0x5554   /* marks an executable as XSTG architecture */

#define EF_XSTG_PIE   0x00000004  /* marks an executable as PIE linked */

/* Section header.  */

typedef struct
{
  Elf64_Word    sh_name;        /* Section name (string tbl index) */
  Elf64_Word    sh_type;        /* Section type */
  Elf64_Xword   sh_flags;       /* Section flags */
  Elf64_Addr    sh_addr;        /* Section virtual addr at execution */
  Elf64_Off     sh_offset;      /* Section file offset */
  Elf64_Xword   sh_size;        /* Section size in bytes */
  Elf64_Word    sh_link;        /* Link to another section */
  Elf64_Word    sh_info;        /* Additional section information */
  Elf64_Xword   sh_addralign;   /* Section alignment */
  Elf64_Xword   sh_entsize;     /* Entry size if section holds table */
} Elf64_Shdr;


/* Legal values for sh_flags (section flags).  */

#define SHF_WRITE            (1 << 0)    /* Writable */
#define SHF_ALLOC            (1 << 1)    /* Occupies memory during execution */
#define SHF_EXECINSTR        (1 << 2)    /* Executable */
#define SHF_MERGE            (1 << 4)    /* Might be merged */
#define SHF_STRINGS          (1 << 5)    /* Contains nul-terminated strings */
#define SHF_INFO_LINK        (1 << 6)    /* `sh_info' contains SHT index */
#define SHF_LINK_ORDER       (1 << 7)    /* Preserve order after combining */
#define SHF_OS_NONCONFORMING (1 << 8)    /* Non-standard OS specific handling required */
#define SHF_GROUP            (1 << 9)    /* Section is member of a group.  */
#define SHF_TLS              (1 << 10)   /* Section hold thread-local data.  */
#define SHF_COMPRESSED       (1 << 11)   /* Section with compressed data. */
#define SHF_MASKOS           0x0ff00000  /* OS-specific.  */
#define SHF_MASKPROC         0xf0000000  /* Processor-specific */
#define SHF_XSTG_RAR         0x02000000  /* section flag for .pie_note section */

/* Program segment header.  */

typedef struct
{
  Elf64_Word    p_type;         /* Segment type */
  Elf64_Word    p_flags;        /* Segment flags */
  Elf64_Off     p_offset;       /* Segment file offset */
  Elf64_Addr    p_vaddr;        /* Segment virtual address */
  Elf64_Addr    p_paddr;        /* Segment physical address */
  Elf64_Xword   p_filesz;       /* Segment size in file */
  Elf64_Xword   p_memsz;        /* Segment size in memory */
  Elf64_Xword   p_align;        /* Segment alignment */
} Elf64_Phdr;


/* Legal values for p_type (segment type).  */

#define PT_NULL        0        /* Program header table entry unused */
#define PT_LOAD        1        /* Loadable program segment */

/* Legal values for p_flags (segment flags).  */

#define PF_X        (1 << 0)    /* Segment is executable */
#define PF_W        (1 << 1)    /* Segment is writable */
#define PF_R        (1 << 2)    /* Segment is readable */

#endif // __TGR_ELF_H__
