#include <ocxxr.hpp>

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Hello world!\n");
    ocxxr::Shutdown();
}
