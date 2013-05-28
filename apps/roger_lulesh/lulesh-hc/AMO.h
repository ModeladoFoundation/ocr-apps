/* double */

void AMO__sync_addition_double(double *memPtr, double addVal);

/* uint64_t */

uint64_t AMO__sync_fetch_and_add_uint64_t(uint64_t *memPtr, uint64_t addVal);

uint64_t AMO__sync_val_compare_and_swap_uint64_t(uint64_t *memPtr, uint64_t compVal, uint64_t exchVal);

int AMO__sync_bool_compare_and_swap_uint64_t(uint64_t *memPtr, uint64_t compVal, uint64_t exchVal);

/* int64_t */

int64_t AMO__sync_fetch_and_add_int64_t(int64_t *memPtr, int64_t addVal);

int64_t AMO__sync_val_compare_and_swap_int64_t(int64_t *memPtr, int64_t compVal, int64_t exchVal);

int AMO__sync_bool_compare_and_swap_int64_t(int64_t *memPtr, int64_t compVal, int64_t exchVal);

/* LOCKS */

extern uint64_t idamin_lock; // 0 --> UNLOCKED; 1 --> LOCKED
extern uint64_t idamax_lock; // 0 --> UNLOCKED; 1 --> LOCKED

void AMO__lock_uint64_t(uint64_t *memPtr);
void AMO__unlock_uint64_t(uint64_t *memPtr);
