/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#include "cncocr_internal.h"

#define SIMPLE_DBCREATE(guid, ptr, sz) ocrDbCreate(guid, ptr, sz, DB_PROP_NONE, NULL_HINT, NO_ALLOC)

static ocrGuid_t _destroyEventEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocrGuid_t eventGuid = *(ocrGuid_t*)paramv;
    ocrEventDestroy(eventGuid);
    ocrGuid_t dbGuid = depv[0].guid;
    if (!ocrGuidIsNull(dbGuid)) {
        ocrDbDestroy(dbGuid);
    }
    return NULL_GUID;
}

u8 ocrEventDestroyDeep(ocrGuid_t event) {
    ocrGuid_t template, edt;
    // Make sure it's already satisfied (idempotent)
    u8 res = ocrEventSatisfy(event, NULL_GUID);
    const u32 paramc = sizeof(ocrGuid_t) / sizeof(u64);
    res = res ||ocrEdtTemplateCreate(&template, _destroyEventEdt, paramc, 1);
    res = res || ocrEdtCreate(&edt, template,
            /*paramc=*/EDT_PARAM_DEF, /*paramv=*/(u64*)&event,
            /*depc=*/EDT_PARAM_DEF, /*depv=*/&event,
            /*properties=*/EDT_PROP_NONE,
            /*hint=*/NULL_HINT, /*outEvent=*/NULL);
    res = res || ocrEdtTemplateDestroy(template);
    return res;
}

/* The structure to hold an item in the item collection */
typedef struct _cncItemCollEntry {
    ocrGuid_t event; /* The event representing the data item. Data will be put through the event when it is satisfied */
    struct _cncItemCollEntry * volatile nxt; /* The next bucket in the hashtable */
    char creator; /* Who created this entry (could be from a Put or a Get)*/
    cncTag_t tag[]; /* Tags are byte arrays, with a known length for each item collection */
} _cncItemCollectionEntry;

/* Get an entry from the item collection, or create and insert one (atomically) if it doesn't exist.
 * The creator parameter PUTTER/GETTER role ensures that multiple puts are not allowed.
 */
static bool _allocateEntryIfAbsent(
        _cncItemCollectionEntry * volatile * hashmap, cncTag_t *tag,
        int length, char creator, _cncItemCollectionEntry **entryOut) {
    const int tagByteCount = length*sizeof(*tag);
    int index = (_cncTagHash(tag, length)) % CNC_TABLE_SIZE;
    _cncItemCollectionEntry * volatile current = hashmap[index];
    _cncItemCollectionEntry * volatile * currentLocation = &hashmap[index];
    _cncItemCollectionEntry * volatile head = current;
    _cncItemCollectionEntry * volatile tail = NULL;

    _cncItemCollectionEntry * entry = NULL;

    while (1) {
        /* traverse the buckets in the table to get to the last one */
        while (current != tail) {
            if (_cncTagEquals(current->tag, tag, length)) {
                /* deallocate the entry we eagerly allocated in a previous iteration of the outer while(1) loop */
                if (entry != NULL){
                    ocrEventDestroy(entry->event);
                    free(entry);
                }

                /* just return the table entry if it already has the tag */
                *entryOut = current;
                return false;
            }
            current = current->nxt;
        }

        /* allocate a new entry if this is the first time we are going to try and insert a new entry to the end of the bucket list */
        if (entry == NULL) {
            entry = malloc(sizeof(_cncItemCollectionEntry)+tagByteCount);
            entry->creator = creator;
            memcpy(entry->tag, tag, tagByteCount);
            ocrEventCreate(&(entry->event), OCR_EVENT_IDEM_T, true);
        }
        entry->nxt=head;

        /* try to insert the new entry into the _first_ position in a bucket of the table */
        if (__sync_bool_compare_and_swap(currentLocation, head, entry)) {
            *entryOut = entry;
            return true;
        }

        /* CAS failed, which means that someone else inserted the new entry into the table while we were trying to do so, we need to try again */
        current = hashmap[index]; //do not update tail anymore if deletes are inserted.
        tail = head;
        head = current;
    }

    ASSERT(!"Unreachable"); /* we should never get here */
    return false;
}

static inline ocrGuid_t _cncItemCollUpdateLocal(_cncItemCollectionEntry **coll, cncTag_t *tag, u32 tagLength, u8 role,
        ocrGuid_t input, u32 slot, ocrDbAccessMode_t mode) {
    // local hashtable update
    _cncItemCollectionEntry *entry;
    bool wasUpdated = _allocateEntryIfAbsent(coll, tag, tagLength, role, &entry);
    if (role == _CNC_PUTTER_ROLE) { // put input into collection
        ocrEventSatisfy(entry->event, input);
    }
    else { // get placeholder and pass to input
        ocrAddDependence(entry->event, input, slot, mode);
    }
    // Notify caller if the entry was already there
    return wasUpdated ? entry->event : NULL_GUID;
}

cncItemCollection_t _cncItemCollectionCreate(void) {
    return calloc(CNC_TABLE_SIZE, sizeof(struct _cncItemCollEntry*));
}

void _cncItemCollectionDestroy(cncItemCollection_t coll) {
    // FIXME - need to do a deep traversal to really destroy the collection
    free(coll);
}

void _cncItemCollUpdate(cncItemCollHandle_t handle, cncTag_t *tag, u32 tagLength, u8 role,
        ocrGuid_t input, u32 slot, ocrDbAccessMode_t mode) {
    _cncItemCollUpdateLocal(handle.coll, tag, tagLength, role, input, slot, mode);
}


cncItemSingleton_t _cncItemCollectionSingletonCreate(void) {
    cncItemSingleton_t coll;
    // FIXME - Create with new event affinity API in OCR v1.2.0
    ocrEventCreate(&coll.only, OCR_EVENT_IDEM_T, EVT_PROP_TAKES_ARG);
    return coll;
}

void _cncItemCollectionSingletonDestroy(cncItemSingleton_t coll) {
    ocrEventDestroyDeep(coll.only);
}



