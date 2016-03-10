#define META_IMPLEMENTATION
#include "meta.h"
#include <cassert>
#include <vector>
#include <memory.h>
#include <iterator>

#define META_KEY_MAX_BYTES 64
#define META_MAX_RECORDS 64
#define META_MAX_TEMPLATES 64

struct meta_task_state
{
	ocrEdt_t func;
};
#define META_PREFIX_SIZE ((sizeof(meta_task_state)+sizeof(u64)-1)/sizeof(u64))

struct meta_global_state
{
	struct metadata_item
	{
		ocrGuid_t object;
		ocrGuid_t key;
		char data[META_KEY_MAX_BYTES];
	};
	struct template_data
	{
		ocrGuid_t guid;
		ocrEdt_t func;
		u32 paramc;
		u32 depc;
	};
	meta_global_state(ocrGuid_t state_guid) : state_guid(state_guid), item_count(0), template_count(0) {}
	void add_data(ocrGuid_t object, ocrGuid_t key, void* data, u64 size)
	{
		items[item_count].object = object;
		items[item_count].key = key;
		::memcpy(&items[item_count].data,data,(std::size_t)size);
		++item_count;
		assert(item_count < META_MAX_RECORDS);
	}
	void* get_data(ocrGuid_t object, ocrGuid_t key)
	{
		for (std::size_t i = 0; i < item_count; ++i)
		{
			if (items[i].object == object && items[i].key == key) return &items[i].data;
		}
		assert(!"metadata not found");
		return 0;
	}
	void add_template(ocrGuid_t template_guid, ocrEdt_t func, u32 paramc, u32 depc)
	{
		templates[template_count].guid = template_guid;
		templates[template_count].func = func;
		templates[template_count].paramc = paramc;
		templates[template_count].depc = depc;
		++template_count;
		assert(template_count < META_MAX_TEMPLATES);
	}
	template_data* get_template(ocrGuid_t template_guid)
	{
		for (std::size_t i = 0; i < template_count; ++i)
		{
			if (templates[i].guid == template_guid) return &templates[i];
		}
		assert(!"template not found");
		return 0;
	}
	ocrGuid_t state_guid;
	ocrGuid_t proxy_task_template;
	metadata_item items[META_MAX_RECORDS];
	u64 item_count;
	template_data templates[META_MAX_TEMPLATES];
	u64 template_count;
};

meta_global_state* meta_global()
{
	return (meta_global_state*)ocrElsUserGet(0);
}

void set_meta_global(meta_global_state* mgs)
{
	ocrElsUserSet(0, (ocrGuid_t)mgs);
}

meta_task_state* meta_local()
{
	return (meta_task_state*)ocrElsUserGet(1);
}

void set_meta_local(meta_task_state* mts)
{
	ocrElsUserSet(1, (ocrGuid_t)mts);
}

ocrGuid_t meta_ocrElsUserGet(u8 offset)
{
	return ocrElsUserGet(offset + 2);
}

void meta_ocrElsUserSet(u8 offset, ocrGuid_t data)
{
	ocrElsUserSet(offset + 2, data);
}

ocrGuid_t metaProxyEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	set_meta_global(static_cast<meta_global_state*>(depv[depc - 1].ptr));
	meta_task_state* prefix = reinterpret_cast<meta_task_state*>(paramv);
	ocrEdt_t fnc = prefix->func;
	set_meta_local(prefix);
	return fnc(paramc - META_PREFIX_SIZE, paramv + META_PREFIX_SIZE, depc - 1, depv);
}

u8 meta_ocrEdtTemplateCreate_internal(ocrGuid_t *guid, ocrEdt_t funcPtr, u32 paramc, u32 depc, const char* name)
{
	u32 orig_paramc = paramc;
	u32 orig_depc = depc;
	//save the declared values, but use the extended values
	if (paramc != EDT_PARAM_UNK) depc += META_PREFIX_SIZE;
	if (depc != EDT_PARAM_UNK) depc += 1;
	ocrEdtTemplateCreate_internal(guid, metaProxyEdt, paramc, depc, (char*)name);
	meta_global()->add_template(*guid, funcPtr, orig_paramc, orig_depc);
	return 0;
}
u8 meta_ocrEdtTemplateDestroy(ocrGuid_t guid)
{
	ocrEdtTemplateDestroy(guid);
	return 0;
}
u8 meta_ocrEdtCreate(ocrGuid_t *guid, ocrGuid_t templateGuid, u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv, u16 properties, ocrGuid_t affinity, ocrGuid_t *outputEvent)
{
	meta_global_state::template_data* td = meta_global()->get_template(templateGuid);
	meta_task_state prefix = *meta_local();
	prefix.func = td->func;
	u32 client_paramc = paramc;
	u32 client_depc = depc;
	u32 effective_paramc = paramc;
	u32 effective_depc = depc;
	if (client_paramc == EDT_PARAM_DEF)
	{
		assert(td->paramc != EDT_PARAM_UNK);
		client_paramc = td->paramc;

	}
	else effective_paramc = client_paramc + META_PREFIX_SIZE;
	if (client_depc == EDT_PARAM_DEF)
	{
		assert(td->depc != EDT_PARAM_UNK);
		client_depc = td->depc;
	}
	else effective_depc = client_depc + 1;
	std::vector<u64> params(reinterpret_cast<u64*>(&prefix), reinterpret_cast<u64*>(&prefix) + META_PREFIX_SIZE);
	if (paramv) std::copy(paramv, paramv + client_paramc, std::back_inserter(params));
	std::vector<ocrGuid_t> deps;
	if (depv)
	{
		std::copy(depv, depv + client_depc, std::back_inserter(deps));
	}
	else
	{
		std::vector<ocrGuid_t> tmp(depc, UNINITIALIZED_GUID);
		deps.swap(tmp);
	}
	deps.push_back(UNINITIALIZED_GUID);
	ocrEdtCreate(guid, templateGuid, effective_paramc, &params.front(), effective_depc, &deps.front(), properties, affinity, outputEvent);
	ocrAddDependence(meta_global()->state_guid, *guid, depc, DB_MODE_RW);
	return 0;

}

meta_task_state meta_main_edt_state;

extern "C" ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	meta_global_state* state;
	ocrGuid_t state_guid;
	ocrDbCreate(&state_guid, (void**)&state, sizeof(meta_global_state), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
	new (state)meta_global_state(state_guid);
	ocrEdtTemplateCreate(&state->proxy_task_template, metaProxyEdt, EDT_PARAM_UNK, EDT_PARAM_UNK);
	set_meta_global(state);
	set_meta_local(&meta_main_edt_state);
	return meta_mainEdt(paramc, paramv, depc, depv);
}

void meta_ocrShutdown()
{
	ocrEdtTemplateDestroy(meta_global()->proxy_task_template);
	ocrDbDestroy(meta_global()->state_guid);
	ocrShutdown();
}

u8 metaCreateKey(ocrGuid_t *guid, ocrGuidUserKind kind, u64 max_data_size)
{
	assert(max_data_size < META_KEY_MAX_BYTES);
	return ocrEventCreate(guid, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
}

u8 metaAssignData(ocrGuid_t object, ocrGuid_t key, void* data, u64 actual_data_size)
{
	meta_global()->add_data(object, key, data, actual_data_size);
	return 0;
}

u8 metaGetData(ocrGuid_t object, ocrGuid_t key, void** data)
{
	*data = meta_global()->get_data(object, key);
	return 0;
}
