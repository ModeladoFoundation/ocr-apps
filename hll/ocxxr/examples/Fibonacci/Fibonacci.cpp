/*
 * Fibonacci.cpp
 *
 * Based on the Fibonacci example in the OCR Apps repo:
 * apps/apps/examples/fib/fib.c
 */

#include <ocxxr-main.hpp>

#include <cstdlib>

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
    PRINTF("Fib(%" PRIu32 ") = %" PRIu64 "\n", n, result.data());
    const u64 expected_answer = SequentialFib(n);
    ASSERT(result.data() == expected_answer);
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
    lhs.data() += rhs.data();
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
        result.data() = params.n;
        params.output.Satisfy(result);

    } else {  // Recursive case (parallel)
        // Set up recursive tasks' output handles
        auto lhs_output = ocxxr::OnceEvent<u64>();
        auto rhs_output = ocxxr::OnceEvent<u64>();

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
    if (kFibVerbose) {
        PRINTF("Starting main task...\n");
    }

    u32 n;
    if (args.data().argc() != 2) {
        n = 10;
        PRINTF("Usage: fib <num>, defaulting to %" PRIu32 "\n", n);
    } else {
        n = atoi(args.data().argv(1));
    }

    // Create recursive compute task templates
    // Note: These templates are reused for the whole computation
    auto fib_template = OCXXR_TEMPLATE_FOR(Fib);
    auto continuation_template = OCXXR_TEMPLATE_FOR(FibContinuation);

    // Set up the root computation task's output handle
    auto root_output = ocxxr::OnceEvent<u64>();

    // Set up the finalization task
    auto result_template = OCXXR_TEMPLATE_FOR(CheckResult);
    result_template().CreateTask(n, root_output);
    result_template.Destroy();

    // Start the computation!
    FibParams fib_params = {n, root_output, fib_template,
                            continuation_template};
    fib_template().CreateTask(fib_params);
}
