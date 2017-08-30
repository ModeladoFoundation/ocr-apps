// Monte-Carlo pi estimation example
// Contributed by Sara Hamouda <sara.salem@anu.edu.au>

#include <cstdlib>
#include <ocxxr-main.hpp>

#ifdef MEASURE_TIME
#include <ctime>
#include <ratio>
#include <chrono>
using namespace std::chrono;

high_resolution_clock::time_point start;
#endif

#define ITERS 10000

struct WorkerArgs {
    int id;
    ocxxr::Event<long> output;
};

void PiWorkerTask(WorkerArgs args) {
    PRINTF("Pi Worker task #%d started!\n", args.id);

    int seed = time(NULL);
    srand48(seed);

    long total = 0;
    double x;
    double y;
    for (int i = 0; i < ITERS; i++) {
        x = drand48();
        y = drand48();
        if (x * x + y * y <= 1.0) {
            total++;
        }
    }
    auto output_data = ocxxr::Datablock<long>::Create();
    *output_data = total;
    output_data.Release();
    args.output.Satisfy(output_data);
}

void PiAccumulatorTask(ocxxr::DatablockList<long> var_args) {
    PRINTF("Pi Accumulator task started!\n");

    long sum_points = 0;
    long successful_task_count = 0;
    float Pi = 0.0;

    for (size_t i = 0; i < var_args.count(); i++) {
        sum_points += *var_args[i];
        successful_task_count++;
        var_args[i].Destroy();
    }

    Pi = 4.0f * sum_points / (ITERS * successful_task_count);
    PRINTF("Pi equals %f \n", Pi);

    PRINTF("Shutting down...\n");

#ifdef MEASURE_TIME
	high_resolution_clock::time_point end = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(end - start);
	PRINTF("elapsed time: %f second\n", time_span.count());
#endif

    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs> args) {
#ifdef MEASURE_TIME
    start = high_resolution_clock::now();
#endif

    PRINTF("Main task started\n");
    int count = 10;
    if (args->argc() < 2) {
        PRINTF("Missing task-count parameter, defaulting to %d.\n", count);
    } else {
        count = atoi(args->argv(1));
        PRINTF("Task count = %d\n", count);
    }

    auto worker_task_template = OCXXR_TEMPLATE_FOR(PiWorkerTask);
    auto accum_task_template = OCXXR_TEMPLATE_FOR(PiAccumulatorTask);

    auto accum_task = accum_task_template().CreateTaskPartial(count);
    accum_task_template.Destroy();

    WorkerArgs worker_args;
    for (int i = 0; i < count; i++) {
        ocxxr::Event<long> out = ocxxr::OnceEvent<long>::Create();
        worker_args.id = i;
        worker_args.output = out;
        accum_task.DependOnWithinList(i, out);
        worker_task_template().CreateTask(worker_args);
    }
    worker_task_template.Destroy();
}
