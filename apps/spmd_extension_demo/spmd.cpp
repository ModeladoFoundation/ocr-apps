#include <cassert>
#include <vector>
#include <iterator>
#include <memory.h>
#include "meta.h"
#define SPMD_IMPLEMENTATION
#include "spmd.h"

//#define CHANNELS_IN_RANGE


struct spmd_task_state
{
	spmd_task_state() : rank(-1), func(0) {}
	u64 rank;
	ocrEdt_t func;
};

struct spmd_global_state
{
	spmd_global_state(ocrGuid_t state_guid) : state_guid(state_guid) {}
	ocrGuid_t state_guid;
	u64 size;
	ocrGuid_t proxy_task_template;
	ocrGuid_t metadata_key;
	ocrGuid_t trigger_template;
#ifdef CHANNELS_IN_RANGE
	ocrGuid_t message_block_range;
#else
#define SPMD_MAX_RANK 2
	ocrGuid_t channels[SPMD_MAX_RANK*SPMD_MAX_RANK];
#endif
};

struct spmd_template_data
{
	ocrEdt_t func;
	u32 paramc;
	u32 depc;
};

#define SPMD_PREFIX_SIZE ((sizeof(spmd_task_state)+sizeof(u64)-1)/sizeof(u64))

spmd_global_state* spmd_global()
{
	return (spmd_global_state*)ocrElsUserGet(0);
}

void set_spmd_global(spmd_global_state* mgs)
{
	ocrElsUserSet(0, (ocrGuid_t)mgs);
}

spmd_task_state* spmd_local()
{
	return (spmd_task_state*)ocrElsUserGet(1);
}

void set_spmd_local(spmd_task_state* mts)
{
	ocrElsUserSet(1, (ocrGuid_t)mts);
}

ocrGuid_t spmd_ocrElsUserGet(u8 offset)
{
	return ocrElsUserGet(offset + 2);
}

void spmd_ocrElsUserSet(u8 offset, ocrGuid_t data)
{
	ocrElsUserSet(offset + 2, data);
}

inline u64 spmdMessageBoxIndex(u64 from, u64 to)
{
	assert(from < spmd_global()->size);
	assert(to < spmd_global()->size);
	return from * spmd_global()->size + to;
}

ocrGuid_t spmdProxyEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	set_spmd_global(static_cast<spmd_global_state*>(depv[depc-1].ptr));
	spmd_task_state* prefix = reinterpret_cast<spmd_task_state*>(paramv);
	ocrEdt_t fnc = prefix->func;
	set_spmd_local(prefix);
	return fnc(paramc - SPMD_PREFIX_SIZE, paramv + SPMD_PREFIX_SIZE, depc - 1, depv);
}

u64 spmdMyRank()
{
	return spmd_local()->rank;
}

u64 spmdSize()
{
	return spmd_global()->size;
}

u8 spmd_ocrEdtTemplateCreate_internal(ocrGuid_t *guid, ocrEdt_t funcPtr, u32 paramc, u32 depc, const char* name)
{
	spmd_template_data td;
	td.func = funcPtr;
	td.paramc = paramc;
	td.depc = depc;
	//save the declared values, but use the extended values
	if (paramc != EDT_PARAM_UNK) depc += SPMD_PREFIX_SIZE;
	if (depc != EDT_PARAM_UNK) depc += 1;
	ocrEdtTemplateCreate_internal(guid, spmdProxyEdt, paramc, depc, (char*)name);
	metaAssignData(*guid, spmd_global()->metadata_key, &td, sizeof(spmd_template_data));
	return 0;
}
u8 spmd_ocrEdtTemplateDestroy(ocrGuid_t guid)
{
	ocrEdtTemplateDestroy(guid);
	return 0;
}
u8 spmd_ocrEdtCreate(ocrGuid_t * guid, ocrGuid_t templateGuid, u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv, u16 properties, ocrGuid_t affinity, ocrGuid_t *outputEvent)
{
	return spmdEdtCreateWithRank(guid, templateGuid, paramc, paramv, depc, depv, properties, affinity, outputEvent, spmdMyRank());
}
u8 spmdEdtCreateWithRank(ocrGuid_t * guid, ocrGuid_t templateGuid, u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv, u16 properties, ocrGuid_t affinity, ocrGuid_t *outputEvent, u64 rank)
{
	spmd_template_data* td;
	metaGetData(templateGuid, spmd_global()->metadata_key, (void**)&td);
	spmd_task_state prefix=*spmd_local();
	prefix.rank = rank;
	prefix.func = td->func;
	u32 client_paramc = paramc;
	u32 client_depc = depc;
	u32 effective_paramc = paramc;
	u32 effective_depc = depc;
	if (client_paramc == EDT_PARAM_DEF)
	{
		assert(td->paramc != EDT_PARAM_UNK);
		client_paramc = td->paramc;

	} else effective_paramc = client_paramc + SPMD_PREFIX_SIZE;
	if (client_depc == EDT_PARAM_DEF)
	{
		assert(td->depc != EDT_PARAM_UNK);
		client_depc = td->depc;
	} else effective_depc = client_depc + 1;
	std::vector<u64> params(reinterpret_cast<u64*>(&prefix), reinterpret_cast<u64*>(&prefix) + SPMD_PREFIX_SIZE);
	if (paramv) std::copy(paramv, paramv + client_paramc,std::back_inserter(params));
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
	ocrAddDependence(spmd_global()->state_guid, *guid, depc, DB_MODE_RW);
	return 0;
}

void spmd_ocrShutdown()
{
	ocrEdtTemplateDestroy(spmd_global()->proxy_task_template);
	ocrEdtTemplateDestroy(spmd_global()->trigger_template);
#ifdef CHANNELS_IN_RANGE
	ocrGuidRangeCreate(&(spmd_global()->message_block_range), spmd_global()->size * spmd_global()->size, GUID_USER_EVENT_CHANNEL);
#else
	for (std::size_t i = 0; i < spmdSize(); ++i)
	{
		for (std::size_t j = 0; j < spmdSize(); ++j)
		{
			ocrEventDestroy(spmd_global()->channels[spmdMessageBoxIndex(i, j)]);
		}
	}
#endif
	ocrDbDestroy(spmd_global()->state_guid);
	ocrShutdown();
}

ocrGuid_t spmdTriggerEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	assert(depc > 0);
	return depv[0].guid;
}

u8 spmdSend(u64 to, ocrGuid_t data, u64 size, ocrGuid_t precondition, ocrGuid_t* completion_event, bool take_db_ownership)
{
	//with precondition, don't take ownership:
	//precondition -> trigger (EDT) -> data_provider (trigger's event) -> DbCopy -> completion_event (DbCopy's event) -> channel
	//data --------/

	//no precondition, don't take ownership:
	//data -> trigger (once event) == data_provider -> DbCopy .... (as previous)

	//with precondition, take ownership:
	//precondition -> trigger (EDT) -> data_provider (trigger's event) == completion_event -> channel
	//data --------/

	//no precondition, take ownership:
	//data -> trigger (once event) == data_provider == completion_event -> channel

	ocrGuid_t buf,channel,trigger,data_provider,evt;
	if (take_db_ownership)
	{
		assert(completion_event == 0 || precondition!=NULL_GUID);
	}
	if (!completion_event) completion_event = &evt;
	if (precondition)
	{
		ocrEdtCreate(&trigger, spmd_global()->trigger_template, 0, 0, 2, 0, 0, NULL_GUID, &data_provider);
		ocrAddDependence(precondition, trigger, 1, DB_MODE_NULL);
	}
	else
	{
		ocrEventCreate(&trigger, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
		data_provider = trigger;
	}
	if (take_db_ownership)
	{
		*completion_event = data_provider;
		//ocrEventCreate(completion_event, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
		//ocrAddDependence(data_provider, *completion_event, 0, DB_MODE_NULL);
	}
	else
	{
		void* foo;
		ocrDbCreate(&buf, &foo, size, DB_PROP_NO_ACQUIRE, NULL_GUID, NO_ALLOC);
		ocrDbCopy(buf, 0, data_provider, 0, size, 0, completion_event);
	}
#ifdef CHANNELS_IN_RANGE
	ocrGuidFromIndex(&channel, spmd_global()->message_block_range, spmdMessageBoxIndex(spmdMyRank(), to));
	ocrEventCreate(&channel, OCR_EVENT_CHANNEL_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
#else
	channel = spmd_global()->channels[spmdMessageBoxIndex(spmdMyRank(), to)];
#endif
	ocrAddDependence(*completion_event, channel, 0, DB_DEFAULT_MODE);
	ocrAddDependence(data, trigger, 0, DB_MODE_RO);
	return 0;
}
u8 spmdRecv(u64 from, u64 size, ocrGuid_t* completion_event)
{
	ocrGuid_t channel;
	assert(completion_event);
#ifdef CHANNELS_IN_RANGE
	ocrGuidFromIndex(&channel, spmd_global()->message_block_range, spmdMessageBoxIndex(from, spmdMyRank()));
	ocrEventCreate(&channel, OCR_EVENT_CHANNEL_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
#else
	channel = spmd_global()->channels[spmdMessageBoxIndex(from, spmdMyRank())];
#endif
	*completion_event = channel;
	return 0;
}

spmd_task_state spmd_main_edt_state;

extern "C" ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	spmd_global_state* state;
	ocrGuid_t state_guid;
	ocrDbCreate(&state_guid, (void**)&state, sizeof(spmd_global_state), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
	new (state)spmd_global_state(state_guid);
	state->size = 2;
	ocrEdtTemplateCreate(&state->proxy_task_template, spmdProxyEdt, EDT_PARAM_UNK, EDT_PARAM_UNK);
	metaCreateKey(&state->metadata_key, GUID_USER_EDT_TEMPLATE, sizeof(spmd_template_data));
	ocrEdtTemplateCreate(&state->trigger_template, spmdTriggerEdt, EDT_PARAM_UNK, EDT_PARAM_UNK);
	set_spmd_global(state);
	spmd_main_edt_state.rank = (u64)-1;
	set_spmd_local(&spmd_main_edt_state);
#ifdef CHANNELS_IN_RANGE
	ocrGuidRangeCreate(&state->message_block_range, state->size * state->size, GUID_USER_EVENT_CHANNEL);
#else
	for (std::size_t i = 0; i < state->size; ++i)
	{
		for (std::size_t j = 0; j < state->size; ++j)
		{
			ocrEventParams_t par;
			par.EVENT_CHANNEL.maxGen = 8;
			par.EVENT_CHANNEL.nbDeps = 1;
			par.EVENT_CHANNEL.nbSat = 1;
			ocrEventCreateParams(&state->channels[spmdMessageBoxIndex(i, j)], OCR_EVENT_CHANNEL_T, EVT_PROP_TAKES_ARG,&par);
		}
	}
#endif
	return spmd_mainEdt(paramc, paramv, depc, depv);
}
