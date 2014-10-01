/*
Intel Base Kernel Library: BenchMarkParam struct

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#include "BenchMarkParam.h"
#include "GU.h"
#include <iostream>
#include <string>
#include <sstream>
using namespace std;
using namespace Intel_Base;

/*
* @param numTasks           Number of parallel tasks for tiled decomposition, else 0 for number of sockets
* @param numThreadsPerTask  Number of parallel threads per task, else 0 for number of cores/socket
* @param dataSize           Number of rows and columns in benchmark data
* @param tileSize           Number of rows and columns in tile (must be such that DataSize % TileSize == 0)
* @param runs               Number of times to run decomposition
* @param dl                 Debugging output level {0: none, 1: summary, 2: detail, 3: verbose}
* @param dlMaxSize          Maximum matrix size for debug level output
* @param rtW                Runtime seconds display width
* @param rtP                Runtime second display precision
* @param matW               Matrix value display width
* @param matP               Matrix value display precision
* @param inFile             Input Matrix file, input to benchmark
* @param outFile            Output Matrix file, result of benchmark
* @param ansFile            Answer Matrix file, precalculated for corresponding InFile should match OutFile
* @param repFile            Output binary report file name, appends record if exists
* @param txtFile            Output text comma separated value report file name, reads RepFile to generate
*/
BenchMarkParam::BenchMarkParam(const string &name, const int &numTasks, const int &numThreadsPerTask, const int &dataSize, const int &tileSize, const int &runs,
    const int &dl, const int &dlMaxSize, const int &rtW, const int &rtP, const int &matW, const int &matP,
    const string &inFile, const string &outFile, const string &ansFile, const string &repFile, const string &txtFile)
{
    if (!name.empty()) Name = name;
    NumTasks = numTasks;
    NumThreadsPerTask = numThreadsPerTask;
    DataSize = dataSize;
    TileSize = tileSize;
    Runs = runs;
    DL = dl;
    DLMaxSize = dlMaxSize;
    RtW = rtW;
    RtP = rtP;
    MatW = matW;
    MatP = matP;
    if (!inFile.empty()) InFile = inFile;
    if (!outFile.empty()) OutFile = outFile;
    if (!ansFile.empty()) AnsFile = ansFile;
    if (!repFile.empty()) RepFile = repFile;
    if (!txtFile.empty()) TxtFile = txtFile;
}

/*
* Checks InFile for existence and sets default for AnsFile.  Returns false if should exit.
* @return                   True on success, which means InFile is non-empty
*/
bool BenchMarkParam::CheckFiles()
{
    if (!InFile.empty())
    {
        if (!FileExists(InFile))
        {
            cerr << "  Error: InFile [" << InFile << "] does not exist" << endl;
            InFile = "";
            return false;
        }
        if (AnsFile.empty() && InFile.find(".in") != string::npos)
        {
            string ans_temp = ReplaceAllSubstr(InFile, ".in", ".ans");
            if (FileExists(ans_temp)) AnsFile = ans_temp;
        }
    }
    return true;
}

/*
* Human-readable version of this
*/
string BenchMarkParam::ToString() const
{
    ostringstream ss;
    if (!Name.empty()) ss << "  " << Name;
    ss << "  Tasks " << NumTasks << "  Threads/Task " << NumThreadsPerTask
        << "  DataSize " << DataSize << "  TileSize " << TileSize
        << "  " << Runs << " Run" << PluralS(Runs) << "\n";
    if (DL > 1)
    {
        if (TA.size() > 0 && (TA.size() < DLMaxSize || DL > 2)) ss << TA.ToString("TA", MatW, MatP);
        else if (A.size() > 0 && (A.size() < DLMaxSize || DL > 2)) ss << A.ToString("A", MatW, MatP);
        else if (DL > 2 && !InFile.empty())  ss << "   InFile " << InFile << "\n";
    }
    else if (DL > 0)
    {
        if (TA.size() > 0 && TA.size() < DLMaxSize) ss << TA.ToStringSize("TA");
        else if (A.size() > 0 && A.size() < DLMaxSize) ss << A.ToStringSize("A");
        else if (DL > 2 && !InFile.empty())  ss << "   InFile " << InFile << "\n";
    }
    if (DL > 2)
    {
        if (!OutFile.empty()) ss << "  OutFile " << OutFile << "\n";
        if (!AnsFile.empty()) ss << "  AnsFile " << AnsFile << "\n";
        if (!RepFile.empty()) ss << "  RepFile " << RepFile << "\n";
        if (!TxtFile.empty()) ss << "  TxtFile " << TxtFile << "\n";
    }
    return ss.str();
}
