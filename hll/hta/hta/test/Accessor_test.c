#include <stdio.h>
#include <assert.h>
#include "Accessor.h"
#include "Tuple.h"
#include "Region.h"

void print_accessor_type(void * x) {
    switch(ACCESSOR_TYPE(x)) {
        case(ACCESSOR_TUPLE):
            printf("The accessor is a tuple\n");
            break;
        case(ACCESSOR_REGION):
            printf("The accessor is a region\n");
            break;
        default:
            assert(0 && "Unsupported accessor type");
    }
}

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    Tuple t0;
    Tuple_init(&t0, 2, 1, 1);
    print_accessor_type(&t0);

    Range range1;
    Range_init(&range1, 0, 0, 1, 1);
    Region r;
    Region_init(&r, 1, range1);
    print_accessor_type(&r);
    return 0;
}
