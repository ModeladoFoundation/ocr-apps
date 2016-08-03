#include <ocxxr-main.hpp>

void SecondFileTask();

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    auto task_template = OCXXR_TEMPLATE_FOR(SecondFileTask);
    task_template().CreateTask();
}
