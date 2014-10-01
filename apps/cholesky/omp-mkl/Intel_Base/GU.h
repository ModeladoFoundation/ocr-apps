/*
Intel Base Kernel Library: General Utilities (GU)

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#pragma once
#include <Intel_Base.h>
#include <string>
using namespace std;

namespace Intel_Base
{
    double TimeNowInSeconds();
    unsigned long long TimeNowInClocks();
    double FreqNowInHertz();
    double FreqMaxInHertz();
    bool SleepSeconds(double seconds);
    double PercentSpeedup(double x, double y);

    string PassFail(const bool &test);
    string PluralS(const int &num);

    string DirCurrent();
    bool DirChange(const string &dirName);
    bool DirExists(const string &dirName);
    bool DirCreate(const string &dirName);
    bool DirDelete(const string &dirName);

    bool FileExists(const string &fileName);
    unsigned long long FileSize(const string &fileName);
    bool FileDelete(const string &fileName);
    bool FileRename(const string &oldFileName, const string &newFileName);

    void ErrorExit(const string &source, const string &message);
    string BuildCommand(const string &path, const string &file, const bool &coutLog = false);
    string BuildArgErrNE(const string &argName, const int &argValue, const string &goalName, const int &goalValue);
    string BuildArgErrGE(const string &argName, const int &argValue, const string &goalName, const int &goalValue);
    string BuildArgErrNotIn(const string &argName, const int &argValue, const int &argMin, const int &argMax);
    string ReplaceAllSubstr(string value, string find_substr, string replace_substr);
    string TrimStr(const string &value, const string &whitespacechars = " \t");
    string ReduceStr(const string &value, const string &fill = " ", const string &whitespacechars = " \t");
    string ExecSysCommand(const string &command, const bool &removeCR = false);
    bool RunSysCommand(const string &command, const bool &coutCommand = true, const unsigned int &waitSeconds = 88);
    string GetBinaryVersion(string binaryPath);

    string SafeGetEnv(string name);
    bool SafeSetEnv(const string &name, const string &value, const bool &overwrite = true);
    bool SafeUnSetEnv(const string &name);

    double Factorial(const unsigned int &N); // Return the factorial N!

    bool IsLittleEndian();
    short ByteSwap(const short &value);
    unsigned short ByteSwap(const unsigned short &value);
    int ByteSwap(const int &value);
    unsigned int ByteSwap(const unsigned int &value);
    long ByteSwap(const long &value);
    unsigned long ByteSwap(const unsigned long &value);
    long long ByteSwap(const long long &value);
    unsigned long long ByteSwap(const unsigned long long &value);
    float ByteSwap(const float &value);
    double ByteSwap(const double &value);
}
