#include <ocxxr-main.hpp>

constexpr unsigned char kPayload0 = 10;
constexpr long kPayload1 = 0x0e1;
constexpr char kPayload2 = 'H';

ocxxr::NullHandle ChildTask(unsigned char suffix, ocxxr::Datablock<long> middle,
                            ocxxr::Datablock<char[2]> prefix) {
    PRINTF("Child task ran.\n");
    PRINTF("%s%lx%u!\n", prefix.data(), middle.data(), suffix);
    assert(suffix == kPayload0);
    assert(middle.data() == kPayload1);
    assert(prefix.data()[0] == kPayload2);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
    return ocxxr::NullHandle();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task\n");
    auto dep1 = ocxxr::Datablock<long>::Create();
    dep1.data() = kPayload1;
    auto dep2 = ocxxr::Datablock<char[2]>::Create();
    dep2.data()[0] = kPayload2;
    dep2.data()[1] = '\0';
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template().CreateTask(kPayload0, dep1, dep2);
}
