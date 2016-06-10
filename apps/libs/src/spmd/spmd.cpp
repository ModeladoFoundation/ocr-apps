#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <cassert>
#include <vector>
#include <iterator>
#include <memory.h>
#include <ocr.h>
#ifdef ENABLE_EXTENSION_DEBUG
#include <extensions/ocr-debug.h>
#endif
#ifdef ENABLE_EXTENSION_AFFINITY
#include <extensions/ocr-affinity.h>
#else
#error Affinity support is required by the SPMD library
#endif
#define SPMD_IMPLEMENTATION
#include "spmd.h"

//#define EDTPRINTF(X) PRINTF("%s", X)
#define EDTPRINTF(X)

//#define STORE_DATA_IN_EVENT

u8 spmdEdtCreate_internal(ocrGuid_t * guid, ocrGuid_t templateGuid, u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv, u16 properties, ocrHint_t* hint, ocrGuid_t *outputEvent, u64 rank, ocrGuid_t join, ocrGuid_t comm_guid);

struct spmd_communicator
{
	ocrGuid_t comm_guid;
	u64 size;
	ocrGuid_t& comm_block(u64 rank)
	{
		assert(rank < size);
		char* ptr = (char*)this;
		ptr += sizeof(spmd_communicator);
		return ((ocrGuid_t*)ptr)[rank];
	}
	ocrGuid_t& comm_block_end()
	{
		char* ptr = (char*)this;
		ptr += sizeof(spmd_communicator);
		return ((ocrGuid_t*)ptr)[size];
	}
	ocrGuid_t& reduce_channel(u64 rank)
	{
		assert(rank < size);
		char* ptr = (char*)&comm_block_end();
		return ((ocrGuid_t*)ptr)[rank];
	}
	ocrGuid_t& reduce_channel_end()
	{
		char* ptr = (char*)&comm_block_end();
		return ((ocrGuid_t*)ptr)[size];
	}
	ocrHint_t* affinity(u64 rank)
	{
		assert(rank < size);
		char* ptr = (char*)&reduce_channel_end();
		return &((ocrHint_t*)ptr)[rank];
	}
	u8 reduce_depth()
	{
		u8 rd = 0;
		u64 s = size;
		while (s > 0)
		{
			++rd;
			s >>= 1;
		}
		return rd;
	}
	struct child_list
	{
		u64 first;
		u64 second;
	};
	u64 distance_at_level(u8 level, u8 depth)
	{
		u8 levels_here_and_below = depth - level;
		u64 res = 1;
		while (levels_here_and_below > 0)
		{
			--levels_here_and_below;
			res *= 2;
		}
		return res - 1;
	}
	u8 level(u64 rank, u8 depth)
	{
		for (u8 l = 1;l<depth;++l)
		{
			if (rank >= l + distance_at_level(l, depth))
			{
				rank -= distance_at_level(l, depth);
			}
		}
		return (u8)rank;
	}
	child_list reduce_children(u64 rank)
	{
		child_list res;
		u8 depth = reduce_depth();
		u8 my_level = level(rank, depth);
		if (my_level + 1 < depth && rank + 1 < size) res.first = rank + 1; else res.first = -1;
		if (my_level + 1 < depth && rank + 1 + distance_at_level(my_level + 1, depth) < size) res.second = rank + 1 + distance_at_level(my_level + 1, depth); else res.second = -1;
		return res;
	}
	u64 reduce_parent(u64 rank)
	{
		u8 depth = reduce_depth();
		u64 parent = -1;
		for (u8 l = 0; l < depth; ++l)
		{
			u64 left = parent + 1;
			u64 dist = distance_at_level(l, depth);
			u64 right = parent + 1 + dist;
			if (rank == left) return parent;
			if (rank == right) return parent;
			if (rank > right) parent = right; else parent = left;
		}
		assert(0);
		return -2;
	}
	static u64 size_of_db(u64 size)
	{
		return sizeof(spmd_communicator) + size * sizeof(ocrGuid_t) + size * sizeof(ocrGuid_t) + size * sizeof(ocrHint_t);
	}
};

struct spmd_task_state
{
	spmd_task_state() : rank(-1), join(NULL_GUID), comm(0) {}
	u64 rank;
	ocrGuid_t join;
	spmd_communicator* comm;
};

struct spmd_global_state
{
	spmd_global_state(ocrGuid_t state_guid) : state_guid(state_guid) {}
	ocrGuid_t state_guid;
	ocrGuid_t task_proxy_template;
	ocrGuid_t sender_template;
	ocrGuid_t receiver_template;
	ocrGuid_t comm_cleaner_template;
	ocrGuid_t reducer_template;
	ocrGuid_t prereducer_template;
	ocrGuid_t rank_finalizer_template;
};

#define SPMD_PREFIX_SIZE ((sizeof(spmd_task_state)+sizeof(u64)-1)/sizeof(u64))

spmd_global_state* spmd_global()
{
	return (spmd_global_state*)ocrElsUserGet(0).guid;
}

void set_spmd_global(spmd_global_state* mgs)
{
	ocrGuid_t g;
	g.guid = (intptr_t)mgs;
	ocrElsUserSet(0, g);
}

spmd_task_state* spmd_local()
{
	return (spmd_task_state*)ocrElsUserGet(1).guid;
}

void set_spmd_comm(spmd_communicator* comm)
{
	assert(spmd_local());
	spmd_local()->comm = comm;
}

spmd_communicator* spmd_comm()
{
	assert(spmd_local());
	return spmd_local()->comm;
}

ocrGuid_t& spmd_comm_blocks(u64 rank)
{
	return spmd_comm()->comm_block(rank);
}

void set_spmd_local(spmd_task_state* mts)
{
	ocrGuid_t g;
	g.guid = (intptr_t)mts;
	ocrElsUserSet(1, g);
}

//use two ELS slots for the library
ocrGuid_t spmd_ocrElsUserGet(u8 offset)
{
	return ocrElsUserGet(offset + 2);
}

void spmd_ocrElsUserSet(u8 offset, ocrGuid_t data)
{
	ocrElsUserSet(offset + 2, data);
}

struct spmd_send_record
{
	u64 from;
	u64 tag;
#ifdef STORE_DATA_IN_EVENT
	ocrGuid_t data_event;
#else
	ocrGuid_t data;
#ifdef ENABLE_EXTENSION_DEBUG
	ocrGuid_t the_edt;
#endif
#endif
	ocrGuid_t signal_in;//this does not have to be stored in the record, it's just making the implementation easier
	ocrGuid_t signal_out;//this does not have to be stored in the record, it's just making the implementation easier
	ocrGuid_t tml;
	bool destroy_signal_in;
	bool first_run;
};

#define SPMD_SEND_RECORD_SIZE ((sizeof(spmd_send_record)+sizeof(u64)-1)/sizeof(u64))

struct spmd_recv_record
{
	u64 from;
	u64 tag;
	ocrGuid_t event;
	ocrGuid_t signal_in;//this does not have to be stored in the record, it's just making the implementation easier
	ocrGuid_t signal_out;//this does not have to be stored in the record, it's just making the implementation easier
#ifndef STORE_DATA_IN_EVENT
#ifdef ENABLE_EXTENSION_DEBUG
	ocrGuid_t the_edt;
#endif
#endif
	ocrGuid_t tml;
	bool destroy_signal_in;
	bool first_run;
};

#define SPMD_RECV_RECORD_SIZE ((sizeof(spmd_recv_record)+sizeof(u64)-1)/sizeof(u64))

struct spmd_send_or_recv_record
{
	enum {NONE,SEND,RECV} type;
	union
	{
		spmd_send_record send;
		spmd_recv_record recv;
	} data;
};

#define SPMD_MAX_RECORDS 16

ocrGuid_t spmdCommCleaner(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	EDTPRINTF("spmdCommCleaner\n");
	spmd_communicator* comm = (spmd_communicator*)depv[1].ptr;
	for (std::size_t i = 0; i < comm->size; ++i) ocrDbDestroy(comm->comm_block(i));
	for (std::size_t i = 0; i < comm->size; ++i) ocrEventDestroy(comm->reduce_channel(i));
	ocrDbDestroy(depv[1].guid);
	return NULL_GUID;
}

ocrHint_t local_edt_hint()
{
	ocrGuid_t local_aff;
	ocrAffinityGetCurrent(&local_aff);
	ocrHint_t res;
	ocrHintInit(&res, OCR_HINT_EDT_T);
	ocrSetHintValue(&res, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(local_aff));
	return res;
}

ocrGuid_t spmdSender(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	EDTPRINTF("spmdSender\n");
	spmd_send_record* sent = (spmd_send_record*)paramv;
	if (sent->first_run && depc == 3)
	{
		ocrDbRelease(depv[2].guid);
#ifdef STORE_DATA_IN_EVENT
		ocrEventCreate(&sent->data_event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
		ocrEventSatisfy(sent->data_event, depv[2].guid);
#else
		sent->data = depv[2].guid;
#endif
	}
	if (sent->first_run)
	{
		if (sent->destroy_signal_in && !IS_GUID_NULL(sent->signal_in)) ocrEventDestroy(sent->signal_in);
	}
	sent->first_run = false;
	std::size_t i = 0;
	std::size_t block_count = 1;
	for (;;)
	{
		if (ocrGuidIsNull(*(ocrGuid_t*)depv[i].ptr)) break;
		++block_count;
		if (i == depc || !ocrGuidIsEq(*(ocrGuid_t*)depv[i].ptr, depv[i + 1].guid))
		{
			//we need more
			ocrGuid_t nextEDT;
			ocrHint_t local = local_edt_hint();
			ocrEdtCreate(&nextEDT, sent->tml, SPMD_SEND_RECORD_SIZE, (u64*)sent, (u32)i + 2, 0, EDT_PROP_NONE, NULL_HINT/*&local*/, 0);
			for (std::size_t j = 0; j <= i; ++j) ocrAddDependence(depv[j].guid, nextEDT, (u32)j, DB_MODE_EW);
			ocrAddDependence(*(ocrGuid_t*)depv[i].ptr, nextEDT, (u32)i+1, DB_MODE_EW);
			return NULL_GUID;
		}
		++i;
	}
	if (!ocrGuidIsNull(sent->signal_out)) ocrEventSatisfy(sent->signal_out, NULL_GUID);
	for (std::size_t block = 0; block < block_count; ++block)
	{
		ocrGuid_t* next = (ocrGuid_t*)depv[block].ptr;
		spmd_send_or_recv_record* records = (spmd_send_or_recv_record*)(next + 1);
		for (u64 i = 0; i < SPMD_MAX_RECORDS; ++i)
		{
			if (records[i].type != spmd_send_or_recv_record::RECV) continue;
			spmd_recv_record& recv = records[i].data.recv;

			if ((recv.from == SPMD_ANY_SOURCE || recv.from == sent->from) && (recv.tag == SPMD_ANY_TAG || recv.tag == sent->tag))
			{
#ifdef STORE_DATA_IN_EVENT
				ocrAddDependence(sent->data_event, recv.event, 0, DB_DEFAULT_MODE);
				ocrEventDestroy(sent->data_event);
#else
#ifdef ENABLE_EXTENSION_DEBUG
				ocrNoteCausality(recv.the_edt, recv.event);
#endif
				ocrEventSatisfy(recv.event, sent->data);
#endif
				records[i].type = spmd_send_or_recv_record::NONE;
				return NULL_GUID;
			}
		}
	}
#ifdef ENABLE_EXTENSION_DEBUG
	sent->the_edt = currentEdtUserGet();
#endif
	for (std::size_t block = 0; block < block_count; ++block)
	{
		ocrGuid_t* next = (ocrGuid_t*)depv[block].ptr;
		spmd_send_or_recv_record* records = (spmd_send_or_recv_record*)(next + 1);
		for (u64 i = 0; i < SPMD_MAX_RECORDS; ++i)
		{
			if (records[i].type != spmd_send_or_recv_record::NONE) continue;
			records[i].type = spmd_send_or_recv_record::SEND;
			records[i].data.send = *sent;
			return NULL_GUID;
		}
	}
	ocrGuid_t new_block;
	ocrGuid_t* ptr;
	ocrDbCreate(&new_block, (void**)&ptr, sizeof(ocrGuid_t) + SPMD_MAX_RECORDS * sizeof(spmd_send_or_recv_record), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
	*(ocrGuid_t*)depv[block_count - 1].ptr = new_block;
	ptr[0] = NULL_GUID;
	spmd_send_or_recv_record* records = (spmd_send_or_recv_record*)(ptr + 1);
	for (std::size_t j = 1; j < SPMD_MAX_RECORDS; ++j) records[j].type = spmd_send_or_recv_record::NONE;
	records[0].type = spmd_send_or_recv_record::SEND;
	records[0].data.send = *sent;
	return NULL_GUID;
}

ocrGuid_t spmdReceiver(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	EDTPRINTF("spmdReceiver\n");
	spmd_recv_record* recv = (spmd_recv_record*)paramv;
	if (recv->first_run)
	{
		if (recv->destroy_signal_in && !IS_GUID_NULL(recv->signal_in)) ocrEventDestroy(recv->signal_in);
	}
	recv->first_run = false;
	std::size_t i = 0;
	std::size_t block_count = 1;
	for (;;)
	{
		if (ocrGuidIsNull(*(ocrGuid_t*)depv[i].ptr)) break;
		++block_count;
		if (i == depc || !ocrGuidIsEq(*(ocrGuid_t*)depv[i].ptr, depv[i + 1].guid))
		{
			//we need more
			ocrGuid_t nextEDT;
			ocrHint_t local = local_edt_hint();
			ocrEdtCreate(&nextEDT, recv->tml, SPMD_RECV_RECORD_SIZE, (u64*)recv, (u32)i + 2, 0, EDT_PROP_NONE, NULL_HINT/*&local*/, 0);
			for (std::size_t j = 0; j <= i; ++j) ocrAddDependence(depv[j].guid, nextEDT, (u32)j, DB_MODE_EW);
			ocrAddDependence(*(ocrGuid_t*)depv[i].ptr, nextEDT, (u32)i + 1, DB_MODE_EW);
			return NULL_GUID;
		}
		++i;
	}
	if (!ocrGuidIsNull(recv->signal_out)) ocrEventSatisfy(recv->signal_out, NULL_GUID);

	for (std::size_t block = 0; block < block_count; ++block)
	{
		ocrGuid_t* next = (ocrGuid_t*)depv[block].ptr;
		spmd_send_or_recv_record* records = (spmd_send_or_recv_record*)(next + 1);
		for (u64 i = 0; i < SPMD_MAX_RECORDS; ++i)
		{
			if (records[i].type != spmd_send_or_recv_record::SEND) continue;
			spmd_send_record& send = records[i].data.send;
			if ((recv->from == SPMD_ANY_SOURCE || recv->from == send.from) && (recv->tag == SPMD_ANY_TAG || recv->tag == send.tag))
			{
#ifdef STORE_DATA_IN_EVENT
				ocrAddDependence(send.data_event, recv->event, 0, DB_DEFAULT_MODE);
				ocrEventDestroy(send.data_event);
#else
#ifdef ENABLE_EXTENSION_DEBUG
				ocrNoteCausality(send.the_edt, recv->event);
#endif
				ocrEventSatisfy(recv->event, send.data);
#endif
				records[i].type = spmd_send_or_recv_record::NONE;
				return NULL_GUID;
			}
		}
	}
#ifdef ENABLE_EXTENSION_DEBUG
	recv->the_edt = currentEdtUserGet();
#endif
	for (std::size_t block = 0; block < block_count; ++block)
	{
		ocrGuid_t* next = (ocrGuid_t*)depv[block].ptr;
		spmd_send_or_recv_record* records = (spmd_send_or_recv_record*)(next + 1);
		for (u64 i = 0; i < SPMD_MAX_RECORDS; ++i)
		{
			if (records[i].type != spmd_send_or_recv_record::NONE) continue;
			records[i].type = spmd_send_or_recv_record::RECV;
			records[i].data.recv = *recv;
			return NULL_GUID;
		}
	}
	ocrGuid_t new_block;
	ocrGuid_t* ptr;
	ocrDbCreate(&new_block, (void**)&ptr, sizeof(ocrGuid_t) + SPMD_MAX_RECORDS * sizeof(spmd_send_or_recv_record), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
	*(ocrGuid_t*)depv[block_count - 1].ptr = new_block;
	ptr[0] = NULL_GUID;
	spmd_send_or_recv_record* records = (spmd_send_or_recv_record*)(ptr + 1);
	for (std::size_t j = 1; j < SPMD_MAX_RECORDS; ++j) records[j].type = spmd_send_or_recv_record::NONE;
	records[0].type = spmd_send_or_recv_record::RECV;
	records[0].data.recv = *recv;
	return NULL_GUID;
}

u8 spmdPSend(u64 to, u32 tag, void* ptr, u64 size, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent)
{
	ocrGuid_t db;
	void* db_ptr;
	ocrDbCreate(&db, &db_ptr, size, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
	::memcpy(db_ptr, ptr, size);
	ocrDbRelease(db);
	return spmdGSend(to, tag, db, triggerEvent, destroyTriggerEvent, continuationEvent);
}

u8 spmdGSend(u64 to, u32 tag, ocrGuid_t dbOrEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent)
{
	spmd_send_record send;
#ifdef STORE_DATA_IN_EVENT
	send.data_event = UNINITIALIZED_GUID;
#else
	send.data = UNINITIALIZED_GUID;
#endif
	send.from = spmdMyRank();
	send.tag = tag;
	send.signal_in = triggerEvent;
	send.signal_out = continuationEvent;
	send.destroy_signal_in = destroyTriggerEvent;
	send.tml = spmd_global()->sender_template;
	send.first_run = true;
	ocrGuid_t sender;
	ocrEdtCreate(&sender, spmd_global()->sender_template, SPMD_SEND_RECORD_SIZE, (u64*)&send, 3, 0, EDT_PROP_NONE, spmd_comm()->affinity(to), 0);
	ocrAddDependence(spmd_comm_blocks(to), sender, 0, DB_MODE_EW);
	ocrAddDependence(triggerEvent, sender, 1, DB_MODE_NULL);
	ocrAddDependence(dbOrEvent, sender, 2, DB_MODE_RO);
	return 0;
}

u8 spmdRecv(u64 from, u32 tag, ocrGuid_t destinationEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent)
{
	spmd_recv_record recv;
	recv.from = from;
	recv.tag = tag;
	recv.event = destinationEvent;
	recv.signal_in = triggerEvent;
	recv.signal_out = continuationEvent;
	recv.destroy_signal_in = destroyTriggerEvent;
	recv.first_run = true;
	recv.tml = spmd_global()->receiver_template;
	ocrGuid_t receiver;
	ocrHint_t local = local_edt_hint();
	ocrEdtCreate(&receiver, spmd_global()->receiver_template, SPMD_RECV_RECORD_SIZE, (u64*)&recv, 2, 0, EDT_PROP_NONE, &local, 0);
	ocrAddDependence(spmd_comm_blocks(spmdMyRank()), receiver, 0, DB_MODE_EW);
	ocrAddDependence(triggerEvent, receiver, 1, DB_MODE_NULL);
	return 0;
}

struct reduce_def
{
	reduce_def(reduceType_t type, reduceOperation_t operation, u64 count, ocrGuid_t local_data_read_event, ocrGuid_t local_continuation, ocrGuid_t sink, u8 source_count, bool destroy_trigger, u64 root, ocrGuid_t root_output_event, bool destroy_source_data, ocrGuid_t trigger)
		: type(type),
		operation(operation),
		source_count(source_count),
		destroy_trigger(destroy_trigger),
		destroy_source_data(destroy_source_data),
		count(count),
		root(root),
		local_data_read_event(local_data_read_event),
		local_continuation(local_continuation),
		sink(sink),
		root_output_event(root_output_event),
		trigger(trigger)
	{}
	reduceType_t type;
	reduceOperation_t operation;
	u8 source_count;
	bool destroy_trigger;
	bool destroy_source_data;
	u64 count;
	u64 root;
	ocrGuid_t local_data_read_event;
	ocrGuid_t local_continuation;
	ocrGuid_t sink;
	ocrGuid_t root_output_event;
	ocrGuid_t trigger;

	ocrGuid_t reducer;
	static u32 size_in_u64()
	{
		return u32((sizeof(reduce_def) + sizeof(u64) - 1) / sizeof(u64));
	}
	u64 size_of_data()
	{
		assert(type == SPMD_REDUCE_TYPE_DOUBLE);
		return count * sizeof(double);
	}
	static u64 size_of_data(reduceType_t type, u64 count)
	{
		assert(type == SPMD_REDUCE_TYPE_DOUBLE);
		return count * sizeof(double);
	}
	void copy(void* from, void* to)
	{
		::memcpy(to, from, size_of_data());
	}
	static void copy(reduceType_t type, u64 count, void* from, void* to)
	{
		::memcpy(to, from, size_of_data(type, count));
	}
	void reduce_add(void* from, void* to)
	{
		assert(type == SPMD_REDUCE_TYPE_DOUBLE);
		assert(operation== SPMD_REDUCE_OP_SUM);
		double* dfrom = (double*)from;
		double* dto = (double*)to;
		for (u64 i = 0; i < count; ++i)
		{
			dto[i] += dfrom[i];
		}
	}
};

ocrGuid_t spmdPreReducer(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	EDTPRINTF("spmdPreReducer\n");
	reduce_def* def = (reduce_def*)paramv;
	if (def->destroy_trigger)
	{
		assert(depc == 1);
		ocrEventDestroy(def->trigger);
	}
	spmd_communicator::child_list children = spmd_comm()->reduce_children(spmdMyRank());
	if (spmdMyRank() == def->root) ocrAddDependence(spmd_comm()->reduce_channel(0), def->root_output_event, 0, DB_DEFAULT_MODE);
	if (children.first != -1) ocrAddDependence(spmd_comm()->reduce_channel(children.first), def->reducer, 1, DB_MODE_EW);
	if (children.second != -1) ocrAddDependence(spmd_comm()->reduce_channel(children.second), def->reducer, 2, DB_MODE_EW);

	if (!ocrGuidIsNull(def->local_continuation)) ocrEventSatisfy(def->local_continuation, NULL_GUID);
	return NULL_GUID;
}

ocrGuid_t spmdReducer(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	EDTPRINTF("spmdReducer\n");
	reduce_def* def = (reduce_def*)paramv;
	ocrGuid_t res;
	if (def->source_count == 1)
	{
		void* ptr;
		ocrDbCreate(&res, &ptr, def->size_of_data(), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
		def->copy(depv[0].ptr, ptr);
	}
	if (def->source_count == 2)
	{
		def->reduce_add(depv[0].ptr, depv[1].ptr);
		res = depv[1].guid;
	}
	if (def->source_count == 3)
	{
		def->reduce_add(depv[0].ptr, depv[1].ptr);
		def->reduce_add(depv[2].ptr, depv[1].ptr);
		ocrDbDestroy(depv[2].guid);
		res = depv[1].guid;
	}
	if (def->destroy_source_data) ocrDbDestroy(depv[0].guid);
	else ocrDbRelease(depv[0].guid);
	ocrDbRelease(res);
	ocrEventSatisfy(def->sink, res);
	if (!ocrGuidIsNull(def->local_data_read_event)) ocrEventSatisfy(def->local_data_read_event, depv[0].guid);
	return NULL_GUID;
}

u8 spmdGDReduce(reduceType_t type, reduceOperation_t operation, u64 count, u64 root, ocrGuid_t dbOrEvent, ocrGuid_t localCompletionEvent, ocrGuid_t rootOutputEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent, bool destroy_block)
{
	assert(ocrGuidIsNull(localCompletionEvent) || !destroy_block);//
	spmd_communicator::child_list children = spmd_comm()->reduce_children(spmdMyRank());
	u32 depc = 1;
	if (children.first != -1) ++depc;
	if (children.second != -1) ++depc;
	ocrGuid_t reducer, prereducer, prereducer_event;
	reduce_def def(type, operation, count, localCompletionEvent, continuationEvent, spmd_comm()->reduce_channel(spmdMyRank()), depc, destroyTriggerEvent, root, rootOutputEvent, destroy_block, triggerEvent);
	ocrHint_t local = local_edt_hint();
	ocrEdtCreate(&reducer, spmd_global()->reducer_template, reduce_def::size_in_u64(), (u64*)&def, depc + 1, 0, EDT_PROP_NONE, NULL_HINT/*&local*/, 0);
	def.reducer = reducer;
	spmdEdtCreate_internal(&prereducer, spmd_global()->prereducer_template, reduce_def::size_in_u64(), (u64*)&def, 1, 0, EDT_PROP_NONE, &local, &prereducer_event, spmdMyRank(), spmd_local()->join, spmd_comm()->comm_guid);
	ocrAddDependence(prereducer_event, reducer, depc, DB_MODE_NULL);
	ocrAddDependence(triggerEvent, prereducer, 0, DB_MODE_NULL);
	ocrAddDependence(dbOrEvent, reducer, 0, DB_MODE_CONST);
	return 0;
}

u8 spmdGReduce(reduceType_t type, reduceOperation_t operation, u64 count, u64 root, ocrGuid_t dbOrEvent, ocrGuid_t localCompletionEvent, ocrGuid_t rootOutputEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent)
{
	return spmdGDReduce(type, operation, count, root, dbOrEvent, localCompletionEvent, rootOutputEvent, triggerEvent, destroyTriggerEvent, continuationEvent, false);
}

u8 spmdDReduce(reduceType_t type, reduceOperation_t operation, u64 count, u64 root, ocrGuid_t dbOrEvent, ocrGuid_t rootOutputEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent)
{
	return spmdGDReduce(type, operation, count, root, dbOrEvent, NULL_GUID, rootOutputEvent, triggerEvent, destroyTriggerEvent, continuationEvent, true);
}

u8 spmdPReduce(reduceType_t type, reduceOperation_t operation, u64 count, u64 root, void* data, ocrGuid_t rootOutputEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent)
{
	ocrGuid_t dataDBK;
	void* dataPTR;
	ocrDbCreate(&dataDBK, &dataPTR, reduce_def::size_of_data(type, count), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
	reduce_def::copy(type, count, data, dataPTR);
	ocrDbRelease(dataDBK);
	return spmdDReduce(type, operation, count, root, dataDBK, rootOutputEvent, triggerEvent, destroyTriggerEvent, continuationEvent);
}

#define SPMD_ADDED_DEP_COUNT (3)

struct template_data
{
#ifdef ENABLE_EXTENSION_HETEROGENEOUS_FUNCTIONS
	u64 funcIdx;
	#else
	ocrEdt_t funcPtr;
#endif
	char name[256];
};


ocrGuid_t spmdProxyEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	EDTPRINTF("spmdProxyEdt\n");
	//the last three dependences are function, global state, and communicator
	set_spmd_global(static_cast<spmd_global_state*>(depv[depc - 2].ptr));
	spmd_task_state* prefix = reinterpret_cast<spmd_task_state*>(paramv);
	template_data* tml = (template_data*)depv[depc-3].ptr;
	set_spmd_local(prefix);
	set_spmd_comm(static_cast<spmd_communicator*>(depv[depc - 1].ptr));
#ifdef ENABLE_EXTENSION_DEBUG
	ocrAttachDebugLabel(currentEdtUserGet(), tml->name, 0, 0);
#endif
#ifdef ENABLE_EXTENSION_HETEROGENEOUS_FUNCTIONS
	ocrEdt_t ptr = (ocrEdt_t)ocrDecodeFunctionPointer(tml->funcIdx);
#else
	ocrEdt_t ptr = tml->funcPtr;
#endif
	return ptr(paramc - SPMD_PREFIX_SIZE, paramv + SPMD_PREFIX_SIZE, depc - SPMD_ADDED_DEP_COUNT, depv);
}

u64 spmdMyRank()
{
	return spmd_local()->rank;
}

u64 spmdSize()
{
	return spmd_comm() ? spmd_comm()->size : -1;
}

u8 spmd_ocrEdtTemplateCreate_internal(ocrGuid_t *guid, ocrEdt_t funcPtr, u32 paramc, u32 depc, const char* name)
{
	template_data* ptr;
	ocrDbCreate(guid, (void**)&ptr, sizeof(template_data), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
#ifdef ENABLE_EXTENSION_HETEROGENEOUS_FUNCTIONS
	ptr->funcIdx = ocrEncodeFunctionPointer((ocrFuncPtr_t)funcPtr);
#else
	ptr->funcPtr = funcPtr;
#endif
	if (name) strncpy(ptr->name, name, 255); else ptr->name[0] = 0;
	ptr->name[255] = 0;
	ocrDbRelease(*guid);
	return 0;
}
u8 spmd_ocrEdtTemplateDestroy(ocrGuid_t guid)
{
	//ocrDbDestroy(guid); -- the template may still be used, we cannot destroy the DB
	//ocrEdtTemplateDestroy(guid);
	return 0;
}
u8 spmdEdtCreate_internal(ocrGuid_t * guid, ocrGuid_t templateGuid, u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv, u16 properties, ocrHint_t* hint, ocrGuid_t *outputEvent, u64 rank, ocrGuid_t join, ocrGuid_t comm_guid)
{
	spmd_task_state prefix = *spmd_local();
	prefix.rank = rank;
	prefix.join = join;
	u32 client_paramc = paramc;
	u32 client_depc = depc;
	u32 effective_paramc = paramc;
	u32 effective_depc = depc;
	assert(client_paramc != EDT_PARAM_DEF);
	assert(client_depc != EDT_PARAM_DEF);
	effective_paramc = client_paramc + SPMD_PREFIX_SIZE;
	effective_depc = client_depc + SPMD_ADDED_DEP_COUNT;
	std::vector<u64> params(reinterpret_cast<u64*>(&prefix), reinterpret_cast<u64*>(&prefix) + SPMD_PREFIX_SIZE);
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
	deps.push_back(UNINITIALIZED_GUID);
	deps.push_back(UNINITIALIZED_GUID);
	assert(deps.size() == effective_depc);
	if (hint == NULL_HINT && spmd_comm() && rank!=-1) hint = spmd_comm()->affinity(rank);
	ocrEdtCreate(guid, spmd_global()->task_proxy_template, effective_paramc, &params.front(), effective_depc, &deps.front(), properties, hint, outputEvent);
	ocrAddDependence(templateGuid, *guid, depc, DB_MODE_CONST);
	ocrAddDependence(spmd_global()->state_guid, *guid, depc + 1, DB_MODE_CONST);
	ocrAddDependence(comm_guid, *guid, depc + 2, DB_MODE_CONST);//comm may be null
	return 0;
}

u8 spmd_ocrEdtCreate(ocrGuid_t * guid, ocrGuid_t templateGuid, u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv, u16 properties, ocrHint_t* hint, ocrGuid_t *outputEvent)
{
	return spmdEdtCreate_internal(guid, templateGuid, paramc, paramv, depc, depv, properties, hint, outputEvent, spmdMyRank(), spmd_local()->join, spmd_comm() ? spmd_comm()->comm_guid : NULL_GUID);
}

/*u8 spmdEdtCreateWithRank(ocrGuid_t * guid, ocrGuid_t templateGuid, u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv, u16 properties, ocrHint_t* hint, ocrGuid_t *outputEvent, u64 rank)
{
	return spmdEdtCreateWithRankAndJoin(guid, templateGuid, paramc, paramv, depc, depv, properties, hint, outputEvent, rank, spmd_local()->join);
}*/

void spmd_ocrShutdown()
{
	ocrEdtTemplateDestroy(spmd_global()->task_proxy_template);
	ocrEdtTemplateDestroy(spmd_global()->sender_template);
	ocrEdtTemplateDestroy(spmd_global()->receiver_template);
	ocrEdtTemplateDestroy(spmd_global()->comm_cleaner_template);
	ocrEdtTemplateDestroy(spmd_global()->reducer_template);

	spmd_ocrEdtTemplateDestroy(spmd_global()->prereducer_template);
	spmd_ocrEdtTemplateDestroy(spmd_global()->rank_finalizer_template);
	ocrDbDestroy(spmd_global()->state_guid);
	ocrShutdown();
}

struct finalize_params
{
	finalize_params(ocrGuid_t event, bool destroy_event) : event(event), destroy_event(destroy_event) {}
	ocrGuid_t event;
	bool destroy_event;
	static u32 size_in_u64()
	{
		return u32((sizeof(finalize_params) + sizeof(u64) - 1) / sizeof(u64));
	}

};

ocrGuid_t spmdFinalizeRank(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	finalize_params* pars = (finalize_params*)paramv;
	if (pars->destroy_event) ocrEventDestroy(pars->event);
	ocrEventSatisfySlot(spmd_local()->join, NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT);
	return NULL_GUID;
}


u8 spmdRankFinalize(ocrGuid_t triggerEvent, bool destroyTriggerEvent)
{

	if (ocrGuidIsNull(triggerEvent))
	{
		assert(destroyTriggerEvent == false);
		ocrEventSatisfySlot(spmd_local()->join, NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT);
	}
	else
	{
		if (destroyTriggerEvent)
		{
			finalize_params pars(triggerEvent, destroyTriggerEvent);
			ocrGuid_t finalizer;
			spmdEdtCreate_internal(&finalizer, spmd_global()->rank_finalizer_template, finalize_params::size_in_u64(), (u64*)&pars, 1, &triggerEvent, EDT_PROP_NONE, NULL_HINT, 0, spmdMyRank(), spmd_local()->join, spmd_comm()->comm_guid);
		}
		else
		{
			ocrAddDependence(triggerEvent, spmd_local()->join, OCR_EVENT_LATCH_DECR_SLOT, DB_MODE_NULL);
		}
	}
	return 0;
}

u64 rank_to_aff_index(u64 aff_count, u64 ranksPerAffinity, u64 rank)
{
	return (rank / ranksPerAffinity) % aff_count;
}

u8 spmdEdtSpawn(ocrGuid_t templateGuid, u64 count, u32 paramc, u64* paramv, u32 depc, ocrGuid_t* depv, ocrDbAccessMode_t* modes, u64 ranksPerAffinity, ocrGuid_t finishEvent)
{
	u64 aff_count;
	ocrAffinityCount(AFFINITY_PD, &aff_count);
	assert(aff_count > 0);
	if (aff_count > count) aff_count = count;
	std::vector<ocrGuid_t> affs((std::size_t)aff_count);
	ocrAffinityGet(AFFINITY_PD, &aff_count, &affs.front());
	ocrGuid_t join,cleanup,cleanup_done;
	ocrEventCreate(&join, OCR_EVENT_LATCH_T, EVT_PROP_NONE);
	ocrEdtCreate(&cleanup, spmd_global()->comm_cleaner_template, 0, 0, 2, 0, EDT_PROP_NONE, NULL_HINT, &cleanup_done);
	if (!ocrGuidIsNull(finishEvent)) ocrAddDependence(cleanup_done, finishEvent, 0, DB_MODE_NULL);//this could also depend directly on join, but this way is a bit "safer" (making sure the cleanup gets actually done before the likely shutdown)
	ocrAddDependence(join, cleanup, 0, DB_MODE_NULL);
	ocrGuid_t comm_guid;
	spmd_communicator* comm;
	ocrDbCreate(&comm_guid, (void**)&comm, spmd_communicator::size_of_db(count), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
	ocrAddDependence(comm_guid, cleanup, 1, DB_MODE_CONST);
	comm->size = count;
	comm->comm_guid = comm_guid;
	if (ranksPerAffinity == 0) ranksPerAffinity = count / aff_count;
	//PRINTF("aff count %d\n", (int)aff_count);
	for (std::size_t i = 0; i < comm->size; ++i)
	{
		//PRINTF("%d: " GUIDF "\n", (int)i, GUIDA(affs[rank_to_aff_index(aff_count, ranksPerAffinity, i)]));
		ocrGuid_t* ptr;
		ocrDbCreate(&comm->comm_block(i), (void**)&ptr, sizeof(ocrGuid_t) + SPMD_MAX_RECORDS * sizeof(spmd_send_or_recv_record), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
		ptr[0] = NULL_GUID;
		spmd_send_or_recv_record* ptr2 = (spmd_send_or_recv_record*)(ptr + 1);
		for (std::size_t j = 0; j < SPMD_MAX_RECORDS; ++j) ptr2[j].type = spmd_send_or_recv_record::NONE;
		ocrDbRelease(comm->comm_block(i));
		ocrHintInit(comm->affinity(i), OCR_HINT_EDT_T);
		ocrSetHintValue(comm->affinity(i), OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(affs[rank_to_aff_index(aff_count,ranksPerAffinity,i)]));
#ifdef ENABLE_EXTENSION_PARAMS_EVT
		ocrEventParams_t eparams;
		eparams.EVENT_CHANNEL.maxGen = 8;
		eparams.EVENT_CHANNEL.nbDeps = 1;
		eparams.EVENT_CHANNEL.nbSat = 1;
		ocrEventCreateParams(&comm->reduce_channel(i), OCR_EVENT_CHANNEL_T, EVT_PROP_TAKES_ARG, &eparams);
#else
		ocrEventCreate(&comm->reduce_channel(i), OCR_EVENT_CHANNEL_T, EVT_PROP_TAKES_ARG);
#endif
	}
	ocrDbRelease(comm_guid);
	u64 aff_index = 0;
	u64 aff_this_index = 0;
	for (std::size_t i = 0; i < count; ++i)
	{
		ocrGuid_t task;
		ocrEventSatisfySlot(join, NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT);
		ocrHint_t hint;
		ocrHintInit(&hint, OCR_HINT_EDT_T);
		ocrSetHintValue(&hint, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(affs[rank_to_aff_index(aff_count, ranksPerAffinity, i)]));
		spmdEdtCreate_internal(&task, templateGuid, paramc, paramv, depc, 0, EDT_PROP_NONE, &hint, 0, i, join, comm_guid);
		for (std::size_t j = 0; j < depc; ++j)
		{
			ocrAddDependence(depv[j], task, (u32)j, modes[j]);
		}
	}
	return 0;
}

extern "C" ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	/*spmd_communicator tc;
	tc.size = 15;
	PRINTF("size: %d, depth: %d\n", (int)tc.size, (int)tc.reduce_depth());
	for (u64 i = 0; i < tc.size; ++i)
	{
		PRINTF("%d: level %d, left %d, right %d, up %d\n",(int)i,(int)tc.level(i, tc.reduce_depth()),(int)tc.reduce_children(i).first, (int)tc.reduce_children(i).second, (int)tc.reduce_parent(i));
		assert(i == 0 || i == tc.reduce_children(tc.reduce_parent(i)).first || i == tc.reduce_children(tc.reduce_parent(i)).second);
	}*/

	spmd_global_state* state;
	ocrGuid_t state_guid;
	ocrDbCreate(&state_guid, (void**)&state, sizeof(spmd_global_state), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
	new (state)spmd_global_state(state_guid);
	ocrEdtTemplateCreate(&state->task_proxy_template, spmdProxyEdt, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEdtTemplateCreate(&state->sender_template, spmdSender, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEdtTemplateCreate(&state->receiver_template, spmdReceiver, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEdtTemplateCreate(&state->comm_cleaner_template, spmdCommCleaner, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEdtTemplateCreate(&state->reducer_template, spmdReducer, EDT_PARAM_UNK, EDT_PARAM_UNK);
	spmd_ocrEdtTemplateCreate_internal(&state->prereducer_template, spmdPreReducer, EDT_PARAM_UNK, EDT_PARAM_UNK, "spmdPreReducer");
	spmd_ocrEdtTemplateCreate_internal(&state->rank_finalizer_template, spmdFinalizeRank, EDT_PARAM_UNK, EDT_PARAM_UNK, "spmdFinalizeRank");
	spmd_global_state state_in_main(*state);
	state = &state_in_main;
	ocrDbRelease(state_guid);
	set_spmd_global(&state_in_main);
	spmd_task_state spmd_main_edt_state;
	spmd_main_edt_state.rank = (u64)-1;
	set_spmd_local(&spmd_main_edt_state);
	set_spmd_comm(0);
	return spmd_mainEdt(paramc, paramv, depc, depv);
}

#ifdef ENABLE_EXTENSION_HETEROGENEOUS_FUNCTIONS
extern "C" void registerEdtFunctions()
{
	ocrRegisterEdtFuntion(mainEdt);
	ocrRegisterEdtFuntion(spmdCommCleaner);
	ocrRegisterEdtFuntion(spmdSender);
	ocrRegisterEdtFuntion(spmdReceiver);
	spmd_ocrRegisterEdtFuntion(spmdPreReducer);
	ocrRegisterEdtFuntion(spmdReducer);
	ocrRegisterEdtFuntion(spmdProxyEdt);
	spmd_ocrRegisterEdtFuntion(spmdFinalizeRank);
	spmd_registerEdtFunctions();
}

u8 spmd_ocrRegisterEdtFuntion(ocrEdt_t funcPtr)
{
	return ocrRegisterFunctionPointer((ocrFuncPtr_t)funcPtr);
}
#endif
