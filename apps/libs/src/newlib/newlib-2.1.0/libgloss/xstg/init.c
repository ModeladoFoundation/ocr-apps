#include <reent.h>
#include <ocr.h>

__asm__(
    ".section .init\n"
    ".globl  _init\n"
    ".align 8\n"
    ".type   _init,@function\n"
);

u64 _init (void)
{
  u8 ret;
  ocrGuid_t affinity;
  ocrInDbAllocator_t allocator = NO_ALLOC;
  u64 addr, len;
  u16 flags = 0;

  // get the affinity of this EDT
  if (ocrAffinityGetCurrent(&affinity) != 0)
    return (u64)-1;

  // create the stack
  len = 0x8000;     // 32K ?
  ret = ocrDbCreate(&_REENT->stack_Guid, &addr, len, flags, affinity, allocator);

  if (ret != 0)
    return (u64)-1;
  else
    // return stack address
    return addr;
}
