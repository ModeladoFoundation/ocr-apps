{% import "common_macros.inc.c" as util with context -%}
{{ util.auto_file_banner() }}

#include "{{g.name}}_internal.h"

{% for i in g.itemDeclarations.values() %}
/* {{i.collName}} */

{{ i.type.ptrType }}cncItemCreateSized_{{i.collName}}(size_t size) {
    return cncMalloc(size);
}

void cncPutChecked_{{i.collName}}({{i.type.ptrType}}_item, {{
        util.print_tag(i.key, typed=True)
        }}bool _checkSingleAssignment, {{util.g_ctx_param()}}) {
    {% if not i.isVirtual -%}
    {#/*****NON-VIRTUAL*****/-#}
    ocrGuid_t _handle = _cncItemGuid(_item);
    // MUST release first to conform with OCR memory model
    // otherwise the item data might not be visible to other EDTs
    // FIXME - Re-enable ocrDbRelease after bug #504 (redmine) is fixed
    // ocrDbRelease(_handle);
    {{ util.log_msg("PUT", i.collName, i.key) }}
    {% if i.key -%}
    cncTag_t _tag[] = { {{i.key|join(", ")}} };
    _cncPut(_handle, (unsigned char*)_tag, sizeof(_tag), {{util.g_ctx_var()}}->_items.{{i.collName}}, _checkSingleAssignment);
    {%- else -%}
    _cncPutSingleton(_handle, {{util.g_ctx_var()}}->_items.{{i.collName}}, _checkSingleAssignment);
    {%- endif %}
    {%- else -%}
    {% set targetColl = g.itemDeclarations[i.mapTarget] -%}
    {% if i.isInline -%}
    {#/*****INLINE VIRTUAL*****/-#}
    cncPutChecked_{{i.mapTarget}}(_item, {{
        util.print_tag(i.keyFunction)
        }}_checkSingleAssignment, {{util.g_ctx_var()}});
    {%- else -%}
    {#/*****EXTERN VIRTUAL******/-#}
    {{i.mapTarget}}ItemKey _key = {{i.functionName}}({{
        util.print_tag(i.key) }}{{util.g_ctx_var()}});
    cncPutChecked_{{i.mapTarget}}(_item, {{
        util.print_tag(targetColl.key, prefix="_key.")
        }}_checkSingleAssignment, {{util.g_ctx_var()}});
    {%- endif %}
    {%- endif %}
}

void cncGet_{{i.collName}}({{ util.print_tag(i.key, typed=True) }}ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, {{util.g_ctx_param()}}) {
    {% if not i.isVirtual -%}
    {#/*****NON-VIRTUAL*****/-#}
    {{ util.log_msg("GET-DEP", i.collName, i.key) }}
    {% if i.key -%}
    cncTag_t tag[] = { {{i.key|join(", ")}} };
    return _cncGet((unsigned char*)tag, sizeof(tag), destination, slot, mode, {{util.g_ctx_var()}}->_items.{{i.collName}});
    {%- else -%}
    return _cncGetSingleton(destination, slot, mode, {{util.g_ctx_var()}}->_items.{{i.collName}});
    {%- endif %}
    {%- else -%}
    {% set targetColl = g.itemDeclarations[i.mapTarget] -%}
    {% if i.isInline -%}
    {#/*****INLINE VIRTUAL*****/-#}
    cncGet_{{i.mapTarget}}({{
        util.print_tag(i.keyFunction)
        }}destination, slot, mode, {{util.g_ctx_var()}});
    {%- else -%}
    {#/*****EXTERN VIRTUAL******/-#}
    {{i.mapTarget}}ItemKey _key = {{i.functionName}}({{
        util.print_tag(i.key) }}{{util.g_ctx_var()}});
    cncGet_{{i.mapTarget}}({{
        util.print_tag(targetColl.key, prefix="_key.")
        }}destination, slot, mode, {{util.g_ctx_var()}});
    {%- endif %}
    {%- endif %}
}

{% endfor %}
