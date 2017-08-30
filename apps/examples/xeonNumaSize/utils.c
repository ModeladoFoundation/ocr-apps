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
    ocrPrintf("\nusage %s ARGS\n", programName);
    ocrPrintf("  ARGS:\n");
    ocrPrintf("    -help         Display this help text\n");
    ocrPrintf("    -tstMov       Run test of moving thread to all NUMA nodes.\n");
    ocrPrintf("    -dcpu         Display number of CPUs per node\n");
    ocrPrintf("    -rdtscCpu     Time rdtsc on current CPU (use with '-cpu')\n");
    ocrPrintf("    -tstTim       Run rdtsc timing test on all NUMA CPU nodes\n");
    ocrPrintf("    -cldbg        Enable command line debugging\n");
    ocrPrintf("    -findMcdram   Test finding MCRAM nodes\n");
    ocrPrintf("    -cpu=#[,#...] Specify which CPU(s) to run on\n");
    ocrPrintf("    -drSize=#     Size of DRAM reads/writes (default=1G)\n");
    ocrPrintf("                  You can append a letter as in '-drSize=4M'\n");
    ocrPrintf("                    'K' means *1<<10, 'k' means *1,000\n");
    ocrPrintf("                    'M' means *1<<20, 'm' means *1,000,000\n");
    ocrPrintf("                    'G' means *1<<30, 'g' means *1,000,000,000\n");
    ocrPrintf("    -drWLoops=#   Number of DRAM write loops to run\n");
    ocrPrintf("    -drRLoops=#   Number of DRAM read loops to run\n");
    ocrPrintf("    -drRRLoops=#  Number of DRAM random read loops to run\n");
    ocrPrintf("    -mcSize=#     Size of MCDRAM reads/writes (default=1G)\n");
    ocrPrintf("                  You can append a letter as in '-drSize' above\n");
    ocrPrintf("    -mcWLoops=#   Number of MCDRAM write loops to run\n");
    ocrPrintf("    -mcRLoops=#   Number of MCDRAM read loops to run\n");
    ocrPrintf("    -mcRRLoops=#  Number of MCDRAM random read loops to run\n");
    ocrPrintf("    -cacheLoops=# Number of cache line offset test loops\n");
    ocrPrintf("    -cacheAllocLoops=#  Number of real cache line offset loops\n");
    ocrPrintf("    -cacheOff=#   Number of bytes for cache line offset allocs\n");
    ocrPrintf("    -cacheLarge=# Minimum size that is considered large\n");
    ocrPrintf("\n");
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
            ocrPrintf("ERROR: Call to malloc(%l) failed: [%s]\n",
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
                ocrPrintf("ERROR: Argument '-cpu=' requies a number!\n");
                ocrPrintf("       Example: '-cpu=13'\n");
                retVal = -1;
                continue;
            }
            char *endptr = NULL;
            char *startptr = &(argv[i][5]);
            int c;
            for (c=0; c<maxCpus; c++) {
                long cpu = strtol(startptr, &endptr, 0);
                if (cpu < 0) {
                    ocrPrintf("ERROR: Invalid CPU number '%d'!\n", cpu);
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
                ocrPrintf("ERROR: Argument '-drSize=' requires a number!\n");
                ocrPrintf("       Example: '-drSize=5M' or '-drSize=5000000'\n");
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
                ocrPrintf("ERROR: Argument '-drWLoops=' requires a number!\n");
                ocrPrintf("       Example: '-drWLoops=100'\n");
                retVal = -1;
                continue;
            }
            clVals.dramWLoops = strtoul(&(argv[i][10]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-drRLoops=", 10)) {
            if (argv[i][10] == '\0') {
                ocrPrintf("ERROR: Argument '-drRLoops=' requires a number!\n");
                ocrPrintf("       Example: '-drRLoops=100'\n");
                retVal = -1;
                continue;
            }
            clVals.dramRLoops = strtoul(&(argv[i][10]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-drRRLoops=", 11)) {
            if (argv[i][11] == '\0') {
                ocrPrintf("ERROR: Argument '-drRRLoops=' requires a number!\n");
                ocrPrintf("       Example: '-drRRLoops=100'\n");
                retVal = -1;
                continue;
            }
            clVals.dramRLoopRand = strtoul(&(argv[i][11]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-mcSize=", 8)) {
            if (argv[i][8] == '\0') {
                ocrPrintf("ERROR: Argument '-mcSize=' requires a number!\n");
                ocrPrintf("       Example: '-mcSize=5M', '-mcSize=5000000'\n");
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
                ocrPrintf("ERROR: Argument '-mcWLoops=' requires a number!\n");
                ocrPrintf("       Example: '-mcWLoops=100'\n");
                retVal = -1;
                continue;
            }
            clVals.mcdramWLoops = strtoul(&(argv[i][10]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-mcRLoops=", 10)) {
            if (argv[i][10] == '\0') {
                ocrPrintf("ERROR: Argument '-mcRLoops=' requires a number!\n");
                ocrPrintf("       Example: '-mcRLoops=100'\n");
                retVal = -1;
                continue;
            }
            clVals.mcdramRLoops = strtoul(&(argv[i][10]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-mcRRLoops=", 11)) {
            if (argv[i][11] == '\0') {
                ocrPrintf("ERROR: Argument '-mcRRLoops=' requires a number!\n");
                ocrPrintf("       Example: '-mcRRLoops=100'\n");
                retVal = -1;
                continue;
            }
            clVals.mcdramRLoopRand = strtoul(&(argv[i][11]), NULL, 10);
            numActionOps++;
            continue;
        }
        if (!strncmp(argv[i], "-cacheLoops=", 12)) {
            if (argv[i][12] == '\0') {
                ocrPrintf("ERROR: Argument '-cacheLoops=' requires a number!\n");
                ocrPrintf("       Example: '-cacheLoops=1000'\n");
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
                ocrPrintf("ERROR: Argument '-cacheOff=' requires a number!\n");
                ocrPrintf("       Example: '-cacheOff=128'\n");
                retVal = -1;
                continue;
            }
            clVals.cacheLineOffset = strtoul(&(argv[i][10]), NULL, 10);
            continue;
        }
        if (!strncmp(argv[i], "-cacheAllocLoops=", 17)) {
            if (argv[i][17] == '\0') {
                ocrPrintf("ERROR: Argument '-cacheAllocLoops=' requires a number!\n");
                ocrPrintf("       Example: '-cacheAllocLoops=1000'\n");
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
                ocrPrintf("ERROR: Argument '-cacheLarge=' requires a number!\n");
                ocrPrintf("       Example: '-cacheLarge=1M'\n");
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
        ocrPrintf("ERROR: Unrecognized command line argument \"%s\"!\n",
               argv[i]);
        retVal = -1;
    }

    if (clVals.cmdLineDebug) {
        ocrPrintf("There %s %d command line argument%s:\n",
               ((argc==1)?"is":"are"), argc, ((argc==1)?"":"s"));
        for (i=0; i<argc; i++)
            ocrPrintf("    ARGV[%d] is \"%s\"\n", i, argv[i]);

        ocrPrintf("Command line flags:\n");
        ocrPrintf("    cmdLineDebug = %d\n", clVals.cmdLineDebug);
        ocrPrintf("    tstSwitchNodes = %d\n", clVals.tstSwitchNodes);
        ocrPrintf("    countNumCpus = %d\n", clVals.countNumCpus);
        ocrPrintf("    rdtscCpu = %d\n", clVals.rdtscCpu);
        ocrPrintf("    timeRdtsc = %d\n", clVals.timeRdtsc);
        ocrPrintf("    findMcdram = %d\n", clVals.findMcdram);
        if (clVals.numCpus == 0)
            ocrPrintf("    runCpus = not set\n");
        else {
            ocrPrintf("    runCpus = ");
            for(i=0; i<clVals.numCpus; i++) {
                if (i > 0)
                    ocrPrintf(", %d", clVals.runCpus[i]);
                else
                    ocrPrintf("%d", clVals.runCpus[i]);
            }
            ocrPrintf("\n");
        }
        ocrPrintf("    dramSize = %lu\n", clVals.dramSize);
        ocrPrintf("    dramWLoops = %d\n", clVals.dramWLoops);
        ocrPrintf("    dramRLoops = %d\n", clVals.dramRLoops);
        ocrPrintf("    dramRRLoops = %d\n", clVals.dramRLoopRand);
        ocrPrintf("    mcdramSize = %lu\n", clVals.mcdramSize);
        ocrPrintf("    mcdramWLoops = %d\n", clVals.mcdramWLoops);
        ocrPrintf("    mcdramRLoops = %d\n", clVals.mcdramRLoops);
        ocrPrintf("    mcdramRRLoops = %d\n", clVals.mcdramRLoopRand);
        ocrPrintf("    cacheLineLoops = %lu\n", clVals.cacheLineLoops);
        ocrPrintf("    cacheLineOffset = %lu\n", clVals.cacheLineOffset);
        ocrPrintf("    cacheAllocLoops = %lu\n", clVals.cacheAllocLoops);
        ocrPrintf("    cacheLineMinLarge = %lu\n", clVals.cacheLineMinLarge);
        ocrPrintf("    cacheAllocLoops = %lu\n", clVals.cacheAllocLoops);
    }

    if (numActionOps == 0) {
        ocrPrintf("There were no action operations specified...\n");
        sendHelp();
        return -1;
    }

    return retVal;
}
