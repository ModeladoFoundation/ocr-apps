/** File: utils.c
 *
 *  Description:
 *    Utility functions to help at startup or if there are problems.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "ocr.h"
#include "utils.h"

struct CmdLineVals clVals = {
     0,      // cmdLineDebug
     0,      // tstSwitchNodes
     0,      // countNumCpus
     0,      // rdtscCpu
     0,      // timeRdtsc
     0,      // findMcdram
     NULL,   // runCpus
     0,      // numCpus (count of valid items in runCpus array)
     1LL<<30, // dramSize
     0,      // dramWLoops
     0,      // dramRLoops
     0,      // dramRLoopRand
     1LL<<30, // mcdramSize
     0,      // mcdramWLoops
     0,      // mcdramRLoops
     0,      // mcdramRLoopRand
     0,      // cacheLineLoops
     0,      // cacheLineOffset
     0,      // cacheAllocLoops
     1LL<<20, // cacheLineMinLarge (1MB)
};


/** Output a help message.
 *
 */
void sendHelp() {
    PRINTF("\nusage %s ARGS\n", programName);
    PRINTF("  ARGS:\n");
    PRINTF("    -help         Display this help text\n");
    PRINTF("    -tstMov       Run test of moving thread to all NUMA nodes.\n");
    PRINTF("    -dcpu         Display number of CPUs per node\n");
    PRINTF("    -rdtscCpu     Time rdtsc on current CPU (use with '-cpu')\n");
    PRINTF("    -tstTim       Run rdtsc timing test on all NUMA CPU nodes\n");
    PRINTF("    -cldbg        Enable command line debugging\n");
    PRINTF("    -findMcdram   Test finding MCRAM nodes\n");
    PRINTF("    -cpu=#[,#...] Specify which CPU(s) to run on\n");
    PRINTF("    -drSize=#     Size of DRAM reads/writes (default=1G)\n");
    PRINTF("                  You can append a letter as in '-drSize=4M'\n");
    PRINTF("                    'K' means *1<<10, 'k' means *1,000\n");
    PRINTF("                    'M' means *1<<20, 'm' means *1,000,000\n");
    PRINTF("                    'G' means *1<<30, 'g' means *1,000,000,000\n");
    PRINTF("    -drWLoops=#   Number of DRAM write loops to run\n");
    PRINTF("    -drRLoops=#   Number of DRAM read loops to run\n");
    PRINTF("    -drRRLoops=#  Number of DRAM random read loops to run\n");
    PRINTF("    -mcSize=#     Size of MCDRAM reads/writes (default=1G)\n");
    PRINTF("                  You can append a letter as in '-drSize' above\n");
    PRINTF("    -mcWLoops=#   Number of MCDRAM write loops to run\n");
    PRINTF("    -mcRLoops=#   Number of MCDRAM read loops to run\n");
    PRINTF("    -mcRRLoops=#  Number of MCDRAM random read loops to run\n");
    PRINTF("    -cacheLoops=# Number of cache line offset test loops\n");
    PRINTF("    -cacheAllocLoops=#  Number of real cache line offset loops\n");
    PRINTF("    -cacheOff=#   Number of bytes for cache line offset allocs\n");
    PRINTF("    -cacheLarge=# Minimum size that is considered large\n");
    PRINTF("\n");
}

/** Parse the command line arguments.
 *
 * @param argc  the number of arguments on the command line
 * @param argv  an array of command line arguments
 * @return 0 on success or -1 if program should exit
 */
int parseCmdLineArgs(int argc, char **argv) {
    int retVal = 0;
    int showHelp = 0;
    int numActionOps = 0;   // Used to print help if no actions
    int maxCpus = 0;

    // Set up array of CPUs...
    {
        int i;
        maxCpus = numa_num_configured_cpus();
        clVals.runCpus = (int *)malloc(maxCpus * sizeof(int));
        if (clVals.runCpus == NULL) {
            PRINTF("ERROR: Call to malloc(%l) failed: [%s]\n",
                   (maxCpus*sizeof(int)), strerror(errno));
            return -1;
        }
        for (i=0; i<maxCpus; i++)
            clVals.runCpus[i] = -1;
    }
    
    int i;
    for (i=1; i<argc; i++) {
        if ((!strncmp(argv[i], "-h", 2)) ||
            (!strncmp(argv[i], "--h", 3))) {
            sendHelp();
            return -1;
        }
        if (!strncmp(argv[i], "-dcpu", 5)) {
            clVals.countNumCpus = 1;
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-rdtscCpu", 2)) {
            clVals.rdtscCpu = 1;
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-tstTim", 5)) {
            clVals.timeRdtsc = 1;
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-cldbg", 4)) {
            clVals.cmdLineDebug = 1;
            continue;
        }
        if (!strncmp(argv[i], "-tstMov", 5)) {
            clVals.tstSwitchNodes = 1;
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-findMcdram", 6)) {
            clVals.findMcdram = 1;
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-cpu=", 5)) {
            if (argv[i][5] == '\0') {
                PRINTF("ERROR: Argument '-cpu=' requies a number!\n");
                PRINTF("       Example: '-cpu=13'\n");
                retVal = -1;
                continue;
            }
            char *endptr = NULL;
            char *startptr = &(argv[i][5]);
            int c;
            for (c=0; c<maxCpus; c++) {
                long cpu = strtol(startptr, &endptr, 0);
                if (cpu < 0) {
                    PRINTF("ERROR: Invalid CPU number '%d'!\n", cpu);
                    retVal = -1;
                    break;
                }
                if (endptr == startptr)  // no value?
                    break;
                clVals.runCpus[c] = cpu;
                clVals.numCpus++;
                if (*endptr == 0)
                    break;
                if (*endptr != ',') // more values?
                    break;
                startptr = endptr + 1;
            }
            continue;
        }
        if (!strncmp(argv[i], "-drSize=", 8)) {
            if (argv[i][8] == '\0') {
                PRINTF("ERROR: Argument '-drSize=' requires a number!\n");
                PRINTF("       Example: '-drSize=5M' or '-drSize=5000000'\n");
                retVal = -1;
                continue;
            }
            u64 val = strtoull(&(argv[i][8]), NULL, 10);
            char suffix = argv[i][strlen(argv[i])-1];
            if (suffix == 'k')
                val *= 1000;
            if (suffix == 'K')
                val *= (1<<10);
            if (suffix == 'm')
                val *= 1000 * 1000;
            if (suffix == 'M')
                val *= (1<<20);
            if (suffix == 'g')
                val *= 1000 * 1000 * 1000;
            if (suffix == 'G')
                val *= (1<<30);
            clVals.dramSize = val;
            continue;
        }
        if (!strncmp(argv[i], "-drWLoops=", 10)) {
            if (argv[i][10] == '\0') {
                PRINTF("ERROR: Argument '-drWLoops=' requires a number!\n");
                PRINTF("       Example: '-drWLoops=100'\n");
                retVal = -1;
                continue;
            }
            clVals.dramWLoops = strtoul(&(argv[i][10]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-drRLoops=", 10)) {
            if (argv[i][10] == '\0') {
                PRINTF("ERROR: Argument '-drRLoops=' requires a number!\n");
                PRINTF("       Example: '-drRLoops=100'\n");
                retVal = -1;
                continue;
            }
            clVals.dramRLoops = strtoul(&(argv[i][10]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-drRRLoops=", 11)) {
            if (argv[i][11] == '\0') {
                PRINTF("ERROR: Argument '-drRRLoops=' requires a number!\n");
                PRINTF("       Example: '-drRRLoops=100'\n");
                retVal = -1;
                continue;
            }
            clVals.dramRLoopRand = strtoul(&(argv[i][11]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-mcSize=", 8)) {
            if (argv[i][8] == '\0') {
                PRINTF("ERROR: Argument '-mcSize=' requires a number!\n");
                PRINTF("       Example: '-mcSize=5M', '-mcSize=5000000'\n");
                retVal = -1;
                continue;
            }
            u64 val = strtoull(&(argv[i][8]), NULL, 10);
            char suffix = argv[i][strlen(argv[i])-1];
            if (suffix == 'k')
                val *= 1000;
            if (suffix == 'K')
                val *= (1<<10);
            if (suffix == 'm')
                val *= 1000 * 1000;
            if (suffix == 'M')
                val *= (1<<20);
            if (suffix == 'g')
                val *= 1000 * 1000 * 1000;
            if (suffix == 'G')
                val *= (1<<30);
            clVals.mcdramSize = val;
            continue;
        }
        if (!strncmp(argv[i], "-mcWLoops=", 10)) {
            if (argv[i][10] == '\0') {
                PRINTF("ERROR: Argument '-mcWLoops=' requires a number!\n");
                PRINTF("       Example: '-mcWLoops=100'\n");
                retVal = -1;
                continue;
            }
            clVals.mcdramWLoops = strtoul(&(argv[i][10]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-mcRLoops=", 10)) {
            if (argv[i][10] == '\0') {
                PRINTF("ERROR: Argument '-mcRLoops=' requires a number!\n");
                PRINTF("       Example: '-mcRLoops=100'\n");
                retVal = -1;
                continue;
            }
            clVals.mcdramRLoops = strtoul(&(argv[i][10]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-mcRRLoops=", 11)) {
            if (argv[i][11] == '\0') {
                PRINTF("ERROR: Argument '-mcRRLoops=' requires a number!\n");
                PRINTF("       Example: '-mcRRLoops=100'\n");
                retVal = -1;
                continue;
            }
            clVals.mcdramRLoopRand = strtoul(&(argv[i][11]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-cacheLoops=", 12)) {
            if (argv[i][12] == '\0') {
                PRINTF("ERROR: Argument '-cacheLoops=' requires a number!\n");
                PRINTF("       Example: '-cacheLoops=1000'\n");
                retVal = -1;
                continue;
            }
            clVals.cacheLineLoops = strtoul(&(argv[i][12]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-cacheOff=", 10)) {
            // Only valid if 'cacheLineLoops' > 0 or 'cacheAllocLoops' > 0
            if (argv[i][10] == '\0') {
                PRINTF("ERROR: Argument '-cacheOff=' requires a number!\n");
                PRINTF("       Example: '-cacheOff=128'\n");
                retVal = -1;
                continue;
            }
            clVals.cacheLineOffset = strtoul(&(argv[i][10]), NULL, 10);
            continue;
        }
        if (!strncmp(argv[i], "-cacheAllocLoops=", 17)) {
            if (argv[i][17] == '\0') {
                PRINTF("ERROR: Argument '-cacheAllocLoops=' requires a number!\n");
                PRINTF("       Example: '-cacheAllocLoops=1000'\n");
                retVal = -1;
                continue;
            }
            clVals.cacheAllocLoops = strtoul(&(argv[i][17]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-cacheLarge=", 12)) {
            // Only valid if 'cacheAllocLoops' > 0
            if (argv[i][12] == '\0') {
                PRINTF("ERROR: Argument '-cacheLarge=' requires a number!\n");
                PRINTF("       Example: '-cacheLarge=1M'\n");
                retVal = -1;
                continue;
            }
            u64 val = strtoull(&(argv[i][12]), NULL, 10);
            char suffix = argv[i][strlen(argv[i])-1];
            if (suffix == 'k')
                val *= 1000;
            if (suffix == 'K')
                val *= (1<<10);
            if (suffix == 'm')
                val *= 1000 * 1000;
            if (suffix == 'M')
                val *= (1<<20);
            if (suffix == 'g')
                val *= 1000 * 1000 * 1000;
            if (suffix == 'G')
                val *= (1<<30);
            clVals.cacheLineMinLarge = val;
            continue;
        }
        PRINTF("ERROR: Unrecognized command line argument \"%s\"!\n",
               argv[i]);
        retVal = -1;
    }

    if (clVals.cmdLineDebug) {
        PRINTF("There %s %d command line argument%s:\n",
               ((argc==1)?"is":"are"), argc, ((argc==1)?"":"s"));
        for (i=0; i<argc; i++)
            PRINTF("    ARGV[%d] is \"%s\"\n", i, argv[i]);

        PRINTF("Command line flags:\n");
        PRINTF("    cmdLineDebug = %d\n", clVals.cmdLineDebug);
        PRINTF("    tstSwitchNodes = %d\n", clVals.tstSwitchNodes);
        PRINTF("    countNumCpus = %d\n", clVals.countNumCpus);
        PRINTF("    rdtscCpu = %d\n", clVals.rdtscCpu);
        PRINTF("    timeRdtsc = %d\n", clVals.timeRdtsc);
        PRINTF("    findMcdram = %d\n", clVals.findMcdram);
        if (clVals.numCpus == 0)
            PRINTF("    runCpus = not set\n");
        else {
            PRINTF("    runCpus = ");
            for(i=0; i<clVals.numCpus; i++) {
                if (i > 0)
                    PRINTF(", %d", clVals.runCpus[i]);
                else
                    PRINTF("%d", clVals.runCpus[i]);
            }
            PRINTF("\n");
        }
        PRINTF("    dramSize = %lu\n", clVals.dramSize);
        PRINTF("    dramWLoops = %d\n", clVals.dramWLoops);
        PRINTF("    dramRLoops = %d\n", clVals.dramRLoops);
        PRINTF("    dramRRLoops = %d\n", clVals.dramRLoopRand);
        PRINTF("    mcdramSize = %lu\n", clVals.mcdramSize);
        PRINTF("    mcdramWLoops = %d\n", clVals.mcdramWLoops);
        PRINTF("    mcdramRLoops = %d\n", clVals.mcdramRLoops);
        PRINTF("    mcdramRRLoops = %d\n", clVals.mcdramRLoopRand);
        PRINTF("    cacheLineLoops = %lu\n", clVals.cacheLineLoops);
        PRINTF("    cacheLineOffset = %lu\n", clVals.cacheLineOffset);
        PRINTF("    cacheAllocLoops = %lu\n", clVals.cacheAllocLoops);
        PRINTF("    cacheLineMinLarge = %lu\n", clVals.cacheLineMinLarge);
        PRINTF("    cacheAllocLoops = %lu\n", clVals.cacheAllocLoops);
    }

    if (numActionOps == 0) {
        PRINTF("There were no action operations specified...\n");
        sendHelp();
        return -1;
    }

    return retVal;
}
