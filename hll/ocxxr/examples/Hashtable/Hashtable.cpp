/*
 * Hashtable.cpp
 *
 * Based on the CnC-OCR internal hashtable implementation
 * used for backing general item collections.
 */

#include <ocxxr-main.hpp>

#include <cstdlib>

u64 myhash(u64 x) { return x * 11400714819323198549UL; }

enum HashtableOpRole {
    kRoleInvalid = 'X',
    kRoleGetter = 'G',
    kRolePutter = 'P',
};

constexpr bool kHashtableVerbose = false;

template <typename K, typename V>
struct HashtableHelper {
    // Some constants...
    static constexpr u32 kItemsPerBlock = 64;
    static constexpr u32 kHashBucketCount = 32;
    static constexpr u32 kNotFound = kItemsPerBlock;

    struct HashtableOpParams;
    typedef ocxxr::Datablock<HashtableOpParams> ParamsDb;

    class BucketBlock;
    static void BucketSearcherTask(ocxxr::BasedPtr<BucketBlock> block,
                                   ParamsDb params, ocxxr::Arena<void>);

    typedef ocxxr::Datablock<ocxxr::BasedPtr<BucketBlock>> BucketDb;
    typedef ocxxr::BasedPtr<ocxxr::BasedPtr<BucketBlock>> BucketPtr;

    struct HashtableOpParams {
        BucketPtr bucketHead;
        ocxxr::BasedPtr<BucketBlock> firstBlock;
        ocxxr::BasedPtr<BucketBlock> oldFirstBlock;
        ocrGuid_t affinity;
        HashtableOpRole role;  // TODO - remove (or only enable in debug mode)
        bool checkedFirst;
        bool firstBlockFull;
        ocxxr::TaskTemplate<decltype(BucketSearcherTask)> searcher;
        // TODO - could do union of output and value (never need both)
        ocxxr::BasedPtr<V> output;
        ocxxr::Event<void> sync;
        // Key/Value pair
        K key;
        V value;
    };

    static void Finalize(ParamsDb params) {
        if (kHashtableVerbose) {
            PRINTF("Finishing %s op...\n",
                   (params->role == kRoleGetter ? "Get" : "Put"));
        }
        params->sync.Satisfy();
        params->searcher.Destroy();
        params.Destroy();
    }

    class BucketBlock {
     public:
        u32 count() const { return count_; }
        bool is_full() const { return count_ == kItemsPerBlock; }
        const ocxxr::BasedPtr<BucketBlock> &next() const { return next_; }

        BucketBlock(ocxxr::Arena<void> arena, ocxxr::BasedPtr<BucketBlock> next)
                : count_(0),
                  next_(next),
                  keys_(arena.NewArray<K>(kItemsPerBlock)),
                  values_(arena.NewArray<V>(kItemsPerBlock)) {}

        void Insert(const K &key, const V &value) {
            assert(!is_full());
            u32 i = count_;
            keys_[i] = key;
            values_[i] = value;
            ++count_;  // must come LAST! (avoid races with readers)
        }

        u32 Find(const K &key, u32 start = 0) const {
            for (u32 i = start; i < count_; i++) {
                if (keys_[i] == key) {
                    return i;
                }
            }
            return kNotFound;
        }

        V &ValueAt(u32 index) const {
            assert(index < count_);
            return values_[index];
        }

     private:
        volatile u32 count_;  // should be using atomics
        const ocxxr::BasedPtr<BucketBlock> next_;
        const ocxxr::RelPtr<K> keys_;
        const ocxxr::RelPtr<V> values_;
    };

    static constexpr size_t kBlockBytes = sizeof(BucketBlock) +
                                          sizeof(K[kItemsPerBlock]) +
                                          sizeof(V[kItemsPerBlock]);

    // Add a new block to the bucket
    static void BlockAdderTask(BucketPtr bucket, ParamsDb params,
                               ocxxr::Arena<void>) {
        assert(params->role == kRolePutter);
        // is our first block still first?
        if (*bucket == params->firstBlock) {
            // success! we can insert the new block now
            auto block_arena = ocxxr::Arena<BucketBlock>::Create(kBlockBytes);
            auto block = block_arena.template New<BucketBlock>(
                    block_arena.Untyped(), params->firstBlock);
            *bucket = block;
            block->Insert(params->key, params->value);
            // ALL DONE!
            Finalize(params);
        } else {  // someone added a new block...
            // try searching again
            params->oldFirstBlock = params->firstBlock;
            params->firstBlock = *bucket;
            params->checkedFirst = false;
            auto block = *bucket;
            params->searcher().CreateTask(block, params, block.target_handle());
        }
    }

    // FIXME - should be method on a Bucket class???
    static void AddBlockToBucket(const ParamsDb &params) {
        assert(params->role == kRolePutter);
        auto bucket = params->bucketHead;
        auto adder_template = OCXXR_TEMPLATE_FOR(BlockAdderTask);
        auto task = adder_template().CreateTaskPartial(bucket, params);
        task.template DependOn<1>(bucket.target_handle(),
                                  ocxxr::AccessMode::kExclusive);
        adder_template.Destroy();
    }

    // Add a new entry to the head block in this bucket
    static void EntryInserterTask(ocxxr::BasedPtr<BucketBlock> block,
                                  ParamsDb params, ocxxr::Arena<void>) {
        assert(params->role == kRolePutter);
        // is it in this block?
        u32 i = block->Find(params->key);
        if (i != kNotFound) {  // Found!
            // update existing entry
            block->ValueAt(i) = params->value;
            // ALL DONE!
            Finalize(params);
        } else if (!block->is_full()) {  // add the entry if there's still room
            // add new entry
            block->Insert(params->key, params->value);
            // ALL DONE!
            Finalize(params);
        } else {  // the block filled up while we were searching
            // need to add a new block to the bucket
            AddBlockToBucket(params);
        }
    }

    // FIXME - should be method on a Block class?
    static void InsertEntryIntoBlock(
            const ParamsDb &params, const ocxxr::BasedPtr<BucketBlock> &block) {
        assert(params->role == kRolePutter);
        // try to update entry in this block
        // (writing V might not be atomic, so we need exclusive access)
        auto inserter_template = OCXXR_TEMPLATE_FOR(EntryInserterTask);
        auto task = inserter_template().CreateTaskPartial(block, params);
        task.template DependOn<1>(block.target_handle(),
                                  ocxxr::AccessMode::kExclusive);
        inserter_template.Destroy();
    }

    // FIXME - it would be great if Output was a BasedPtr,
    // so you can write the output to an arbitrary location
    // in an arbitrary datablock (instead of the first byte)
    static void OutputSetterTask(ocxxr::BasedPtr<V> val, ParamsDb params,
                                 ocxxr::Arena<void> valdb, ocxxr::Arena<void>) {
        assert(params->role == kRoleGetter);
        auto output = params->output;
        if (kHashtableVerbose) {
            PRINTF("Writing output into " GUIDF " %p from " GUIDF ", %p...\n",
                   GUIDA(output.target_guid()), &*output,
                   GUIDA(val.target_guid()), &*val);
        }
        if (!val) {
            if (kHashtableVerbose) {
                PRINTF("Null Out\n");
            }
            // FIXME - Currently no support for null return...
            *output = V{};  // return default value
        } else {
            if (kHashtableVerbose) {
                PRINTF("Non-null Out\n");
            }
            *output = *val;
        }
        valdb.Release();
        // ALL DONE!
        Finalize(params);
    }

    static void SetOutput(const ParamsDb &params,
                          const ocxxr::BasedPtr<V> &ptr) {
        assert(params->role == kRoleGetter);
        auto out_template = OCXXR_TEMPLATE_FOR(OutputSetterTask);
        out_template().CreateTask(ptr, params, params->output.target_handle(),
                                  ptr.target_handle());
        out_template.Destroy();
    }

    static void BucketGetSearcherTask(ocxxr::BasedPtr<BucketBlock> block,
                                      ParamsDb params, ocxxr::Arena<void>) {
        assert(params->role == kRoleGetter);
        if (kHashtableVerbose) {
            PRINTF("Searching bucket block (get)...\n");
        }
        // record first block size (if applicable)
        if (!params->checkedFirst) {
            params->checkedFirst = true;
            params->firstBlockFull = block->is_full();
        }
        // is it in this block?
        u32 i = block->Find(params->key);
        if (i != kNotFound) {  // Found!
            // read existing entry's value
            SetOutput(params, &block->ValueAt(i));
            // ALL DONE!
        } else if (!block->next() || block == params->oldFirstBlock) {
            // not found: return null
            SetOutput(params, nullptr);  // FIXME - this doesn't work
        } else {
            // keep looking...
            auto next = block->next();
            params->searcher().CreateTask(next, params, next.target_handle());
        }
    }

    static_assert(std::is_same<decltype(BucketSearcherTask),
                               decltype(BucketGetSearcherTask)>::value,
                  "Expecting consistent function signature");

    static void BucketPutSearcherTask(ocxxr::BasedPtr<BucketBlock> block,
                                      ParamsDb params, ocxxr::Arena<void>) {
        assert(params->role == kRolePutter);
        // record first block size (if applicable)
        if (!params->checkedFirst) {
            params->checkedFirst = true;
            params->firstBlockFull = block->is_full();
        }
        // is it in this block?
        u32 i = block->Find(params->key);
        if (i != kNotFound) {  // Found!
            // try to update entry in this block
            // (writing V might not be atomic, so we need exclusive access)
            InsertEntryIntoBlock(params, block);
        } else if (!block->next() || block == params->oldFirstBlock) {
            // Didn't find it, so we need to try to add a new entry...
            if (params->firstBlockFull) {
                // need to add a new block to the bucket (first block was full)
                AddBlockToBucket(params);
            } else {
                // try to add a new entry to the bucket's head block
                InsertEntryIntoBlock(params, params->firstBlock);
            }
        } else {
            // keep looking...
            auto next = block->next();
            params->searcher().CreateTask(next, params, next.target_handle());
        }
    }

    static_assert(std::is_same<decltype(BucketSearcherTask),
                               decltype(BucketPutSearcherTask)>::value,
                  "Expecting consistent function signature");

    static void BucketEntryTask(BucketPtr bucket, ParamsDb params,
                                ocxxr::Arena<void>) {
        if (params->role == kRoleGetter) {
            if (kHashtableVerbose) {
                PRINTF("Searching bucket (get)...\n");
            }
        } else {
            if (kHashtableVerbose) {
                PRINTF("Searching bucket block (put)...\n");
            }
        }
        // save bucket info for first block
        params->firstBlock = *bucket;
        if (!params->firstBlock) {  // empty bucket
            if (params->role == kRolePutter) {
                // need to add a new block to the bucket
                AddBlockToBucket(params);
            } else {
                // the entry is obviously not in the bucket
                SetOutput(params, nullptr);  // FIXME - this doesn't work
            }
        } else {  // search the bucket
            auto block = params->firstBlock;
            params->searcher().CreateTask(block, params, block.target_handle());
        }
    }

    static void HasherTask(ocxxr::BasedPtr<BucketPtr> buckets, ParamsDb params,
                           ocxxr::Arena<void>) {
        if (kHashtableVerbose) {
            PRINTF("Hashing for bucket...\n");
        }
        // find the the bucket index
        // TODO - switch to std::hash
        u64 hash = myhash(params->key);
        u32 index = static_cast<u32>(hash) % kHashBucketCount;
        assert(index < kHashBucketCount);
        if (kHashtableVerbose) {
            PRINTF("Hashed to bucket %u...\n", index);
        }
        // save bucket info
        params->bucketHead = buckets[0];
        params->bucketHead = buckets[index];
        params->oldFirstBlock = nullptr;
        params->checkedFirst = false;
// affinity
// FIXME - affinitizing each bucket with node, and running all the
// lookup stuff on that node would probably be cheaper.
#ifdef OCXXR_HASHTABLE_AFFINITIES
        ocrAffinityGetCurrent(&params->affinity);
#else
        params->affinity = NULL_GUID;
#endif /* OCXXR_HASHTABLE_AFFINITIES */
        params.Release();
        // go into the bucket
        auto bucket_template = OCXXR_TEMPLATE_FOR(BucketEntryTask);
        auto bucket = params->bucketHead;
        bucket_template().CreateTask(bucket, params, bucket.target_handle());
        bucket_template.Destroy();
    }
};

template <typename K, typename V>
class Hashtable {
 public:
    typedef HashtableHelper<K, V> Helper;
    typedef typename Helper::ParamsDb ParamsDb;
    typedef typename Helper::BucketPtr BucketPtr;
    typedef typename Helper::BucketDb BucketDb;
    static constexpr u32 kHashBucketCount = Helper::kHashBucketCount;

    void Put(const K &key, const V &value, ocxxr::Event<void> sync) {
        if (kHashtableVerbose) {
            PRINTF("Starting put op...\n");
        }
        ParamsDb params = ParamsDb::Create();
        params->role = kRolePutter;
        params->key = key;
        params->value = value;
        params->sync = sync;
        params->searcher = OCXXR_TEMPLATE_FOR(Helper::BucketPutSearcherTask);
        OpHelper(params);
    }

    void Get(const K &key, ocxxr::BasedPtr<V> out, ocxxr::Event<void> sync) {
        if (kHashtableVerbose) {
            PRINTF("Starting get op...\n");
        }
        ParamsDb params = ParamsDb::Create();
        params->role = kRoleGetter;
        params->key = key;
        params->output = out;
        params->sync = sync;
        params->searcher = OCXXR_TEMPLATE_FOR(Helper::BucketGetSearcherTask);
        OpHelper(params);
    }

    static ocxxr::Arena<Hashtable> Create() {
        constexpr size_t bytes =
                sizeof(Hashtable) + sizeof(*buckets_) * kHashBucketCount;
        // XXX - adding 32 extra bytes for alignment padding
        auto arena = ocxxr::Arena<Hashtable>::Create(bytes + 32);
        CreateIn(arena.Untyped());
        return arena;
    }

    static Hashtable *CreateIn(ocxxr::Arena<void> arena) {
        return arena.New<Hashtable>(arena);
    }

    Hashtable(ocxxr::Arena<void> arena) {
        auto buckets = arena.template NewArray<BucketPtr>(kHashBucketCount);
        for (u32 i = 0; i < kHashBucketCount; i++) {
            auto bucket = BucketDb::Create();
            *bucket = nullptr;
            buckets[i] = bucket.data_ptr();
            bucket.Release();
        }
        buckets_ = buckets;
    }

 private:
    ocxxr::RelPtr<BucketPtr> buckets_;

    void OpHelper(ParamsDb params) {
        assert(params->role != kRoleInvalid);
        params.Release();
        // FIXME - doing this extra lookup once per op seems bad...?
        // but it might be good to leave for benchmarking
        ocxxr::BasedPtr<BucketPtr> buckets = buckets_;
        // Start searching...
        auto hasher_template = OCXXR_TEMPLATE_FOR(Helper::HasherTask);
        hasher_template().CreateTask(buckets, params, buckets.target_handle());
        hasher_template.Destroy();
    }

    // FIXME - need to do a deep traversal to really destroy the collection
};

void FinalTask(ocxxr::Arena<void> table, ocxxr::Datablock<char> result,
               ocxxr::Datablock<void>) {
    table.Destroy();  // FIXME - should do deep destroy
    char res_str[] = {*result ? *result : 'X', '\0'};
    PRINTF("Result = %s\n", res_str);
    ocxxr::Shutdown();
}

void GetterTask(ocxxr::Arena<Hashtable<u64, char>> table, ocxxr::NullHandle) {
    if (kHashtableVerbose) {
        PRINTF("Starting gets\n");
    }
    auto result = ocxxr::Datablock<char>::Create();
    auto sync = ocxxr::OnceEvent<void>::Create();
    auto end_template = OCXXR_TEMPLATE_FOR(FinalTask);
    end_template().CreateTask(table.Untyped(), result, sync);
    end_template.Destroy();
    table->Get(6, result.data_ptr(), sync);
    if (kHashtableVerbose) {
        PRINTF("Done with gets\n");
    }
}

struct CreateTaskParam {
    u64 start;
    u64 end;
    ocxxr::Event<void> sync;
};

void CreateBlockTask(CreateTaskParam param,
                     ocxxr::Arena<Hashtable<u64, char>> table) {
    for (u64 i = param.start; i < param.end; i++) {
        table->Put(i, static_cast<char>('a' + i), param.sync);
    }
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs> args) {
    u32 n;
    if (args->argc() != 2) {
        n = 100000;
        PRINTF("Usage: Hashtable <num>, defaulting to %" PRIu32 "\n", n);
    } else {
        n = atoi(args->argv(1));
    }

    auto table = Hashtable<u64, char>::Create();

    u64 kPutCount = n;
    auto puts_latch = ocxxr::LatchEvent<void>::Create(kPutCount);

    auto getter_template = OCXXR_TEMPLATE_FOR(GetterTask);
    getter_template().CreateTask(table.handle(), puts_latch);
    getter_template.Destroy();

    if (kHashtableVerbose) {
        PRINTF("Starting puts\n");
    }
    u64 blockSize = 5000;
    auto create_task_template = OCXXR_TEMPLATE_FOR(CreateBlockTask);
    for (u64 i = 0; i < kPutCount / blockSize; i++) {
        CreateTaskParam param = {i * blockSize, (i + 1) * blockSize,
                                 puts_latch};
        create_task_template().CreateTask(param, table);
    }
    create_task_template.Destroy();

    //    for (u64 i = 0; i < kPutCount; i++) {
    //        table->Put(i, static_cast<char>('a' + i), puts_latch);
    //    }
    if (kHashtableVerbose) {
        PRINTF("Done with puts\n");
    }
}
