#include <ocxxr-main.hpp>

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    constexpr int kPayload = 7;
    int x = kPayload;
    ocxxr::RelPtr<int> p1 = &x;
    ocxxr::NestedRelPtr<int, 2> p2 = &p1;
    ocxxr::NestedRelPtr<int, 3> p3 = &p2;
    ocxxr::NestedRelPtr<int, 4> p4 = &p3;
    PRINTF("x = %d\n", ****p4);
    assert(****p4 == kPayload);
    ocxxr::Shutdown();
}
