{% import "common_macros.inc.c" as util with context -%}
{{ util.auto_file_banner() }}

#include "{{g.name}}_internal.h"
#include <string.h>

{% for i in g.itemDeclarations.values() %}
/* {{i.collName}} */

void cncPut_{{i.collName}}({{i.type.ptrType}}_item, {{
        util.print_tag(i.key, typed=True) ~ util.g_ctx_param()}}) {
    {% if not i.isVirtual -%}
    {#/*****NON-VIRTUAL*****/-#}
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    if (!_cncItemGetReleasedFlag(_item)) {
        _cncItemToggleReleasedFlag(_item);
        ocrDbRelease(_handle);
    }
    // Re-putting an updated item
    else if (_item) {
        #ifdef CNC_DISTRIBUTED
        // XXX - it would be really nice if we didn't have to make a copy here
        void *_oldItem = _item;
        const size_t _bytes = _cncItemMeta(_item)->size;
        _item = cncItemAlloc(_bytes);
        _handle = _cncItemGuid(_item);
        memcpy(_item, _oldItem, _bytes);
        _cncItemToggleReleasedFlag(_item);
        // FIXME - this doesn't work when there are multiple puts in the same step
        // with the same item. Could fix this with OCR feature #678.
        ocrDbRelease(_handle);
        #else
        // Don't need to do anything in shared memory
        #endif /* CNC_DISTRIBUTED */
    }
    #ifdef CNC_AFFINITIES
    // FIXME - Need to copy to remote node if the affinity doesn't match
    const cncLocation_t _loc = {{g.itemDistFn(i.collName, util.g_ctx_var()~"->_affinityCount")}};
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif
    {{ util.log_msg("PUT", i.collName, i.key) }}
    {% if i.key -%}
    cncTag_t _tag[] = { {{i.key|join(", ")}} };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    _cncPut(_handle, _tag, _tagSize, _CNC_ITEM_COLL_HANDLE({{util.g_ctx_var()}}, {{i.collName}}, _loc));
    {%- else -%}
    _cncPutSingleton(_handle, _CNC_ITEM_COLL_HANDLE({{util.g_ctx_var()}}, {{i.collName}}, _loc));
    {%- endif %}
    {%- else -%}
    {% set targetColl = g.itemDeclarations[i.mapTarget] -%}
    {% if i.isInline -%}
    {#/*****INLINE VIRTUAL*****/-#}
    cncPut_{{i.mapTarget}}(_item, {{ util.print_tag(i.keyFunction) ~ util.g_ctx_var()}});
    {%- else -%}
    {#/*****EXTERN VIRTUAL******/-#}
    {{i.mapTarget}}ItemKey _key = {{i.functionName}}({{
        util.print_tag(i.key) }}{{util.g_ctx_var()}});
    cncPut_{{i.mapTarget}}(_item, {{
        util.print_tag(targetColl.key, prefix="_key.") ~ util.g_ctx_var()}});
    {%- endif %}
    {%- endif %}
}

void cncGet_{{i.collName}}({{ util.print_tag(i.key, typed=True) }}ocrGuid_t _destination, u32 _slot, ocrDbAccessMode_t _mode, {{util.g_ctx_param()}}) {
    {% if not i.isVirtual -%}
    {#/*****NON-VIRTUAL*****/-#}
    {{ util.log_msg("GET-DEP", i.collName, i.key) }}
    #ifdef CNC_AFFINITIES
    const cncLocation_t _loc = {{g.itemDistFn(i.collName, util.g_ctx_var()~"->_affinityCount")}};
    #else
    const cncLocation_t _loc = CNC_CURRENT_LOCATION; MAYBE_UNUSED(_loc);
    #endif
    {% if i.key -%}
    cncTag_t _tag[] = { {{i.key|join(", ")}} };
    const size_t _tagSize = sizeof(_tag)/sizeof(*_tag);
    return _cncGet(_tag, _tagSize, _destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE({{util.g_ctx_var()}}, {{i.collName}}, _loc));
    {%- else -%}
    return _cncGetSingleton(_destination, _slot, _mode, _CNC_ITEM_COLL_HANDLE({{util.g_ctx_var()}}, {{i.collName}}, _loc));
    {%- endif %}
    {%- else -%}
    {% set targetColl = g.itemDeclarations[i.mapTarget] -%}
    {% if i.isInline -%}
    {#/*****INLINE VIRTUAL*****/-#}
    cncGet_{{i.mapTarget}}({{
        util.print_tag(i.keyFunction)
        }}_destination, _slot, _mode, {{util.g_ctx_var()}});
    {%- else -%}
    {#/*****EXTERN VIRTUAL******/-#}
    {{i.mapTarget}}ItemKey _key = {{i.functionName}}({{
        util.print_tag(i.key) }}{{util.g_ctx_var()}});
    cncGet_{{i.mapTarget}}({{
        util.print_tag(targetColl.key, prefix="_key.")
        }}_destination, _slot, _mode, {{util.g_ctx_var()}});
    {%- endif %}
    {%- endif %}
}

{% endfor %}
