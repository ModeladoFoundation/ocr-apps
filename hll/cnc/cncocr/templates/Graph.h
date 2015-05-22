{% import "common_macros.inc.c" as util with context -%}
{{ util.auto_file_banner() }}

{% set defname = "_CNCOCR_" ~ g.name.upper() ~ "_H_" -%}
#ifndef {{defname}}
#define {{defname}}

#include "cncocr.h"
#include "{{g.name}}_defs.h"

/***************************\
 ******** CNC GRAPH ********
\***************************/
{#- /* TODO - Move this section into _external.h */ #}

typedef struct {{g.name}}Context {
    struct {
        ocrGuid_t self;
        ocrGuid_t finalizedEvent;
        ocrGuid_t quiescedEvent;
        ocrGuid_t doneEvent;
        ocrGuid_t awaitTag;
    } _guids;
    struct {
        {%- for i in g.concreteItems %}
        {{ "cncItemCollection_t" if i.key else "ocrGuid_t" }} {{i.collName}};
        {%- endfor %}
    } _items;
    struct {
        {%- for s in g.finalAndSteps %}
        ocrGuid_t {{s.collName}};
        {%- endfor %}
    } _steps;
{%- for line in g.ctxParams %}
    {{ line }}
{%- endfor %}
} {{util.g_ctx_t()}};
{# /* TODO - Use a GUID instead of a pointer to the context */ #}
{{util.g_ctx_t()}} *{{g.name}}_create(void);
void {{g.name}}_destroy({{util.g_ctx_param()}});

void {{g.name}}_launch({{g.name}}Args *args, {{util.g_ctx_param()}});
void {{g.name}}_await({{
        util.print_tag(g.finalizeFunction.tag, typed=True)
        }}{{util.g_ctx_param()}});

/**********************************\
 ******** ITEM KEY STRUCTS ********
\**********************************/

{% for name, i in g.itemDeclarations.items() if not i.isSingleton -%}
typedef struct { cncTag_t {{ i.key|join(", ") }}; } {{name}}ItemKey;
{% endfor %}
{% if g.externVms -%}
/****************************************\
 ******** ITEM MAPPING FUNCTIONS ********
\****************************************/

{% for i in g.externVms -%}
{{i.mapTarget}}ItemKey {{i.functionName}}({{
  util.print_tag(i.key, typed=True)
  }}{{util.g_ctx_param()}});
{% endfor %}
{% endif -%}
/*****************************\
 ******** ITEM CREATE ********
\*****************************/
{% for name, i in g.itemDeclarations.items() %}
{{i.type.ptrType}}cncItemCreateSized_{{name}}(size_t size);
{# /* TODO - ADD NAMESPACE PREFIX DEFINE THING */ -#}
static inline {{i.type.ptrType}}cncItemCreate_{{name}}(void) {
    return cncItemCreateSized_{{name}}(sizeof({{i.type.baseType}}));
}
static inline {{i.type.ptrType}}cncItemCreateVector_{{name}}(size_t count) {
    return cncItemCreateSized_{{name}}(sizeof({{i.type.baseType}}) * count);
}
{% endfor %}
static inline void cncItemDestroy(void *item) {
    cncFree(item);
}

/**************************\
 ******** ITEM PUT ********
\**************************/
{% for name, i in g.itemDeclarations.items() %}
{# /* TODO - ADD NAMESPACE PREFIX DEFINE THING */ -#}
// {{i.collName}}

void cncPutChecked_{{name}}({{i.type.ptrType}}_item, {{
        util.print_tag(i.key, typed=True)
        }}bool checkSingleAssignment, {{util.g_ctx_param()}});

static inline void cncPut_{{name}}({{i.type.ptrType}}_item, {{
        util.print_tag(i.key, typed=True)
        }}{{util.g_ctx_param()}}) {
    cncPutChecked_{{name}}(_item, {{ util.print_tag(i.key) }}true, {{util.g_ctx_var()}});
}
{% endfor %}
/************************************\
 ******** STEP PRESCRIPTIONS ********
\************************************/

{% for stepfun in g.finalAndSteps -%}
void cncPrescribe_{{stepfun.collName}}({{
        util.print_tag(stepfun.tag, typed=True) }}{{util.g_ctx_param()}});
{% endfor %}
#endif /*{{defname}}*/
