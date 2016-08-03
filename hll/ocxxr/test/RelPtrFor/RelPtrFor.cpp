#include <ocxxr-main.hpp>

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    constexpr int kPayload = 7;
    int x = kPayload;
    ocxxr::RelPtrFor<decltype(&x)> p1 = &x;
    ocxxr::RelPtrFor<int**> p2 = &p1;
    ocxxr::RelPtrFor<int***> p3 = &p2;
    ocxxr::RelPtrFor<int****> p4 = &p3;
    ocxxr::RelPtrFor<decltype(&p3)> q4 = p4;
    PRINTF("x = %d = %d\n", ****p4, ****q4);
    ASSERT(****p4 == kPayload);
    ASSERT(****q4 == kPayload);
    ocxxr::Shutdown();
}
