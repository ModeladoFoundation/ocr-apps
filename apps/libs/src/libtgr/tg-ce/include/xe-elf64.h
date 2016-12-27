#ifndef __xe_elf64_h__
#define __xe_elf64_h__

// int tgr_load_xe_elf_file( block_info * bi, const char * filename, uint64_t *entry );
int tgr_load_xe_elf_image( block_info * bi, uint64_t imgAddr );

#endif // __xe_elf64_h__
