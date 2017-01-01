#ifndef INCLUSION_UTEST_CHANNELG_H
#define INCLUSION_UTEST_CHANNELG_H

#include "ocr.h" //ocrGuid_t, PRINTF
#include "../../tools/app_ocr_err_util.h"
#include "../../tools/app_ocr_debug_util.h"

#define RANK_COUNT 2
#define RANK_LEFT 0
#define RANK_RIGHT 1
#define NOTA_RANK 999

#define SLOTCNT_EndInit 2  //1 for the normal traffic, and 1 for labeled event
#define SLOTCNT_WorkStop 2  //1 for the normal traffic, and 1 for channel event

typedef struct utcLedger
{
    unsigned int rankid;
    ocrGuid_t whenDone;
    ocrGuid_t labeledGuids; //Two events needed to transport the "myChannel" event to the
                            // neighboring EDT.
    ocrGuid_t myChannel; //This is the channel event to be share with the other "rank",
                         //and which the current rank ocrEventSatisfy with a DBK
                         // in order to pass to the neighbor.  Sender channel?
    ocrGuid_t neighborChannel; //This is the channel event, received from the neighbor,
                               //and which I'll pass to my next EDT in order to get
                               //its DBK.  Receiver channel?
    ocrGuid_t temp_myLabelEvt;  //This is a temporary used for destroying the label EVT
                                //after use.
} utcLedger_t;

Err_t create_utcLedger(OA_DEBUG_ARGUMENT, utcLedger_t * io_ledger, ocrGuid_t * in_whenDone);
Err_t destroy_utcLedger(OA_DEBUG_ARGUMENT, utcLedger_t * io_ledger);
Err_t clear_utcLedger(OA_DEBUG_ARGUMENT, utcLedger_t * io_ledger);
void copy_utcLedger(OA_DEBUG_ARGUMENT, utcLedger_t * in_source, utcLedger_t * io_target);
void print_utcLedger(OA_DEBUG_ARGUMENT, utcLedger_t * in_ledger, const char * in_text);

unsigned int utcGet_next_slot(unsigned int in_rankid);
unsigned int utcGet_neighbor_rank(unsigned int in_rankid);
unsigned int utcGet_neighbor_slot(unsigned int in_rankid);

Err_t Userfcn_MainEdt(OA_DEBUG_ARGUMENT, ocrGuid_t * in_finalEdt, utcLedger_t * io_leftLedger,
                      utcLedger_t * io_rightLedger, utcLedger_t * io_refLedger);

Err_t Userfcn_FinalEdt(OA_DEBUG_ARGUMENT, utcLedger_t * io_refLedger, utcLedger_t * io_leftLedger,
                       utcLedger_t * io_rightLedger);

Err_t Userfcn_BeginInit(OA_DEBUG_ARGUMENT, utcLedger_t * in_ledger, utcLedger_t * o_ledger,
                        ocrGuid_t * io_destEDT, unsigned int in_destSlot);

Err_t Userfcn_EndInit(OA_DEBUG_ARGUMENT, utcLedger_t * in_ledger, utcLedger_t * o_ledger,
                      ocrEdtDep_t in_borLabelEVT);


Err_t work_start(OA_DEBUG_ARGUMENT, utcLedger_t * io_ledger,
                 ocrGuid_t * io_destEDT, unsigned int in_destSlot);

Err_t work_stop(OA_DEBUG_ARGUMENT, utcLedger_t * io_ledger, ocrGuid_t * io_finalEdt,
                ocrEdtDep_t in_borChanlEVT);


#endif //INCLUSION_UTEST_CHANNELG_H
