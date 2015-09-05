typedef void (* fptr_t)(void);
//
// constructor list end
//
const fptr_t
    __CTOR_LIST_END__[]
    __attribute__ (( aligned(sizeof(void *)) ))
    __attribute__ (( section(".ctors") )) = { (fptr_t) -1, };

//
// destructor section
//
const fptr_t
    __DTOR_LIST_END__[]
    __attribute__ (( aligned(sizeof(void *)) ))
    __attribute__ (( section(".dtors") )) = { (fptr_t) -1, };

