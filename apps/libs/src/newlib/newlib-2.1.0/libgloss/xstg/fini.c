#include <reent.h>
#include <ocr.h>

__asm__(
    ".section .fini\n"
    ".globl  _fini\n"
    ".align 8\n"
    ".type   _fini,@function\n"
);

void _fini (void)
{
  // destroy stack
  ocrDbDestroy(_REENT->stack_Guid);
  return;
}
