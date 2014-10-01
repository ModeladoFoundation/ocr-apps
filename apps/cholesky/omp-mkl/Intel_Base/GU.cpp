/*
Intel Base Kernel Library: General Utilities (GU)

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#include "GU.h"
#ifdef _WIN32
#include <Windows.h>
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
#else
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#endif
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <mkl.h>
using namespace std;

namespace Intel_Base
{
    /*
     * Return current time in seconds since Jan 1, 1970
     */
    double TimeNowInSeconds()
    {
#ifdef _WIN32
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft); // Resolution is 15625 micro-seconds
        unsigned long long tt = ft.dwHighDateTime;
        tt <<= 32;
        tt |= ft.dwLowDateTime;
        tt /= 10; // Convert to micro-seconds
        tt -= DELTA_EPOCH_IN_MICROSECS;
        return tt / 1000000.0;
#else
        struct timespec time_now;
        if (clock_gettime(CLOCK_MONOTONIC_RAW, &time_now) == 0)
        {
            return time_now.tv_sec + time_now.tv_nsec / 1.0e9;
        }
        else return 0; // Error
#endif
    }

    /*
     * Return current time in clock ticks
     */
    unsigned long long TimeNowInClocks()
    {
        MKL_UINT64 clocks;
        mkl_get_cpu_clocks(&clocks);
        return clocks;
    }

    /*
     * Return current CPU frequency in Hertz
     */
    double FreqNowInHertz()
    {
        return mkl_get_cpu_frequency() * 1e9;
    }

    /*
     * Return maximum CPU frequency in Hertz
     */
    double FreqMaxInHertz()
    {
        return mkl_get_max_cpu_frequency() * 1e9;
    }

    /*
     * Suspend current thread for a number of seconds (may be milliseconds e.g. 0.010 is 10ms).  Returns true if not interrupted.
     */
    bool SleepSeconds(double seconds)
    {
#ifdef _WIN32
        Sleep((DWORD)(seconds * 1000));
        return true;
#else
        timespec ts;
        ts.tv_sec = (time_t) seconds;
        ts.tv_nsec = (long)((seconds - ts.tv_sec) * 1e9);
        return clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL) == 0;
#endif
    }

    /*
     * Return percentage speedup of x vs. y
     */
    double PercentSpeedup(double x, double y)
    {
        const double epsilon = 1e-11;
        return fabs(y) < epsilon ? 100.0 / epsilon : (x / y - 1) * 100;
    }

    /*
     * Return PASS if test true, else FAIL
     */
    string PassFail(const bool &test)
    {
        return test ? string("PASS") : string("FAIL");
    }

    /*
     * Return 's' if num gt 1, else ''
     */
    string PluralS(const int &num)
    {
        return num > 1 ? string("s") : string("");
    }

    /*
     * Return current working directory
     */
    string DirCurrent()
    {
#ifdef _WIN32
        unsigned long cd_len = GetCurrentDirectoryA(0, NULL);
        char *cd = new char[cd_len + 1];
        GetCurrentDirectoryA(cd_len, cd);
        return cd;
#else
        return get_current_dir_name();
#endif
    }

    /*
    * Return true if current directory changed to dirName
    */
    bool DirChange(const string &dirName)
    {
        if (dirName.empty()) return false;
#ifdef _WIN32
        return SetCurrentDirectoryA(dirName.c_str());
#else
        return chdir(dirName.c_str()) == 0;
#endif
    }

    /*
    * Return true if dirName exists
    */
    bool DirExists(const string &dirName)
    {
        if (dirName.empty()) return false;
#ifdef _WIN32
        DWORD attribs = GetFileAttributesA(dirName.c_str());
        return attribs != INVALID_FILE_ATTRIBUTES && attribs & FILE_ATTRIBUTE_DIRECTORY;
#else
        struct stat ds;
        return stat(dirName.c_str(), &ds) == 0 && (ds.st_mode & S_IFMT) == S_IFDIR;
#endif
    }

    /*
    * Return true if dirName created
    */
    bool DirCreate(const string &dirName)
    {
        if (dirName.empty()) return false;
#ifdef _WIN32
        return CreateDirectoryA(dirName.c_str(), NULL);
#else
        return mkdir(dirName.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0;
#endif
    }

    /*
    * Return true if dirName deleted
    */
    bool DirDelete(const string &dirName)
    {
        if (dirName.empty()) return false;
#ifdef _WIN32
        return RemoveDirectoryA(dirName.c_str());
#else
        return rmdir(dirName.c_str()) == 0;
#endif
    }

    /*
    * Return true if filename exists
    */
    bool FileExists(const string &fileName)
    {
        if (fileName.empty()) return false;
        ifstream in(fileName.c_str(), ifstream::in);
        bool exists = in.is_open();
        in.close();
        return exists;
    }

    /*
    * Return true if filename exists
    */
    unsigned long long FileSize(const string &fileName)
    {
        if (fileName.empty()) return 0;
#ifdef _WIN32
        WIN32_FIND_DATA fd;
        HANDLE fh = FindFirstFile(fileName.c_str(), &fd);
        if (fh != INVALID_HANDLE_VALUE)
        {
            FindClose(fh);
            return fd.nFileSizeHigh * (MAXDWORD + 1) + fd.nFileSizeLow;
        }
        else return 0;
#else
        struct stat fs;
        return stat(fileName.c_str(), &fs) == 0 ? fs.st_size : 0;
#endif
    }

    /*
    * Return true if filename deleted
    */
    bool FileDelete(const string &fileName)
    {
        return remove(fileName.c_str()) == 0;
    }

    /*
    * Return true if filename renamed (can be used for a move if on same filesystem)
    */
    bool FileRename(const string &oldFileName, const string &newFileName)
    {
        return rename(oldFileName.c_str(), newFileName.c_str()) == 0;
    }

    /*
    * Print message to cerr with 'Error in source: ' prefix then exit the running program
    */
    void ErrorExit(const string &source, const string &message)
    {
        if (source.empty())
            cerr << "Error: " << message << endl;
        else
            cerr << "Error in " << source << ": " << message << endl;
        exit(1);
    }

    /*
     * Build Command, checking for existence of path/file.  Return empty string if does not exist or on error.
     */
    string BuildCommand(const string &path, const string &file, const bool &coutLog /*= false*/)
    {
        string command;
        if (path.empty())
        {
            if (Intel_Base::FileExists(file))
                command = file;
            else
                if (coutLog) cout << "BuildCommand: [" << path << "] does not exist" << endl;
        }
        else
        {
            if (Intel_Base::DirExists(path))
            {
                string file_path = path[path.size() - 1] == '/' ? path + file : path + "/" + file;
                if (Intel_Base::FileExists(file_path))
                    command = file_path;
                else
                    if (coutLog) cout << "BuildCommand: [" << file_path << "] does not exist" << endl;
            }
            else if (coutLog) cout << "BuildCommand: [" << path << "] does not exist" << endl;
        }
        return command;
    }

    /*
    * Build an error string that describes the not equals argument (i.e. argValue != argGoal)
    */
    string BuildArgErrNE(const string &argName, const int &argValue, const string &goalName, const int &goalValue)
    {
        ostringstream oss;
        oss << "Argument " << argName << " " << argValue << " does not equal " << goalName << " " << goalValue;
        return oss.str();
    }

    /*
    * Build an error string that describes the greater argument (i.e. argValue >= argGoal)
    */
    string BuildArgErrGE(const string &argName, const int &argValue, const string &goalName, const int &goalValue)
    {
        ostringstream oss;
        oss << "Argument " << argName << " " << argValue << " >= " << goalName << " " << goalValue << ", and it should be less than";
        return oss.str();
    }

    /*
    * Build an error string that describes the out of bounds argument (i.e. argValue not in [argMin, argMax])
    */
    string BuildArgErrNotIn(const string &argName, const int &argValue, const int &argMin, const int &argMax)
    {
        ostringstream oss;
        oss << "Argument " << argName << " " << argValue << " is out of bounds, not in [" << argMin << ", " << argMax << "]";
        return oss.str();
    }

    /*
     * Replace find_substr in value string with replace_substr
     */
    string ReplaceAllSubstr(string value, string find_substr, string replace_substr)
    {
        size_t position = 0;
        for (position = value.find(find_substr); position != string::npos; position = value.find(find_substr, position))
        {
            value.replace(position, find_substr.length(), replace_substr);
        }
        return value;
    }

    /*
    * Remove leading and trailing whitespace, leaving whitespace in the middle alone (e.g. '  Hi guys ! \t ' returns 'Hi guys !')
    */
    string TrimStr(const string &value, const string &whitespacechars /*= " \t"*/)
    {
        string::size_type str_begin = value.find_first_not_of(whitespacechars);
        const string::size_type str_end = value.find_last_not_of(whitespacechars);
        if (str_begin == string::npos) str_begin = 0;
        if (str_end == string::npos) return value.substr(str_begin);
        else
        {
            const string::size_type str_range = str_end - str_begin + 1;
            return value.substr(str_begin, str_range);
        }
    }

    /*
    * Reduce string value by trimming leading and trailing whitespacechars, then replacing internal whitespacechars with fill
    */
    string ReduceStr(const string &value, const string &fill /*= " "*/, const string &whitespacechars /*= " \t"*/)
    {
        string str = TrimStr(value);
        string::size_type str_begin = str.find_first_of(whitespacechars);
        while (str_begin != string::npos)
        {
            const string::size_type str_end = str.find_first_not_of(whitespacechars, str_begin),
                str_range = str_end - str_begin;
            str.replace(str_begin, str_range, fill);
            str_begin = str.find_first_of(whitespacechars, str_begin + fill.length());
        }
        return str;
    }

    // Execute system command using popen and return its stdout as a string, optionally removing endlines if removeCR is true
    string ExecSysCommand(const string &command, const bool &removeCR /*= false*/)
    {
        string result;
#ifdef _WIN32
        throw runtime_error("ExecSysCommand: Not implemented on Windows yet");
#else
        FILE* fp;
        fflush(NULL);
        fp = popen(command.c_str(), "r");
        if (fp == NULL)
        {
            cerr << "ExecSysCommand: Can not execute command:\n  " << command << endl;
        }
        else
        {
            char* line = NULL;
            size_t len = 0;
            while (getline(&line, &len, fp) != -1)
            {
                result += line;
                free(line);
                line = NULL;
            }
            fflush(fp);
            if (pclose(fp) != 0) cerr << "ExecSysCommand: Cannot close command stream." << endl;
            if  (removeCR)
            {
                for (int position = result.find("\n"); position != string::npos; position = result.find("\n", position))
                {
                    result.replace(position, 1, "");
                }
            }
        }
#endif
        return result;
    }

    // Execute command in operating system, returning true on success
    bool RunSysCommand(const string &command, const bool &coutCommand /*= true*/, const unsigned int &waitSeconds /*= 88*/)
    {
        if (coutCommand) cout << "Executing [" << command << "]" << endl;
        if (command.empty()) return false;
#ifdef _WIN32
        STARTUPINFO startup_info;
        PROCESS_INFORMATION process_info;
        ZeroMemory(&startup_info, sizeof(startup_info)); startup_info.cb = sizeof(startup_info);
        ZeroMemory(&process_info, sizeof(process_info));
        char command_line[32768];
        strcpy_s(command_line, 32768, command.c_str());
        if (CreateProcessA(NULL, command_line, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startup_info, &process_info))
        {
            WaitForSingleObject(process_info.hProcess, waitSeconds * 1000);
            CloseHandle(process_info.hProcess);
            CloseHandle(process_info.hThread);
            return true;
        }
        else return false;
#else
        return system(command.c_str()) == 0;
#endif
    }

#ifdef _WIN32
    // Get version information for an application or DLL
    string GetBinaryVersion(string binaryPath)
    {
        string version;
        WORD MajorVersion, MinorVersion, BuildNumber, RevisionNumber;
        DWORD dwHandle, dwLen;
        UINT BufLen;
        LPTSTR lpData;
        VS_FIXEDFILEINFO *pFileInfo;
        dwLen = GetFileVersionInfoSizeA(binaryPath.c_str(), &dwHandle);
        if (dwLen)
        {
            lpData = (LPTSTR)malloc(dwLen);
            if (lpData)
            {
                if (GetFileVersionInfoA(binaryPath.c_str(), dwHandle, dwLen, lpData))
                {
                    if (VerQueryValueA(lpData, "\\", (LPVOID *)&pFileInfo, (PUINT)&BufLen))
                    {
                        MajorVersion = HIWORD(pFileInfo->dwFileVersionMS);
                        MinorVersion = LOWORD(pFileInfo->dwFileVersionMS);
                        BuildNumber = HIWORD(pFileInfo->dwFileVersionLS);
                        RevisionNumber = LOWORD(pFileInfo->dwFileVersionLS);
                        ostringstream ss;
                        ss << MajorVersion << "." << MinorVersion << "." << BuildNumber << "." << RevisionNumber;
                        version = ss.str();
                    }
                }
                free(lpData);
            }
        }
        return version;
    }
#endif

    // Safely get environment variable, covering case where could not be get at all because it is undefined
    string SafeGetEnv(string name)
    {
        if (name.empty()) return "";
        else
        {
            char *value = getenv(name.c_str());
            return value == NULL ? "" : value;
        }
    }

    // Safely Set environment variable, where overwrite true means replace existing value.  Returns true on success.
    bool SafeSetEnv(const string &name, const string &value, const bool &overwrite /*= true*/)
    {
        if (name.empty()) return false;
#ifdef _WIN32
        if (!overwrite)
        {
            size_t size = 0;
            int errcode = getenv_s(&size, NULL, 0, name.c_str());
            if (errcode || size) return false; // Variable exists, so fail before resetting it
        }
        return _putenv_s(name.c_str(), value.c_str()) == 0;
#else
        return setenv(name.c_str(), value.c_str(), overwrite ? 1 : 0) == 0;
#endif
    }

    // Safely UnSet environment variable.  Returns true on success.
    bool SafeUnSetEnv(const string &name)
    {
        if (name.empty()) return false;
#ifdef _WIN32
        return _putenv_s(name.c_str(), "") == 0;
#else
        return unsetenv(name.c_str()) == 0;
#endif
    }

    // Return the factorial N!
    double Factorial(const unsigned int &N)
    {
        double x = 1;
        if (N > 1)
        {
            switch (N)
            {
            case 2: x = 2; break;
            case 3: x = 6; break;
            case 4: x = 24; break;
            case 5: x = 120; break;
            case 6: x = 720; break;
            case 7: x = 5040; break;
            case 8: x = 40320; break;
            case 9: x = 362880; break;
            case 10: x = 3628800; break;
            case 11: x = 39916800; break;
            case 12: x = 479001600; break;
            case 13: x = 6227020800; break;
            case 14: x = 87178291200; break;
            case 15: x = 1307674368000; break;
            default:
                x = 20922789888000;
                for (unsigned int i = 17; i <= N; ++i)
                {
                    x *= i;
                }
                break;
            }
        }
        return x;
    }

    // Return true if Windows or Linux, else false for endianess on this host
    bool IsLittleEndian()
    {
#ifdef _WIN32
        return true;
#else
#ifdef __linux
        return true;
#else
        return false;
#endif
#endif
    }

    // Perform a byte swap on value
    short ByteSwap(const short &value)
    {
        return (((value >> 8) & 0xff) | ((value & 0xff) << 8));
    }

    // Perform a byte swap on value
    unsigned short ByteSwap(const unsigned short &value)
    {
        return (((value >> 8) & 0xff) | ((value & 0xff) << 8));
    }

    // Perform a byte swap on value
    int ByteSwap(const int &value)
    {
        return (
            ((value & 0xff000000) >> 24) |
            ((value & 0x00ff0000) >> 8) |
            ((value & 0x0000ff00) << 8) |
            ((value & 0x000000ff) << 24));
    }

    // Perform a byte swap on value
    unsigned int ByteSwap(const unsigned int &value)
    {
        return (
            ((value & 0xff000000) >> 24) |
            ((value & 0x00ff0000) >> 8) |
            ((value & 0x0000ff00) << 8) |
            ((value & 0x000000ff) << 24));
    }

    // Perform a byte swap on value
    long ByteSwap(const long &value)
    {
        if (sizeof(value) == 8)
            return (
            ((value & 0xff00000000000000ull) >> 56) |
            ((value & 0x00ff000000000000ull) >> 40) |
            ((value & 0x0000ff0000000000ull) >> 24) |
            ((value & 0x000000ff00000000ull) >> 8) |
            ((value & 0x00000000ff000000ull) << 8) |
            ((value & 0x0000000000ff0000ull) << 24) |
            ((value & 0x000000000000ff00ull) << 40) |
            ((value & 0x00000000000000ffull) << 56));
        else
            return (
            ((value & 0xff000000) >> 24) |
            ((value & 0x00ff0000) >> 8) |
            ((value & 0x0000ff00) << 8) |
            ((value & 0x000000ff) << 24));
    }

    // Perform a byte swap on value
    unsigned long ByteSwap(const unsigned long &value)
    {
        if (sizeof(value) == 8)
            return (
            ((value & 0xff00000000000000ull) >> 56) |
            ((value & 0x00ff000000000000ull) >> 40) |
            ((value & 0x0000ff0000000000ull) >> 24) |
            ((value & 0x000000ff00000000ull) >> 8) |
            ((value & 0x00000000ff000000ull) << 8) |
            ((value & 0x0000000000ff0000ull) << 24) |
            ((value & 0x000000000000ff00ull) << 40) |
            ((value & 0x00000000000000ffull) << 56));
        else
            return(
            ((value & 0xff000000) >> 24) |
            ((value & 0x00ff0000) >> 8) |
            ((value & 0x0000ff00) << 8) |
            ((value & 0x000000ff) << 24));
    }

    // Perform a byte swap on value
    long long ByteSwap(const long long &value)
    {
        return (
            ((value & 0xff00000000000000ull) >> 56) |
            ((value & 0x00ff000000000000ull) >> 40) |
            ((value & 0x0000ff0000000000ull) >> 24) |
            ((value & 0x000000ff00000000ull) >> 8) |
            ((value & 0x00000000ff000000ull) << 8) |
            ((value & 0x0000000000ff0000ull) << 24) |
            ((value & 0x000000000000ff00ull) << 40) |
            ((value & 0x00000000000000ffull) << 56));
    }

    // Perform a byte swap on value
    unsigned long long ByteSwap(const unsigned long long &value)
    {
        return (
            ((value & 0xff00000000000000ull) >> 56) |
            ((value & 0x00ff000000000000ull) >> 40) |
            ((value & 0x0000ff0000000000ull) >> 24) |
            ((value & 0x000000ff00000000ull) >> 8) |
            ((value & 0x00000000ff000000ull) << 8) |
            ((value & 0x0000000000ff0000ull) << 24) |
            ((value & 0x000000000000ff00ull) << 40) |
            ((value & 0x00000000000000ffull) << 56));
    }

    // Perform a byte swap on value
    float ByteSwap(const float &value)
    {
        unsigned int val = ByteSwap(*(unsigned int*)&value);
        return *(float *)&val;
    }

    // Perform a byte swap on value
    double ByteSwap(const double &value)
    {
        unsigned long long val = ByteSwap(*(unsigned long long*)&value);
        return *(double *)&val;
    }

}