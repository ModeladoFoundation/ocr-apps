#pragma once
#include <Intel_Base.h>
#include <string>
#include <map>
using namespace std;

#ifndef _WIN32
#include <vector>
#include <boost/tuple/tuple.hpp>
typedef vector<boost::tuple<string,string> > str_str_tuple_list;
typedef vector<boost::tuple<string,string,string,string> > str_str_str_str_tuple_list;
typedef vector<boost::tuple<int,string,string,float,int,int,int,int> > elem_x_8_tuple_list;
#endif

/*
* Hardware and Operating System Information class, with caching of results for speed
*/
class SysInfo
{
private:
    bool _initialized = false;
    string _hostname, _dns_suffix, _os_name, _os_inst_type, _os_version, _cpu_vendor, _cpu_description;
    int _cpu_num_sockets, _cpu_cores_per_socket, _cpu_threads_per_core, _cpu_total_cores, _cpu_total_threads;
    int _mem_page_size;
    map<int, int> _mem_cache_count, _mem_cache_size;
    float _cpu_act_frequency, _cpu_max_frequency;
    void Initialize();
#ifndef _WIN32
    str_str_tuple_list HostDiscov();
#endif

public:
    SysInfo();
    ~SysInfo();

#ifndef _WIN32
    str_str_tuple_list HardwareDiscov(string, vector<string>); // Returns tuple list of values for parameters in HWlist
    elem_x_8_tuple_list GroupHardwareDiscov(); // Organize parameters for each HW component
    double ConvertMemKBtoMiB(string); // Converts Linux /proc/meminfo string kB values to double MiB values
//    string TrimStr(const string&, const string& /*whitespace = " \t"*/); // Trim string whitespace and tabs
//    string ReduceStr(const string&, const string& /*fill = " "*/, const string& /*whitespace = " \t"*/); // Reduce excess whitespace within string
    str_str_str_str_tuple_list FabricDiscov(); // Returns tuple list of all detected fabric names, types, switches, and status
#endif
    string HostName() const; // Host Name (e.g. sam, which is in sam.jf.intel.com)
    string DnsSuffix() const; // Domain Name System (DNS) suffix (e.g. jf.intel.com)
    string OsName() const; // Operating System Name
    string OsInstType() const; // Operating System Instruction Type (e.g. x86, x86_64)
    string OsVersion() const; // Operating System Version string
    string CpuVendor() const; // CPU Vendor ID (e.g. GenuineIntel)
    string CpuDescription() const; // CPU Description (e.g. Intel(R) Core(TM) i5-2520M CPU @ 2.50GHz)
    int CpuNumSockets() const; // Number of CPU NUMA nodes
    int CpuCoresPerSocket() const; // Number of processing cores per NUMA node
    int CpuThreadsPerCore() const; // Number of threads that run simultantoeously per core (e.g. 2 with hyper-threading)
    int CpuTotalCores() const; // CpuNumSockets() * CpuCoresPerSocket()
    int CpuTotalThreads() const; // CpuTotalCores() * CpuThreadsPerCore()
    int MemPageSize() const; // Memory Page Size in bytes
    const map<int, int> &MemCacheCount() const; // Cache count for each level, which is the index, so L1 cache count is v[1]
    const map<int, int> &MemCacheSize() const; // Cache size in bytes for each level, which is the index, so L1 cache size is v[1]
    float CpuActFrequency() const; // CPU Actual Frequency
    float CpuMaxFrequency() const; // CPU Maximum Frequency
    string ToString() const; // String version of the attributes of this
};
