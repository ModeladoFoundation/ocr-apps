/*
Intel Base Kernel Library: BenchMarkReturn struct

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#pragma once
#include "BenchMarkResult.h"
#include "BenchMarkParam.h"
#include <string>
using namespace std;

/*
* BenchMark returns struct
*/
struct BenchMarkReturn
{
    BenchMarkResult R;   // Struct containing all the performance results
    Matrix B;            // Result from Benchmark operation on input matrix A
    TileMatrix TB;       // Result from Benchmark operation on input tiled matrix TA

    /*
    * Constructor
    * @param p                  Benchmark parameter struct
    */
    BenchMarkReturn(BenchMarkParam &p);

    ~BenchMarkReturn();

    /*
    * Human-readable version of this
    * @param p                  Benchmark parameter struct
    * @param passLimit          Benchmark validation test passing error threshold
    * @param ntVsTErr           Non-Tiled vs. Tiled Error (default -1)
    * @param errW               Error display width (default 11)
    * @param errP               Error display precision (default 0)
    */
    string ToString(const BenchMarkParam &p, const double &passLimit, const double &ntVsTErr = -1, const int &errW = 11, const int &errP = 0) const;
};
