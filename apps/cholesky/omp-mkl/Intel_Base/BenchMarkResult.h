/*
Intel Base Kernel Library: BenchMarkResult struct

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#pragma once
#include "GU.h"
#include "BinaryFile.h"
#include <string.h>
#include <string>
#include <sstream>
using namespace std;

#define BENCHMARKRESULT_NAME_MAXLEN 33

// Binary Intel_BenchMark result struct 
struct BenchMarkResult
{
    int NumTasks = 0;           // Number of parallel tasks for tiled decomposition, else 0 for number of sockets
    int NumThreadsPerTask = 0;  // Number of parallel threads per task, else 0 for number of cores/socket
    int DataSize = 0;           // Number of rows and columns in benchmark data
    int TileSize = 0;           // Number of rows and columns in tile (must be such that DataSize % TileSize == 0)
    int Runs = 1;               // Number of times to run benchmark (1 default)
    double NonTiledSec = -1;    // Number of average seconds (total sec / Runs) for Non-Tiled benchmark
    double TiledSec = -1;       // Number of average seconds (total sec / Runs) for Tiled benchmark
    double NonTiledErr = -1;    // Non-Tiled benchmark error from expected result
    double TiledErr = -1;       // Tiled benchmark error from expected result
    float SpeedUpNtVT = -1;    // Non-Tiled vs. Tiled percentage speedup using NonTiledSec and TiledSec
    char Name[BENCHMARKRESULT_NAME_MAXLEN]; // Name of benchmark

    /*
    * Constructor
    */
    BenchMarkResult();

    /*
    * Constructor
    * @param name               Benchmark Name
    * @param numTasks           Number of parallel tasks for tiled decomposition, else 0 for number of sockets
    * @param numThreadsPerTask  Number of parallel threads per task, else 0 for number of cores/socket
    * @param dataSize           Number of rows and columns in benchmark data
    * @param tileSize           Number of rows and columns in tile (must be such that DataSize % TileSize == 0)
    * @param runs               Number of times to benchmark run
    * @param nonTiledSec        Number of average seconds (total sec / Runs) for Non-Tiled benchmark, else -1 if not defined
    * @param tiledSec           Number of average seconds (total sec / Runs) for Tiled benchmark, else -1 if not defined
    * @param nonTiledErr        Non-Tiled benchmark error from expected result, else -1 if not defined
    * @param tiledErr           Tiled benchmark error from expected result, else -1 if not defined
    */
    BenchMarkResult(const string &name, const int &numTasks, const int &numThreadsPerTask, const int &dataSize = 0, const int &tileSize = 0,
        const int &runs = 1, const double &nonTiledSec = -1, const double &tiledSec = -1, const double &nonTiledErr = -1, const double &tiledErr = -1);

    /*
    * Update SpeedUpNtVT by setting it to Intel_Base::PercentSpeedup(NonTiledSec, TiledSec);
    */
    float UpdateSpeedUpNtVT();

    /*
    * Human-readable version of this
    * @param dl                 Debugging output level {0: none, 1: summary, 2: detail, 3: verbose} (1 default)
    * @param rtW                Runtime seconds display width (default 11)
    * @param rtP                Runtime second display precision (default 0)
    * @param errW               Error display width (default 11)
    * @param errP               Error display precision (default 0)
    * @return                   Human readable and formatted version of the properties of this
    */
    string ToString(const int &dl = 0, const int &rtW = 11, const int &rtP = 0, const int &errW = 11, const int &errP = 0) const;

    /*
    * Convert this to a separator separated list of values
    * @param separator          Character between values of this
    */
    string ToString(const string &separator = " ") const;

    /*
    * Write this to binary fileName with optional append
    * @param fileName           Output binary file name
    * @param append             True to append a record to existing fileName (true default)
    * @param dl                 Debugging output level {0: none, 1: summary, 2: detail, 3: verbose} (1 default)
    * @return                   True upon successful file creation and record addition
    */
    bool WriteRecord(const string &fileName, const bool &append = true, const int &dl = 0) const;

    /*
    * Read contents of binFileName from previous WriteRecord calls, and output to separator separated value csvFileName
    * @param binFileName        Input binary file name from previous WriteRecord calls
    * @param txtFileName        Output separator separated value file name using ToString(",")
    * @param separator          Character between values of this
    * @param dl                 Debugging output level {0: none, 1: summary, 2: detail, 3: verbose} (1 default)
    * @return                   True upon successful file creation
    */
    static bool WriteTextFile(const string &binFileName, const string &txtFileName, const string &separator = ",", const int &dl = 0);
};

