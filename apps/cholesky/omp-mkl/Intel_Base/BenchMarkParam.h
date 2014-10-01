/*
Intel Base Kernel Library: BenchMarkParam struct

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#pragma once
#include <string.h>
#include <TileMatrix.h>
using namespace std;

/*
* BenchMark Parameters struct
*/
struct BenchMarkParam
{
    string Name;                // BenchMark Name
    int DataSize = 0;           // Number of rows and columns in benchmark data
    int TileSize = 0;           // Number of rows and columns in tile (must be such that DataSize % TileSize == 0)
    int NumTasks = 0;           // Number of parallel tasks for tiled decomposition, else 0 for number of sockets
    int NumThreadsPerTask = 0;  // Number of parallel threads per task, else 0 for number of cores/socket
    int Runs = 2;               // Number of times to run benchmark (2 default)
    int DL = 0;                 // Debugging output level {0: none, 1: summary, 2: detail, 3: verbose} (0 default)
    int DLMaxSize = 4000;       // Maximum matrix size for debug level output (3500 default)
    int RtW = 11;               // Runtime seconds display width (default 12)
    int RtP = 0;                // Runtime second display precision (default 0)
    int MatW = 6;               // Matrix value display width (default 12)
    int MatP = 0;               // Matrix value display precision (default 0)
    string InFile;              // Input Matrix file, input to benchmark (default '')
    string OutFile;             // Output Matrix file, result of benchmark (default '')
    string AnsFile;             // Answer Matrix file, precalculated for corresponding InFile should match OutFile (default '')
    string RepFile;             // Report file, contains BenchMarkResult records from multiple runs (default '')
    string TxtFile;             // Output text comma separated value report file name, reads RepFile to generate (default '')
    Matrix A;                   // Input Matrix
    TileMatrix TA;              // Input TileMatrix

    /*
    * @param numTasks           Number of parallel tasks for tiled decomposition, else 0 for number of sockets
    * @param numThreadsPerTask  Number of parallel threads per task, else 0 for number of cores/socket
    * @param dataSize           Number of rows and columns in benchmark data
    * @param tileSize           Number of rows and columns in tile (must be such that DataSize % TileSize == 0)
    * @param runs               Number of times to run benchmark (2 default)
    * @param dl                 Debugging output level {0: none, 1: summary, 2: detail, 3: verbose} (0 default)
    * @param dlMaxSize          Maximum matrix size for debug level output (4000 default)
    * @param rtW                Runtime seconds display width (default 11)
    * @param rtP                Runtime second display precision (default 0)
    * @param matW               Matrix value display width (default 7)
    * @param matP               Matrix value display precision (default 0)
    * @param inFile             Input Matrix file, input to benchmark (default '')
    * @param outFile            Output Matrix file, result of benchmark (default '')
    * @param ansFile            Answer Matrix file, precalculated for corresponding InFile should match OutFile (default '')
    * @param repFile            Output binary report file name, appends record if exists (default '')
    * @param txtFile            Output text comma separated value report file name, reads RepFile to generate (default '')
    */
    BenchMarkParam(const string &name = "", const int &numTasks = 0, const int &numThreadsPerTask = 0, const int &dataSize = 0, const int &tileSize = 0, const int &runs = 2,
        const int &dl = 0, const int &dlMaxSize = 4000, const int &rtW = 11, const int &rtP = 0, const int &matW = 7, const int &matP = 0,
        const string &inFile = "", const string &outFile = "", const string &ansFile = "", const string &repFile = "", const string &txtFile = "");

    /*
    * Checks InFile for existence and sets default for AnsFile.  Returns false if should exit.
    * @return                   True on success, which means InFile is non-empty
    */
    bool CheckFiles();

    /*
    * Human-readable version of this
    */
    string ToString() const;
};
