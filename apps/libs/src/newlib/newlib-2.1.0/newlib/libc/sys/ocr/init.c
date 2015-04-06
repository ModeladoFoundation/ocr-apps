#include <reent.h>
#include <sys/ocr.h>

#define APP_STACK_SIZE  0x8000;     // 32K ?
//
// Do our OCR initialization dance
// Allocate a user stack and return its top or 0 on error.
//
u64 _init( int argc, char** argv, ocrConfig_t *oc )
{
  ocrGuid_t affinity;
  //
  // initialize OCR
  //
  ocrParseArgs(argc, (const char **) argv, oc);
  ocrInit(&_REENT->_ocr.legacyContext, oc);

  // get the affinity of this EDT
  if (ocrAffinityGetCurrent(&affinity) != 0)
    return 0L;
  //
  // create the stack
  //
  u16 flags = 0;

  _REENT->_ocr.stack_size = APP_STACK_SIZE;

  u8 ret = ocrDbCreate( &_REENT->_ocr.stack_Guid,
                        &_REENT->_ocr.stack_start,
                        _REENT->_ocr.stack_size,
                        flags, affinity, NO_ALLOC );

  if (ret == 0)
    ret = ocrLegacyContextAssociateMemory(_REENT->_ocr.legacyContext,
                                          _REENT->_ocr.stack_Guid,
                                          LEGACY_MEM_STACK);
  if (ret != 0)
    return 0L;
  else
    //
    // Stacks grow down, so we return the address just past
    // the top of the stack
    //
    return _REENT->_ocr.stack_start + _REENT->_ocr.stack_size;
}
