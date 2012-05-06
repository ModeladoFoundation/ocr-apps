int RAG_fetch_and_add(int *ptr, int add_val)
{ return __sync_fetch_and_add(ptr,add_val); }
