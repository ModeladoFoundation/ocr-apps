#include <ocxxr.hpp>

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    ocxxr::Event<void> v0 = NullHandle();
    (void)v0;
    ocxxr::Event<double> v1 = NullHandle();
    (void)v1;
    ocxxr::StickyEvent<u32> v2 = NullHandle();
    (void)v2;
    ocxxr::OnceEvent<ocxxr::OnceEvent<ObjectHandle>> v3 = NullHandle();
    (void)v3;
    ocxxr::DatablockHandle<float> v4 = NullHandle();
    (void)v4;
    ocxxr::DataHandle<NullHandle> v5 = NullHandle();
    (void)v5;
    ocxxr::Datablock<void> v6 = NullHandle();
    (void)v6;
    PRINTF("Done.\n");
    ocxxr::Shutdown();
}
