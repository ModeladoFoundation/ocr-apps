#include <ocxxr-main.hpp>

struct Pointers {
    ocxxr::BasedPtrFor<int*> p1;
    ocxxr::BasedPtrFor<int**> p2;
    ocxxr::BasedPtrFor<int***> p3;
    ocxxr::BasedPtrFor<int****> p4;
    ocxxr::NestedBasedPtr<int, 5> p5;
    ocxxr::NestedBasedPtr<int, 5> q5;
};

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    auto x = ocxxr::Datablock<Pointers>::Create();
    auto y = ocxxr::Datablock<int>::Create();
    constexpr int kPayload = 7;
    *y = kPayload;
    x->p1 = y.data_ptr();
    x->p2 = &x->p1;
    x->p3 = &x->p2;
    x->p4 = &x->p3;
    x->p5 = &x->p4;
    x->q5 = x->p5;
    PRINTF("p4 = %d\n", ****x->p4);
    ASSERT(****x->p4 == kPayload);
    PRINTF("p5 = %d\n", *****x->p5);
    ASSERT(*****x->p5 == kPayload);
    PRINTF("q5 = %d\n", *****x->q5);
    ASSERT(*****x->q5 == kPayload);
    ocxxr::Shutdown();
}
