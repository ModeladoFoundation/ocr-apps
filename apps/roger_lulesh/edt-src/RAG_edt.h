#ifdef CILK
extern "C" {
#endif

void domain_AllocateNodalPersistent_edt_1(Index_t i_out, Index_t i_end, struct Domain_t *domain);

void domain_AllocateElemPersistent_edt_1(Index_t i_out, Index_t i_end, struct Domain_t *domain);

#ifdef CILK
} // extern "C"
#endif
