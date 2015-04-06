#include <reent.h>
#include <sys/ocr.h>

void _fini (void)
{
  u8 ret;

  ocrLegacyContextRemoveMemory(_REENT->_ocr.legacyContext,
                               _REENT->_ocr.stack_Guid,
                               LEGACY_MEM_STACK);

  // destroy stack
  ocrDbDestroy(_REENT->_ocr.stack_Guid);

  ocrFinalize(_REENT->_ocr.legacyContext);

  return;
}
