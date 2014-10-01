/*
Intel Base Kernel Library: System Information (SysInfo) class

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#include "SysInfo.h"
#include "GU.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <ctype.h>
#include <functional>
#endif
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;
using namespace Intel_Base;

SysInfo::SysInfo()
{
    Initialize();
}

SysInfo::~SysInfo()
{
}

#ifndef _WIN32
// This discovers all of the host information on linux OS.
// The values are returned as a vector of tuples. Each
// tuple includes two strings where string<0> is the identifier,
// string<1> is the corresponding result.
str_str_tuple_list SysInfo::HostDiscov()
{
    string hostname, version, platform, osname;
    vector<string> words;
    string dns_suffix = ExecSysCommand("hostname -d");
    if (dns_suffix.empty())
        dns_suffix = ExecSysCommand("hostname -i");
 
    string os_info = ExecSysCommand("uname -norp");
    boost::split(words, os_info, boost::is_any_of(" "), boost::token_compress_on);

    // Remove newline char from the strings
    dns_suffix.erase(remove(dns_suffix.begin(), dns_suffix.end(), '\n'), dns_suffix.end());
    hostname = words[0];
    hostname.erase(remove(hostname.begin(), hostname.end(), '\n'), hostname.end());
    version = words[1];
    version.erase(remove(version.begin(), version.end(), '\n'), version.end());
    platform = words[2];
    platform.erase(remove(platform.begin(), platform.end(), '\n'), platform.end());
    osname = words[3];
    osname.erase(remove(osname.begin(), osname.end(), '\n'), osname.end());
    
    str_str_tuple_list tl;
    tl.push_back(boost::tuple<string,string>("dns_suffix", dns_suffix));
    tl.push_back(boost::tuple<string,string>("hostname", hostname));
    tl.push_back(boost::tuple<string,string>("version", version));
    tl.push_back(boost::tuple<string,string>("platform", platform));
    tl.push_back(boost::tuple<string,string>("osname", osname));

    return tl;
}

// This uses boost::regex to sort through a command (file or alias)
// to identify hardware components on Linux, where it is primarily
// optimized for sorting through the /proc/cpuinfo and /proc/meminfo
// files. Input 1 is the command file or command and input 2 is a
// a list of the hardware components to identify and get values of.
// The return value is a vector of tuples, where the tuples each
// contain a pair of strings, where string<0> is the hardware
// identifier and string<1> is the result value. 
str_str_tuple_list SysInfo::HardwareDiscov(string cmdfile, vector<string> HWlist)
{
    int j=0;
    string line, os_cmd, os_file_template, os_file;
    str_str_tuple_list hw;
    char os_file_temp [] = "/tmp/sam_tmp_cmd.XXXXXX";
    
    if (!FileExists(cmdfile))
    {
	os_cmd = cmdfile;
	mkstemp(os_file_temp);
	os_file = os_file_temp;
	
	RunSysCommand((os_cmd + " > " + os_file).c_str(), false);
	cmdfile = os_file;
    }

    // Trim whitespace and unwanted characters from output
    string ws_trimmer;
    for (j=0; j<HWlist.size(); j++)
    {
	ifstream cmdline(cmdfile.c_str());
	if (cmdline.is_open())
	{
	    while (cmdline.good())
	    {
		getline(cmdline, line);
		boost::regex procInfo(HWlist[j]+"\\s*:\\s*(.*)");
		boost::cmatch results;
		if (regex_match(line.c_str(), results, procInfo))
		{
		    ws_trimmer = ReduceStr(results[1], " ", " \t");
		    hw.push_back( boost::tuple<string,string>(HWlist[j], ws_trimmer));
		}
	    }
	}
    }
    
    if (FileExists(os_file)) unlink(os_file_temp);
    return hw;
}

// This function groups and organizes the discovered HW
elem_x_8_tuple_list SysInfo::GroupHardwareDiscov()
{
    // Create a list of necessary HW and 
    // retrieve the values from /proc/cpuinfo
    vector<string> hwlist;
    hwlist.push_back("processor");
    hwlist.push_back("vendor_id");
//    hwlist.push_back("cpu family");
//    hwlist.push_back("model");
//    hwlist.push_back("stepping");
    hwlist.push_back("model name");
    hwlist.push_back("cpu MHz");
    hwlist.push_back("physical id");
    hwlist.push_back("siblings");
    hwlist.push_back("core id");
    hwlist.push_back("cpu cores");

    str_str_tuple_list discovHWTuple = HardwareDiscov("/proc/cpuinfo", hwlist);

    int j=0, k=0;
    int totalCounts; 
    elem_x_8_tuple_list hwInfoStore;

    totalCounts = discovHWTuple.size()/hwlist.size();

    // This will group together the relative info associated with a single detected CPU and convert data types
    for (j=0; j<totalCounts; j++)
    {
	hwInfoStore.push_back (boost::tuple<int,string,string,float,int,int,int,int>(0,"","",0,0,0,0,0));
	for (k=0; k < hwlist.size(); k++)
	{
	    if(discovHWTuple[j+k*totalCounts].get<0>()=="processor")
		hwInfoStore[j].get<0>()= std::atoi(discovHWTuple[j+k*totalCounts].get<1>().c_str());
	    if(discovHWTuple[j+k*totalCounts].get<0>()=="vendor_id")
		hwInfoStore[j].get<1>()=discovHWTuple[j+k*totalCounts].get<1>();
	    if(discovHWTuple[j+k*totalCounts].get<0>()=="model name")
		hwInfoStore[j].get<2>()=discovHWTuple[j+k*totalCounts].get<1>();
	    if(discovHWTuple[j+k*totalCounts].get<0>()=="cpu MHz")
		hwInfoStore[j].get<3>()=std::atof(discovHWTuple[j+k*totalCounts].get<1>().c_str());
	    if(discovHWTuple[j+k*totalCounts].get<0>()=="physical id")
		hwInfoStore[j].get<4>()=std::atoi(discovHWTuple[j+k*totalCounts].get<1>().c_str());
	    if(discovHWTuple[j+k*totalCounts].get<0>()=="siblings")
		hwInfoStore[j].get<5>()=std::atoi(discovHWTuple[j+k*totalCounts].get<1>().c_str());
	    if(discovHWTuple[j+k*totalCounts].get<0>()=="core id")
		hwInfoStore[j].get<6>()=std::atoi(discovHWTuple[j+k*totalCounts].get<1>().c_str());
	    if(discovHWTuple[j+k*totalCounts].get<0>()=="cpu cores")
		hwInfoStore[j].get<7>()=std::atoi(discovHWTuple[j+k*totalCounts].get<1>().c_str());
	}
    }

    return hwInfoStore;
}

// This function returns the memory as a double in units of MiB
// Linux /proc/meminfo returns a string in kB
double SysInfo::ConvertMemKBtoMiB(string meminfo)
{
    // Remove all alpha and space
    meminfo.erase(remove_if(meminfo.begin(), meminfo.end(), not1(ptr_fun((int(*)(int))isdigit))), meminfo.end());

    // Convert the string into a double and convert units from kB to MiB
    double memKBtoMiB = atof(meminfo.c_str())/1000;

    return memKBtoMiB;
}


// TrimStr and ReduceStr are now called in GU.cpp
/*
string SysInfo::TrimStr(const string &str, const string &whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}
*/
/*
string SysInfo::ReduceStr(const string &str, const string &fill = " ", const string &whitespace = " \t")
{
    // trim first
    auto result =TrimStr(str, whitespace);

    // replace sub ranges
    auto beginSpace = result.find_first_of(whitespace);
    while (beginSpace != string::npos)
    {
        const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
        const auto range = endSpace - beginSpace;

        result.replace(beginSpace, range, fill);

        const auto newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }

    return result;
}
*/

// This uses boost::regex to tie together multiple commands to properly 
// obtain the fabric information. The foundational operation is near 
// identical to the str_str_tuple_list HardwareDiscov(...) function, but does not
// accept any inputs and returns a vector of tuples. Each tuple contains
// four string values, where: 
// string<0> is the alias name of the network device
// string<1> is the physical pci slot and hardware switch of string<0>
// string<2> is the type of fabric used by the network device
// string<3> is the current operational status of the device
str_str_str_str_tuple_list SysInfo::FabricDiscov()
{
    vector<string> pci;
    vector<string> net;
    string line, lspci_cmd, lspci_file, netlist_cmd, netlist_file, iplink_cmd, iplink_file;

    netlist_cmd = "ls -la /sys/class/net/* | grep device | awk '{print $NF;}'";
    char netlist_file_temp [] = "/tmp/sam_netlist_tmp_cmd.XXXXXX";
    mkstemp(netlist_file_temp);

    netlist_file = netlist_file_temp;
    ExecSysCommand((netlist_cmd + " > " + netlist_file).c_str());
 
    lspci_cmd = "/sbin/lspci";
    char lspci_file_temp []= "/tmp/sam_lspci_tmp_cmd.XXXXXXX";
    mkstemp(lspci_file_temp);

    lspci_file = lspci_file_temp;
    ExecSysCommand((lspci_cmd + " > " + lspci_file).c_str());

    iplink_cmd = "ip link";
    char iplink_file_temp []= "/tmp/sam_iplink_tmp_cmd.XXXXXX";
    mkstemp(iplink_file_temp);

    iplink_file = iplink_file_temp;
    ExecSysCommand((iplink_cmd + " > " + iplink_file).c_str());

    str_str_str_str_tuple_list netinfo;

    int i=0, j=0, k=0, devCount=0;
    unsigned count = 0;
    
    ifstream netline(netlist_file.c_str());
    if (netline.is_open())
    {
	while (netline.good())
	{
	    getline(netline, line);

	    boost::regex testFab("/");
	    boost::sregex_token_iterator k(line.begin(), line.end(), testFab, -1);
	    boost::sregex_token_iterator j;
	    
	    count = 0;
	    while(k != j)
	    {
		net.push_back(*k++);
		count++;
	    }

	    if (count != 0)
	    {
		netinfo.push_back( boost::tuple<string, string, string, string>(net[net.size()-1], net[net.size()-3], "" , ""));
		++devCount;
	    }
	}
    }
    
    bool linkfound = false;
    string pciFind, netStatFind;
    int countValues=0;
    for (str_str_str_str_tuple_list::const_iterator i = netinfo.begin(); i != netinfo.end(); ++i)
    {
	netStatFind = i->get<0>();
	pciFind = i->get<1>();
	if (pciFind != "virtual")
	{
	    // Detecting the corresponding fabric switch hardware located @ each network device pci slot
	    pciFind = pciFind.substr(5);
	    ifstream lspciline(lspci_file.c_str());
	    if (lspciline.is_open())
	    {
		while (lspciline.good())
		{
		    getline(lspciline, line);

		    boost::regex pciInfo("(" + pciFind +") (.*)");
		    boost::cmatch results;
		    if (regex_match(line.c_str(), results, pciInfo))
			netinfo[countValues].get<1>() = results[0];
		}
	    }
	}

	ifstream iplinkline(iplink_file.c_str());
	if (iplinkline.is_open())
	{
	    while (iplinkline.good())
	    {
		getline(iplinkline, line);

		// Detecting the status of each network device
		boost::regex iplinkInfo("(.*)(" + netStatFind + ":)(.*)(state UP)(.*)");
		boost::cmatch results;
		boost::regex iplinkInfo2("(.*)(" + netStatFind + ":)(.*)(state DOWN)(.*)");
		boost::cmatch results2;
		boost::regex iplinkInfo3("(.*)(" + netStatFind + ":)(.*)(state UNKNOWN)(.*)");
		boost::cmatch results3;
		boost::regex iplinkInfo4(".*link/(.*)\\s.*\\s.*brd.*");
		boost::cmatch results4;

		if (regex_match(line.c_str(), results, iplinkInfo))
		{
		    netinfo[countValues].get<3>() = "state UP";
		    linkfound = true;
		}
		else if (regex_match(line.c_str(), results2, iplinkInfo2))
		{
		    netinfo[countValues].get<3>() = "state DOWN";
		    linkfound = true;
		}
		else if (regex_match(line.c_str(), results3, iplinkInfo3))
		{
		    netinfo[countValues].get<3>() = "state UNKNOWN";
		    linkfound = true;
		}
	
		if (regex_match(line.c_str(), results4, iplinkInfo4) && linkfound)
		{
		    netinfo[countValues].get<2>() = results4[1];
		    linkfound = false;
		}
	    }
	}
	++countValues;
    }
    
    unlink(netlist_file_temp);
    unlink(lspci_file_temp);
    unlink(iplink_file_temp);

    return netinfo;
}
#endif

void SysInfo::Initialize() // Set all values then set _initialized = true
{
#ifdef _WIN32
    DWORD hostname_len = MAX_COMPUTERNAME_LENGTH;
    char *hostname = new char [hostname_len + 1];
    if (GetComputerNameA(hostname, &hostname_len)) _hostname = string(hostname);
    free(hostname);

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    //_cpu_total_cores = si.dwNumberOfProcessors; // This is _cpu_total_threads (including hyper-threads), but we do it below instead
    _mem_page_size = si.dwPageSize;

    {
        _os_name = "Microsoft Windows";
        LPTSTR win_dir = (LPTSTR)malloc(4096);
        GetWindowsDirectoryA(win_dir, 4096);
        string binary_path = win_dir + string("\\System32\\Kernel32.dll");
        free(win_dir);
        _os_version = Intel_Base::GetBinaryVersion(binary_path);
    }

    DWORD slpi_len = 0;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION slpi = NULL;
    GetLogicalProcessorInformation(slpi, &slpi_len); // Sets slpi_len
    slpi = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(slpi_len);
    if (GetLogicalProcessorInformation(slpi, &slpi_len))
    {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION slpi_ptr = slpi;
        DWORD slpi_byte_offset = 0;
        _cpu_num_sockets = _cpu_cores_per_socket = _cpu_total_cores = 0;
        _mem_cache_count = map<int, int>();
        while (slpi_byte_offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= slpi_len)
        {
            switch (slpi_ptr->Relationship)
            {
            case RelationNumaNode:
                //_cpu_num_sockets++; // Non-NUMA systems report a single record of this type
                break;

            case RelationProcessorCore:
                _cpu_cores_per_socket++;

                // A hyper-threaded core supplies more than one logical processor
                {
                    DWORD LSHIFT = sizeof(ULONG_PTR)* 8 - 1, bitSetCount = 0;
                    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
                    for (DWORD i = 0; i <= LSHIFT; ++i)
                    {
                        bitSetCount += ((slpi_ptr->ProcessorMask & bitTest) ? 1 : 0);
                        bitTest /= 2;
                    }
                    _cpu_total_cores += bitSetCount;
                }
                break;

            case RelationCache:
            {
                map<int, int>::iterator mi = _mem_cache_count.find(slpi_ptr->Cache.Level);
                if (mi == _mem_cache_count.end())
                    _mem_cache_count.insert(pair<int, int>(slpi_ptr->Cache.Level, 1));
                else
                    ++_mem_cache_count[slpi_ptr->Cache.Level]; // One CACHE_DESCRIPTOR structure for each cache
            }
	    break;

            case RelationProcessorPackage:
                _cpu_num_sockets++; // Logical processors share a physical package
                break;

            default:
                cerr << "Error: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value." << endl;
                break;
            }
            slpi_byte_offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
            slpi_ptr++;
        }
    }
    free(slpi);

    HKEY key;  // Could access all CPUs this way by using ID at end of key
    if (RegOpenKey(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &key) == ERROR_SUCCESS)
    {
        DWORD mhz = 0, mhz_len = sizeof(mhz);
        if (RegQueryValueEx(key, "~MHz", NULL, NULL, (LPBYTE)&mhz, &mhz_len) == ERROR_SUCCESS)
        {
            _cpu_max_frequency = (float)mhz * 1e6f;
        }

        DWORD cpu_data_len = 1024, cpu_data_type;
        char cpu_data[1024];
        memset(cpu_data, 0, cpu_data_len);
        if (RegQueryValueEx(key, "ProcessorNameString", NULL, &cpu_data_type, (BYTE*)cpu_data, &cpu_data_len) == ERROR_SUCCESS)
        {
            _cpu_description = cpu_data;
        }
        memset(cpu_data, 0, cpu_data_len);
        if (RegQueryValueEx(key, "VendorIdentifier", NULL, &cpu_data_type, (BYTE*)cpu_data, &cpu_data_len) == ERROR_SUCCESS)
        {
            _cpu_vendor = cpu_data;
        }
        RegCloseKey(key);
    }

    _cpu_threads_per_core = _cpu_total_cores / _cpu_cores_per_socket;
    _cpu_cores_per_socket /= _cpu_num_sockets;
    _cpu_total_cores = _cpu_num_sockets * _cpu_cores_per_socket;
    _cpu_act_frequency = _cpu_max_frequency; // TODO: Actually calculate this

    //GetAdaptersAddresses

// Identify the system information on Linux OS
#else
    str_str_tuple_list discovHostTuple = HostDiscov(); 
    for (str_str_tuple_list::const_iterator i = discovHostTuple.begin(); i != discovHostTuple.end(); ++i)
    {
	if (i->get<0>()=="hostname")
	    _hostname = i->get<1>();
	else if (i->get<0>()=="dns_suffix")
	    _dns_suffix = i->get<1>();
	else if (i->get<0>()=="version")
	    _os_version = i->get<1>();
	else if (i->get<0>()=="platform")
	    _os_inst_type = i->get<1>();
	else if (i->get<0>()=="osname")
	    _os_name = i->get<1>();
    }

    elem_x_8_tuple_list hwInfoStore = GroupHardwareDiscov();
    int totalCounts = hwInfoStore.size();
	
    int same_core_id_count=0, same_physical_id_count=0, same_siblings_count, same_cpu_cores_count;
    int siblings=hwInfoStore[0].get<5>(); 
 
    for (elem_x_8_tuple_list::const_iterator i = hwInfoStore.begin(); i != hwInfoStore.end(); ++i)
    {
	if (hwInfoStore[0].get<4>()==i->get<4>())
	    same_physical_id_count++;
	if (hwInfoStore[0].get<5>()==i->get<5>())
	    same_siblings_count++;
	if (hwInfoStore[0].get<6>()==i->get<6>())
	    same_core_id_count++;
	if (hwInfoStore[0].get<7>()==i->get<7>())
	    same_cpu_cores_count++;	    
    }
    

    _cpu_vendor = hwInfoStore[0].get<1>();
    _cpu_description = hwInfoStore[0].get<2>();
    _cpu_num_sockets = totalCounts / same_physical_id_count;
    _cpu_cores_per_socket = hwInfoStore[0].get<7>();
    // threads/core = siblings / cpu cores, 
    // e.g. 8 siblings / 8 cpu cores = 1 thread/core
    _cpu_threads_per_core = hwInfoStore[0].get<5>() / hwInfoStore[0].get<7>(); 
    _cpu_total_cores = _cpu_num_sockets * _cpu_cores_per_socket;
    _cpu_act_frequency = hwInfoStore[0].get<3>(); 
    _cpu_max_frequency = hwInfoStore[0].get<3>(); // Currently using the actual frequency and not the spec freq

    _mem_page_size = atoi(ExecSysCommand("getconf PAGESIZE").c_str());

    vector<string> memlist;
    memlist.push_back("MemTotal");
    memlist.push_back("MemFree");
    memlist.push_back("Buffers");
    memlist.push_back("Cached");
    memlist.push_back("SwapTotal");
    memlist.push_back("SwapFree");
    memlist.push_back("Shmem");
    
    str_str_tuple_list discovMemTuple = HardwareDiscov("/proc/meminfo", memlist);
    totalCounts = discovMemTuple.size()/memlist.size();

    str_str_str_str_tuple_list discovFabricInfo = FabricDiscov();

#endif

    _cpu_total_threads = _cpu_total_cores * _cpu_threads_per_core;
    _initialized = true;
}

string SysInfo::HostName() const // Host Name (e.g. sam, which is in sam.jf.intel.com)
{
    return _hostname;
}

string SysInfo::DnsSuffix() const // Domain Name System (DNS) suffix (e.g. jf.intel.com)
{
    return _dns_suffix;
}

string SysInfo::OsName() const // Operating System Name
{
    return _os_name;
}

string SysInfo::OsInstType() const // Operating System Instruction Type (e.g. x86, x86_64)
{
    return _os_inst_type;
}

string SysInfo::OsVersion() const // Operating System Version string
{
    return _os_version;
}

string SysInfo::CpuVendor() const // CPU Vendor ID (e.g. GenuineIntel)
{
    return _cpu_vendor;
}

string SysInfo::CpuDescription() const // CPU Description (e.g. Intel(R) Core(TM) i5-2520M CPU @ 2.50GHz)
{
    return _cpu_description;
}

int SysInfo::CpuNumSockets() const // Number of CPU NUMA nodes
{
    return _cpu_num_sockets;
}

int SysInfo::CpuCoresPerSocket() const // Number of processing cores per NUMA node
{
    return _cpu_cores_per_socket;
}

int SysInfo::CpuThreadsPerCore() const // Number of threads that run simultantoeously per core (e.g. 2 with hyper-threading)
{
    return _cpu_threads_per_core;
}

int SysInfo::CpuTotalCores() const // CpuNumSockets() * CpuCoresPerSocket()
{
    return _cpu_total_cores; 
}

int SysInfo::CpuTotalThreads() const // CpuTotalCores() * CpuThreadsPerCore()
{
    return _cpu_total_threads; 
}

int SysInfo::MemPageSize() const // Memory Page Size
{
    return _mem_page_size;
}

const map<int, int> &SysInfo::MemCacheCount() const // Cache count for each level, which is the index, so L1 cache count is v[1]
{
    return _mem_cache_count;
}

const map<int, int> &SysInfo::MemCacheSize() const // Cache size in bytes for each level, which is the index, so L1 cache size is v[1]
{
    return _mem_cache_size;
}

float SysInfo::CpuActFrequency() const // CPU Actual Frequency
{
    return _cpu_act_frequency;
}

float SysInfo::CpuMaxFrequency() const // CPU Maximum Frequency
{
    return _cpu_max_frequency;
}

string SysInfo::ToString() const // String version of the attributes of this
{
    ostringstream ss;
    ss << "HostName " << _hostname << "  DnsSuffix " << _dns_suffix
       << "\nOS  Name " << _os_name << "  InstType " << _os_inst_type << "  Version " << _os_version
       << "\nCPU  Desc " << _cpu_description << "  Vendor " << _cpu_vendor
       << "\nCPU  Sockets " << _cpu_num_sockets << "  Cores/Socket " << _cpu_cores_per_socket
       << "  Threads/Core " << _cpu_threads_per_core << "  TotalCores " << _cpu_total_cores << "  TotalThreads " << _cpu_total_threads
       << "  ActFreq " << _cpu_act_frequency << "  MaxFreq " << _cpu_max_frequency
       << "\nMem  PageSize " << _mem_page_size;
    if (_mem_cache_count.size() > 0)
    {
        ss << "\nMem CacheCount";
        for (map<int, int>::const_iterator mi = _mem_cache_count.begin(); mi != _mem_cache_count.end(); ++mi)
        {
            if (mi->second > 0) ss << "  L" << mi->first << " " << mi->second;
        }
    }
    if (_mem_cache_size.size() > 0)
    {
        ss << "\nMem CacheSize";
        for (map<int, int>::const_iterator mi = _mem_cache_size.begin(); mi != _mem_cache_size.end(); ++mi)
        {
            if (mi->second > 0) ss << "  L" << mi->first << " " << mi->second;
        }
    }
    return ss.str();
}
