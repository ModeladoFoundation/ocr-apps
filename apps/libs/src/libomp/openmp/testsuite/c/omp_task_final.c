<ompts:test>
<ompts:testdescription>Test which checks the omp task directive. The idea of the tests is to generate a set of tasks in a single region. We let pause the tasks generated so that other threads get sheduled to the newly opened tasks.</ompts:testdescription>
<ompts:ompversion>3.0</ompts:ompversion>
<ompts:directive>omp task final</ompts:directive>
<ompts:dependences>omp single</ompts:dependences>
<ompts:testcode>
#include <stdio.h>
#include <math.h>
#include "omp_testsuite.h"
#include "omp_my_sleep.h"

#define FINAL_START 11

int <ompts:testcode:functionname>omp_task_final</ompts:testcode:functionname>(FILE * logFile){
    <ompts:orphan:vars>
    int tids[NUM_TASKS][2];
    int i;
    </ompts:orphan:vars>
    int error = 0;
#pragma omp parallel
{
#pragma omp single
    {
        for (i = 0; i < NUM_TASKS; i++) {
            <ompts:orphan>
            /* First we have to store the value of the loop index in a new variable
             * which will be private for each task because otherwise it will be overwritten
             * if the execution of the task takes longer than the time which is needed to
             * enter the next step of the loop!
             */
            int myi;
            myi = i;
            fprintf (logFile, "%02d: Single thread nr %d\n", myi, omp_get_thread_num());

            #pragma omp task <ompts:check>final(i>=FINAL_START)</ompts:check>
            {

                int ti;
                ti = myi;

                tids[ti][0] = omp_get_thread_num();

                fprintf (logFile, "%02d: Task thread nr %d\n", ti, tids[ti][0]);

                #pragma omp task
                {
                    //if( !omp_in_final() )
                        //my_sleep (SLEEPTIME_LONG);

                    tids[ti][1] = omp_get_thread_num();
                    fprintf (logFile, "%02d: Woke, thread nr %d\n", ti, tids[ti][1]);
                }
            } /* end of omp task */
            </ompts:orphan>
        } /* end of for */
    } /* end of single */
} /*end of parallel */

/* Now we check if more than one thread executed the tasks. */
    fprintf( logFile, "Final start = %d\n", FINAL_START );
    for (i = FINAL_START; i < NUM_TASKS; i++) {
        if (tids[i][0] != tids[i][1]) {
            fprintf( logFile, "tids[%d][0] (%d) != tids[%d][1] (%d)\n",
                    i, tids[i][0], i, tids[i][1] );
            error++;
        }
    }
    return (error==0);
} /* end of check_parallel_for_private */
</ompts:testcode>
</ompts:test>
