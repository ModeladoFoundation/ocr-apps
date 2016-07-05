#include <ocxxr.hpp>

void ocxxr::Main(ocxxr::AcquiredDatablock<ocxxr::MainTaskArgs>) {
    PRINTF("Hello world!\n");
    ocxxr::Shutdown();
}
