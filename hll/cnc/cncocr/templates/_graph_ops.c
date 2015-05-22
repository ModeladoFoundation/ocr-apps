{% import "common_macros.inc.c" as util with context -%}
{{ util.auto_file_banner() }}

#include "{{g.name}}_internal.h"

#ifdef CNC_DEBUG_LOG
#ifndef CNCOCR_x86
#error "Debug logging mode only supported on x86 targets"
#endif
#include <pthread.h>
pthread_mutex_t _cncDebugMutex = PTHREAD_MUTEX_INITIALIZER;
#endif /* CNC_DEBUG_LOG */

{{util.g_ctx_t()}} *{{g.name}}_create() {
#ifdef CNC_DEBUG_LOG
    // init debug logger (only once)
    if (!cncDebugLog) {
        cncDebugLog = fopen(CNC_DEBUG_LOG, "w");
    }
#endif /* CNC_DEBUG_LOG */
    // allocate the context datablock
    ocrGuid_t contextGuid;
    {{util.g_ctx_param()}};
    SIMPLE_DBCREATE(&contextGuid, (void**)&{{util.g_ctx_var()}}, sizeof(*{{util.g_ctx_var()}}));
    // store a copy of its guid inside
    {{util.g_ctx_var()}}->_guids.self = contextGuid;
    // initialize graph events
    // TODO - these events probably shouldn't be marked as carrying data
    ocrEventCreate(&{{util.g_ctx_var()}}->_guids.finalizedEvent, OCR_EVENT_STICKY_T, TRUE);
    ocrEventCreate(&{{util.g_ctx_var()}}->_guids.quiescedEvent, OCR_EVENT_STICKY_T, FALSE);
    ocrEventCreate(&{{util.g_ctx_var()}}->_guids.doneEvent, OCR_EVENT_STICKY_T, TRUE);
    ocrEventCreate(&{{util.g_ctx_var()}}->_guids.awaitTag, OCR_EVENT_ONCE_T, TRUE);
    // initialize item collections
    {% call util.render_indented(1) -%}
{% block arch_itemcoll_init scoped -%}
s32 i;
ocrGuid_t *itemTable;
{% for i in g.concreteItems -%}
{% if i.key -%}
SIMPLE_DBCREATE(&{{util.g_ctx_var()}}->_items.{{i.collName}}, (void**)&itemTable, sizeof(ocrGuid_t) * CNC_TABLE_SIZE);
for (i=0; i<CNC_TABLE_SIZE; i++) {
    ocrGuid_t *_ptr;
    // Add one level of indirection to help with contention
    SIMPLE_DBCREATE(&itemTable[i], (void**)&_ptr, sizeof(ocrGuid_t));
    *_ptr = NULL_GUID;
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(itemTable[i]);
}
// FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
// ocrDbRelease({{util.g_ctx_var()}}->_items.{{i.collName}});
{% else -%}
ocrEventCreate(&{{util.g_ctx_var()}}->_items.{{i.collName}}, OCR_EVENT_IDEM_T, TRUE);
{% endif -%}
{% endfor -%}
{% endblock arch_itemcoll_init -%}
    {% endcall %}
    // initialize step collections
    {% for s in g.finalAndSteps -%}
    ocrEdtTemplateCreate(&{{util.g_ctx_var()}}->_steps.{{s.collName}},
            _{{g.name}}_cncStep_{{s.collName}}, EDT_PARAM_UNK, EDT_PARAM_UNK);
    {% endfor -%}
    return {{util.g_ctx_var()}};
}

void {{g.name}}_destroy({{util.g_ctx_param()}}) {
    ocrEventDestroy({{util.g_ctx_var()}}->_guids.finalizedEvent);
    ocrEventDestroy({{util.g_ctx_var()}}->_guids.quiescedEvent);
    ocrEventDestroy({{util.g_ctx_var()}}->_guids.doneEvent);
    // destroy item collections
    // XXX - need to do a deep free by traversing the table
    {% call util.render_indented(1) -%}
{% block arch_itemcoll_destroy -%}
{% for i in g.concereteItems -%}
{% if i.key -%}
ocrDbDestroy({{util.g_ctx_var()}}->_items.{{i.collName}});
{% else -%}
ocrEventDestroy({{util.g_ctx_var()}}->_items.{{i.collName}});
{% endif -%}
{% endfor -%}
{% endblock arch_itemcoll_destroy -%}
    {% endcall -%}
    // destroy step collections
    {% for s in g.finalAndSteps -%}
    ocrEdtTemplateDestroy({{util.g_ctx_var()}}->_steps.{{s.collName}});
    {% endfor -%}
    ocrDbDestroy({{util.g_ctx_var()}}->_guids.self);
}

static ocrGuid_t _emptyEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    return NULL_GUID;
}

/* EDT runs when all compute steps are done AND graph is finalized (graph is DONE) */
static ocrGuid_t _graphFinishedEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    ocrGuid_t finalizerResult = depv[1].guid;
    return finalizerResult;
}

/* EDT runs when all compute steps are done (graph is quiesced) */
static ocrGuid_t _stepsFinishedEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    {{util.g_args_param()}} = depv[0].ptr;
    {{util.g_ctx_param()}} = depv[1].ptr;
    // XXX - just do finalize from within the finish EDT
    // The graph isn't done until the finalizer runs as well,
    // so we need to make a dummy EDT depending on the
    // finalizer's output event.
    ocrGuid_t emptyEdtGuid, templGuid;
    ocrEdtTemplateCreate(&templGuid, _emptyEdt, 0, 1);
    ocrEdtCreate(&emptyEdtGuid, templGuid,
        /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
        /*depc=*/EDT_PARAM_DEF, /*depv=*/&{{util.g_ctx_var()}}->_guids.finalizedEvent,
        /*properties=*/EDT_PROP_NONE,
        /*affinity=*/NULL_GUID, /*outEvent=*/NULL);
    // XXX - destroying this template caused crash on FSim
    //ocrEdtTemplateDestroy(templGuid);
    // Start graph execution
    {{ util.step_enter() }}
    {{util.qualified_step_name(g.initFunction)}}({{util.g_args_var()}}, {{util.g_ctx_var()}});
    {{ util.step_exit() }}
    if ({{util.g_args_var()}}) ocrDbDestroy(depv[0].guid);
    return NULL_GUID;
}

static ocrGuid_t _finalizerEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    {{util.g_ctx_param()}} = depv[0].ptr;
    cncTag_t *tag = depv[1].ptr; MAYBE_UNUSED(tag);
    cncPrescribe_{{g.finalizeFunction.collName}}(
        {%- for x in g.finalizeFunction.tag %}tag[{{loop.index0}}], {% endfor -%}
        {{util.g_ctx_var()}});
    // TODO - I probably need to free this (the tag) sometime
    // XXX - for some reason this causes a segfault?
    //ocrDbDestroy(depv[1].guid);
    return NULL_GUID;
}

void {{g.name}}_launch({{util.g_args_param()}}, {{util.g_ctx_param()}}) {
    {{util.g_args_param()}}Copy;
    ocrGuid_t graphEdtGuid, finalEdtGuid, doneEdtGuid, edtTemplateGuid, outEventGuid, argsDbGuid;
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease({{util.g_ctx_var()}}->_guids.self);
    // copy the args struct into a data block
    // TODO - I probably need to free this sometime
    if (sizeof(*{{util.g_args_var()}}) > 0) {
        SIMPLE_DBCREATE(&argsDbGuid, (void**)&argsCopy, sizeof(*{{util.g_args_var()}}));
        *argsCopy = *{{util.g_args_var()}};
        // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
        // ocrDbRelease(argsDbGuid);
    }
    // Don't need to copy empty args structs
    else {
        argsDbGuid = NULL_GUID;
    }
    // create a finish EDT for the CnC graph
    {
        ocrEdtTemplateCreate(&edtTemplateGuid, _stepsFinishedEdt, 0, 2);
        ocrEdtCreate(&graphEdtGuid, edtTemplateGuid,
                /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
                /*depc=*/EDT_PARAM_DEF, /*depv=*/NULL,
                /*properties=*/EDT_PROP_FINISH,
                /*affinity=*/NULL_GUID, /*outEvent=*/&outEventGuid);
        ocrEdtTemplateDestroy(edtTemplateGuid);
        // hook the graph's quiescedEvent into the graph's output event
        ocrAddDependence(outEventGuid, {{util.g_ctx_var()}}->_guids.quiescedEvent, 0, DB_DEFAULT_MODE);
    }
    // set up the finalizer
    {
        ocrEdtTemplateCreate(&edtTemplateGuid, _finalizerEdt, 0, 2);
        ocrGuid_t deps[] = { {{util.g_ctx_var()}}->_guids.self, {{util.g_ctx_var()}}->_guids.awaitTag };
        ocrEdtCreate(&finalEdtGuid, edtTemplateGuid,
            /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
            /*depc=*/EDT_PARAM_DEF, /*depv=*/deps,
            /*properties=*/EDT_PROP_FINISH,
            /*affinity=*/NULL_GUID, /*outEvent=*/&outEventGuid);
        ocrEdtTemplateDestroy(edtTemplateGuid);
        // hook the graph's finalizedEvent into the finalizer's output event
        ocrAddDependence(outEventGuid, {{util.g_ctx_var()}}->_guids.finalizedEvent, 0, DB_DEFAULT_MODE);
    }
    // set up the EDT that controls the graph's doneEvent
    {
        ocrEdtTemplateCreate(&edtTemplateGuid, _graphFinishedEdt, 0, 2);
        ocrGuid_t deps[] = { {{util.g_ctx_var()}}->_guids.quiescedEvent, {{util.g_ctx_var()}}->_guids.finalizedEvent };
        ocrEdtCreate(&doneEdtGuid, edtTemplateGuid,
            /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
            /*depc=*/EDT_PARAM_DEF, /*depv=*/deps,
            /*properties=*/EDT_PROP_NONE,
            /*affinity=*/NULL_GUID, /*outEvent=*/&outEventGuid);
        ocrEdtTemplateDestroy(edtTemplateGuid);
        ocrAddDependence(outEventGuid, {{util.g_ctx_var()}}->_guids.doneEvent, 0, DB_DEFAULT_MODE);
    }
    // start the graph execution
    ocrAddDependence(argsDbGuid, graphEdtGuid, 0, DB_DEFAULT_MODE);
    ocrAddDependence({{util.g_ctx_var()}}->_guids.self, graphEdtGuid, 1, DB_DEFAULT_MODE);
}

void {{g.name}}_await({{
        util.print_tag(g.finalizeFunction.tag, typed=True)
        }}{{util.g_ctx_param()}}) {
    // Can't launch the finalizer EDT from within the finish EDT,
    // so we copy the tag information into a DB and do it indirectly.
    {% if g.finalizeFunction.tag -%}
    cncTag_t *_tagPtr;
    ocrGuid_t _tagGuid;
    int _i = 0;
    SIMPLE_DBCREATE(&_tagGuid, (void**)&_tagPtr, sizeof(cncTag_t) * {{ g.finalizeFunction.tag|count}});
    {% for x in g.finalizeFunction.tag -%}
    _tagPtr[_i++] = {{x}};
    {% endfor -%}
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_tagGuid);
    {% else -%}
    ocrGuid_t _tagGuid = NULL_GUID;
    {% endif -%}
    ocrEventSatisfy({{util.g_ctx_var()}}->_guids.awaitTag, _tagGuid);
}

/* define NO_CNC_MAIN if you want to use mainEdt as the entry point instead */
#ifndef NO_CNC_MAIN

extern int cncMain(int argc, char *argv[]);

#pragma weak mainEdt
ocrGuid_t mainEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
    // Unpack argc and argv (passed thru from mainEdt)
    int i, argc = OCR_MAIN_ARGC;
    char **argv = cncMalloc(sizeof(char*)*argc);
    for (i=0; i<argc; i++) argv[i] = OCR_MAIN_ARGV(i);
    // Run user's cncEnvIn function
    cncMain(argc, argv);
    cncFree(argv);
    return NULL_GUID;
}

#endif /* NO_CNC_MAIN */

