#include <ocxxr-main.hpp>

constexpr unsigned char kPayload0 = 10;
constexpr long kPayload1 = 0x0e1;

ocxxr::NullHandle ChildTask(unsigned char suffix, ocxxr::Datablock<long> middle,
                            ocxxr::Datablock<char[2]> prefix) {
    PRINTF("Child task ran.\n");
    PRINTF("%s%lx%u!\n", prefix.data(), middle.data(), suffix);
    assert(suffix == kPayload0);
    assert(middle.data() == kPayload1);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
    return ocxxr::NullHandle();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task\n");
    auto dep1 = ocxxr::Datablock<long>::Create();
    dep1.data() = kPayload1;
    auto dep2 = ocxxr::Datablock<char[2]>::Create();
    dep2.data()[0] = 'H';
    dep2.data()[1] = '\0';
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    auto task = task_template().CreateTaskPartial(kPayload0, dep1);
    task.DependOn<1>(dep2);
}
