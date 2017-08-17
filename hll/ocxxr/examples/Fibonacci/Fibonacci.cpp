/*
 * Fibonacci.cpp
 *
 * Based on the Fibonacci example in the OCR Apps repo:
 * apps/apps/examples/fib/fib.c
 */

#include <ocxxr-main.hpp>

#include <cstdlib>

#ifdef MEASURE_TIME
#include <ctime>
#include <ratio>
#include <chrono>
using namespace std::chrono;

high_resolution_clock::time_point start;
#endif

static constexpr bool kFibVerbose = false;

// Using the same recursive structure as in the EDTs
u64 SequentialFib(u32 n) {
    // Base case
    if (n < 2) return n;
    // Recursive case
    return SequentialFib(n - 1) + SequentialFib(n - 2);
}

void CheckResult(u32 n, ocxxr::Datablock<u64> result) {
    if (kFibVerbose) {
        PRINTF("Computation completed!\n");
    }
    PRINTF("Fib(%" PRIu32 ") = %" PRIu64 "\n", n, *result);
    const u64 expected_answer = SequentialFib(n);
    assert(*result == expected_answer);

#ifdef MEASURE_TIME
	high_resolution_clock::time_point end = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(end - start);
	PRINTF("elapsed time: %f second\n", time_span.count());
#endif

    ocxxr::Shutdown();
}

struct FibContinuationParams {
    u32 n;
    ocxxr::Event<u64> output;
};

void FibContinuation(FibContinuationParams &params, ocxxr::Datablock<u64> lhs,
                     ocxxr::Datablock<u64> rhs) {
    if (kFibVerbose) {
        PRINTF("Completing Fib(%" PRIu32 ")...\n", params.n);
    }
    *lhs += *rhs;
    rhs.handle().Destroy();
    params.output.Satisfy(lhs);
}

struct FibParams;
void Fib(FibParams &params);

struct FibParams {
    u32 n;
    ocxxr::Event<u64> output;
    ocxxr::TaskTemplate<decltype(Fib)> task_template;
    ocxxr::TaskTemplate<decltype(FibContinuation)> continuation_template;
};

void Fib(FibParams &params) {
    if (kFibVerbose) {
        PRINTF("Starting Fib(%" PRIu32 ")...\n", params.n);
    }
    if (params.n < 2) {  // Base case
        auto result = ocxxr::Datablock<u64>::Create();
        *result = params.n;
        params.output.Satisfy(result);

    } else {  // Recursive case (parallel)
        // Set up recursive tasks' output handles
        auto lhs_output = ocxxr::OnceEvent<u64>::Create();
        auto rhs_output = ocxxr::OnceEvent<u64>::Create();

        // Set up continuation
        FibContinuationParams continuation_params = {params.n, params.output};
        params.continuation_template().CreateTask(continuation_params,
                                                  lhs_output, rhs_output);

        // Start left-hand recursive task
        params.n -= 1;
        params.output = lhs_output;
        params.task_template().CreateTask(params);

        // Start right-hand recursive task
        params.n -= 1;
        params.output = rhs_output;
        params.task_template().CreateTask(params);
    }
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs> args) {
#ifdef MEASURE_TIME
    start = high_resolution_clock::now();
#endif

    if (kFibVerbose) {
        PRINTF("Starting main task...\n");
    }

    u32 n;
    if (args->argc() != 2) {
        n = 20;
        PRINTF("Usage: fib <num>, defaulting to %" PRIu32 "\n", n);
    } else {
        n = atoi(args->argv(1));
    }

    // Create recursive compute task templates
    // Note: These templates are reused for the whole computation
    auto fib_template = OCXXR_TEMPLATE_FOR(Fib);
    auto continuation_template = OCXXR_TEMPLATE_FOR(FibContinuation);

    // Set up the root computation task's output handle
    auto root_output = ocxxr::OnceEvent<u64>::Create();

    // Set up the finalization task
    auto result_template = OCXXR_TEMPLATE_FOR(CheckResult);
    result_template().CreateTask(n, root_output);
    result_template.Destroy();

    // Start the computation!
    FibParams fib_params = {n, root_output, fib_template,
                            continuation_template};
    fib_template().CreateTask(fib_params);
}
