//===============================================
// OCR main function (entry point)
//===============================================

#if defined MEASURE_TIME || defined INSTRUMENT_POINTER_OP
#include <ctime>
#include <ratio>
#include <chrono>

std::chrono::high_resolution_clock::time_point program_start, program_end;
#endif

extern "C" {
ocrGuid_t mainEdt(u32 paramc, u64 /*paramv*/[], u32 depc, ocrEdtDep_t depv[]) {
#if defined MEASURE_TIME || defined INSTRUMENT_POINTER_OP
    program_start = std::chrono::high_resolution_clock::now();
#endif
    assert(paramc == 0 && depc == 1);
    ocxxr::internal::PushTaskState();
    ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>(depv[0]));
    ocxxr::internal::PopTaskState();
    return NULL_GUID;
}
}

namespace ocxxr {
namespace internal {

OCXXR_THREAD_LOCAL TaskLocalState *_task_local_state;

#if defined MEASURE_TIME || defined INSTRUMENT_POINTER_OP
    void recordEnd() {
        using namespace std::chrono;
        program_end = high_resolution_clock::now();
    }
#endif

#ifdef MEASURE_TIME
    void outputTime() {
        using namespace std::chrono;
        duration<double> time_span = duration_cast<duration<double>>(program_end - program_start);
        PRINTF("elapsed time: %f second\n", time_span.count());
    }
#endif

#ifdef INSTRUMENT_POINTER_OP
    std::atomic<u64> rp_indirect_count(0), rp_arrow_count(0), rp_subscript_count(0), rp_cast_count(0), rp_equal_count(0), rp_assign_count(0), rp_negate_count(0), rp_get_count(0), rp_set_count(0);
    std::atomic<u64> bp_indirect_count(0), bp_arrow_count(0), bp_subscript_count(0), bp_cast_count(0), bp_equal_count(0), bp_assign_count(0), bp_negate_count(0), bp_get_count(0), bp_set_count(0);
//    void outputAllCount() {
//        using namespace std;
//        using namespace std::chrono;
//
//        double time_span = duration_cast<duration<double>>(program_end - program_start).count() * 1000;
//
//        cout << "bp_equal_count " << bp_equal_count.load() / time_span << endl;
//        cout << "bp_indirect_count " << bp_indirect_count.load() / time_span << endl;
//        cout << "bp_arrow_count " << bp_arrow_count.load() / time_span << endl;
//        cout << "bp_subscript_count " << bp_subscript_count.load() / time_span << endl;
//        cout << "bp_cast_count " << bp_cast_count.load() / time_span << endl;
//        cout << "bp_assign_count " << bp_assign_count.load() / time_span << endl;
//        cout << "bp_negate_count " << bp_negate_count.load() / time_span << endl;
//        cout << "bp_get_count " << bp_get_count.load() / time_span << endl;
//        cout << "bp_set_count " << bp_set_count.load() / time_span << endl;
//        cout << "rp_equal_count " << rp_equal_count.load() / time_span << endl;
//        cout << "rp_indirect_count " << rp_indirect_count.load() / time_span << endl;
//        cout << "rp_arrow_count " << rp_arrow_count.load() / time_span << endl;
//        cout << "rp_subscript_count " << rp_subscript_count.load() / time_span << endl;
//        cout << "rp_cast_count " << rp_cast_count.load() / time_span << endl;
//        cout << "rp_assign_count " << rp_assign_count.load() / time_span << endl;
//        cout << "rp_negate_count " << rp_negate_count.load() / time_span << endl;
//        cout << "rp_get_count " << rp_get_count.load() / time_span << endl;
//        cout << "rp_set_count " << rp_set_count.load() / time_span << endl;
//    }
    void outputAllCount() {
        using namespace std;
        using namespace std::chrono;


        cout << "bp_equal_count " << bp_equal_count.load()  << endl;
        cout << "bp_indirect_count " << bp_indirect_count.load()  << endl;
        cout << "bp_arrow_count " << bp_arrow_count.load()  << endl;
        cout << "bp_subscript_count " << bp_subscript_count.load()  << endl;
        cout << "bp_cast_count " << bp_cast_count.load()  << endl;
        cout << "bp_assign_count " << bp_assign_count.load()  << endl;
        cout << "bp_negate_count " << bp_negate_count.load()  << endl;
        cout << "bp_get_count " << bp_get_count.load()  << endl;
        cout << "bp_set_count " << bp_set_count.load()  << endl;
        cout << "rp_equal_count " << rp_equal_count.load()  << endl;
        cout << "rp_indirect_count " << rp_indirect_count.load()  << endl;
        cout << "rp_arrow_count " << rp_arrow_count.load()  << endl;
        cout << "rp_subscript_count " << rp_subscript_count.load()  << endl;
        cout << "rp_cast_count " << rp_cast_count.load()  << endl;
        cout << "rp_assign_count " << rp_assign_count.load()  << endl;
        cout << "rp_negate_count " << rp_negate_count.load()  << endl;
        cout << "rp_get_count " << rp_get_count.load()  << endl;
        cout << "rp_set_count " << rp_set_count.load()  << endl;
    }
#endif

}  // namespace internal
}  // namespace ocxxr
