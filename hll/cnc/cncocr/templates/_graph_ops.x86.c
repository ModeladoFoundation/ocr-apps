{% extends "_graph_ops.c" %}
{% import "common_macros.inc.c" as util with context -%}

{% block arch_itemcoll_init scoped -%}
{% for i in g.concreteItems -%}
{% if i.key -%}
{{util.g_ctx_var()}}->_items.{{i.collName}} = calloc(CNC_TABLE_SIZE, sizeof(struct ItemCollEntry*));
{% else -%}
ocrEventCreate(&{{util.g_ctx_var()}}->_items.{{i.collName}}, OCR_EVENT_IDEM_T, TRUE);
{% endif -%}
{% endfor -%}
{% endblock arch_itemcoll_init %}

{% block arch_itemcoll_destroy -%}
{% for i in g.concreteItems -%}
{% if i.key -%}
FREE({{util.g_ctx_var()}}->_items.{{i.collName}});
{% else -%}
ocrEventDestroy({{util.g_ctx_var()}}->_items.{{i.collName}});
{% endif -%}
{% endfor -%}
{% endblock arch_itemcoll_destroy %}
