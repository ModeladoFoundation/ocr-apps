/*
Intel Base Kernel Library: BenchMarkResult struct

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#include "BenchMarkResult.h"

/*
* Constructor
*/
BenchMarkResult::BenchMarkResult()
{
    Name[0] = '\0'; // Empty string
}

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
BenchMarkResult::BenchMarkResult(const string &name, const int &numTasks, const int &numThreadsPerTask, const int &dataSize, const int &tileSize,
    const int &runs, const double &nonTiledSec, const double &tiledSec, const double &nonTiledErr, const double &tiledErr)
{
    if (name.empty()) Name[0] = '\0'; // Empty string
    else strncpy(Name, name.c_str(), BENCHMARKRESULT_NAME_MAXLEN);

    NumTasks = numTasks;
    NumThreadsPerTask = numThreadsPerTask;
    DataSize = dataSize;
    TileSize = tileSize;
    Runs = runs;
    NonTiledSec = nonTiledSec;
    TiledSec = tiledSec;
    NonTiledErr = nonTiledErr;
    TiledErr = tiledErr;
    SpeedUpNtVT = Intel_Base::PercentSpeedup(NonTiledSec, TiledSec);
}

/*
* Update SpeedUpNtVT by setting it to Intel_Base::PercentSpeedup(NonTiledSec, TiledSec);
*/
float BenchMarkResult::UpdateSpeedUpNtVT()
{
    return SpeedUpNtVT = Intel_Base::PercentSpeedup(NonTiledSec, TiledSec);
}

/*
* Human-readable version of this
* @param dl                 Debugging output level {0: none, 1: summary, 2: detail, 3: verbose}
* @param rtW                Runtime seconds display width
* @param rtP                Runtime second display precision
* @param errW               Error display width
* @param errP               Error display precision
* @return                   Human readable and formatted version of the properties of this
*/
string BenchMarkResult::ToString(const int &dl, const int &rtW, const int &rtP, const int &errW, const int &errP) const
{
    ostringstream ss;
    if (NonTiledSec >= 0 || NonTiledErr >= 0)
    {
        ss << "  " << Name << "  Non-Tiled";
        ss << "  Runtime ";
        if (rtW > 0) ss.width(rtW);
        if (rtP > 0) ss.precision(rtP);
        ss << NonTiledSec;
        if (NonTiledErr >= 0)
        {
            ss << "  Error ";
            if (errW > 0) ss.width(errW);
            if (errP > 0) ss.precision(errP);
            ss << NonTiledErr;
        }
        ss << "\n";
    }
    if (TiledSec >= 0 || TiledErr >= 0)
    {
        ss << "  " << Name << "      Tiled";
        ss << "  Runtime ";
        if (rtW > 0) ss.width(rtW);
        if (rtP > 0) ss.precision(rtP);
        ss << TiledSec;
        if (TiledErr >= 0)
        {
            ss << "  Error ";
            if (errW > 0) ss.width(errW);
            if (errP > 0) ss.precision(errP);
            ss << TiledErr;
        }
        if (NonTiledSec >= 0) ss << "  SpeedUp " << SpeedUpNtVT << "%";
        ss << "\n";
    }
    if (dl > 2)
    {
        ss << "  " << Name << "  Tasks " << NumTasks << "  Threads/Task " << NumThreadsPerTask
            << "  DataSize " << DataSize << "  TileSize " << TileSize << "  " << Runs << " Run" << Intel_Base::PluralS(Runs) << "\n";
    }
    return ss.str();
}

/*
* Convert this to a separator separated list of values
* @param separator          Character between values of this
*/
string BenchMarkResult::ToString(const string &separator) const
{
    ostringstream os;
    os << Name << separator << NumTasks << separator << NumThreadsPerTask << separator
        << DataSize << separator << TileSize << separator << Runs << separator
        << NonTiledSec << separator << TiledSec << separator << NonTiledErr << separator << TiledErr
        << separator << SpeedUpNtVT;
    return os.str();
}

/*
* Write this to binary fileName with optional append
* @param fileName           Output binary file name
* @param append             True to append a record to existing fileName
* @param dl                 Debugging output level {0: none, 1: summary, 2: detail, 3: verbose}
* @return                   True upon successful file creation and record addition
*/
bool BenchMarkResult::WriteRecord(const string &fileName, const bool &append, const int &dl) const
{
    BinaryFile<BenchMarkResult> bf = BinaryFile<BenchMarkResult>(INTEL_BASE_KERNEL_VERSION, fileName);
    if (dl > 2) cout << "  BenchMarkResult  bf.HeaderSize " << bf.HeaderSize() << "  bf.RecordSize " << bf.RecordSize() << endl;
    if (append && Intel_Base::FileExists(fileName) && bf.OpenRead() && bf.ReadFooter() > 0)
    {
        // Will append to existing file
        if (bf.SeekWriteRecord(bf.RecordCount()))
        {
            if (bf.Write(*this) && bf.WriteFooter()) return true;
            else bf.Close();
        }
    }
    else if (bf.WriteHeader())
    {
        if (bf.Write(*this) && bf.WriteFooter()) return true;
        else bf.Close();
    }
    return false;
}

/*
* Read contents of binFileName from previous WriteRecord calls, and output to separator separated value csvFileName
* @param binFileName        Input binary file name from previous WriteRecord calls
* @param txtFileName        Output separator separated value file name using ToString(",")
* @param separator          Character between values of this
* @param dl                 Debugging output level {0: none, 1: summary, 2: detail, 3: verbose} (1 default)
* @return                   True upon successful file creation
*/
bool BenchMarkResult::WriteTextFile(const string &binFileName, const string &txtFileName, const string &separator, const int &dl)
{
    BinaryFile<BenchMarkResult> bf = BinaryFile<BenchMarkResult>(INTEL_BASE_KERNEL_VERSION, binFileName);
    vector<BenchMarkResult> recs = bf.ReadMany();
    if (recs.size() > 0)
    {
        fstream fs;
        fs.open(txtFileName, ios::out | ios::trunc);
        if (fs.is_open())
        {
            fs << "Name" << separator << "NumTasks" << separator << "NumThreadsPerTask" << separator
                << "DataSize" << separator << "TileSize" << separator << "Runs" << separator
                << "NonTiledSec" << separator << "TiledSec" << separator << "NonTiledErr" << separator << "TiledErr"
                << separator << "SpeedUpNtVT" << endl;
            if (dl > 2) cout << "BenchMarkResult file [" << binFileName << "] has " << recs.size() << " records" << endl;
            for (vector<BenchMarkResult>::iterator ri = recs.begin(); ri != recs.end(); ++ri)
            {
                const string &rs_string = ri->ToString(separator);
                fs << rs_string << endl;
                if (dl > 2) cout << rs_string << endl;
            }
            fs.close();
            return true;
        }
        else
        {
            cerr << "  Error: Could not open [" << txtFileName << "] for output" << endl;
            return false;
        }
    }
    else
    {
        cerr << "  Error: Found no records in [" << binFileName << "]" << endl;
        return false;
    }
}
