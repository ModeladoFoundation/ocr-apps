#include "BenchMarkReturn.h"
#include "GU.h"
using namespace std;
using namespace Intel_Base;

/*
* Constructor
* @param p                  Benchmark parameter struct
*/
BenchMarkReturn::BenchMarkReturn(BenchMarkParam &p)
{
    R = BenchMarkResult(p.Name, p.NumTasks, p.NumThreadsPerTask, p.DataSize, p.TileSize, p.Runs);
}


BenchMarkReturn::~BenchMarkReturn()
{
}

/*
* Human-readable version of this
* @param p                  Benchmark parameter struct
* @param passLimit          Benchmark validation test passing error threshold
* @param ntVsTErr           Non-Tiled vs. Tiled Error (default -1)
* @param errW               Error display width (default 11)
* @param errP               Error display precision (default 0)
*/
string BenchMarkReturn::ToString(const BenchMarkParam &p, const double &passLimit, const double &ntVsTErr, const int &errW, const int &errP) const
{
    ostringstream ss;
    if (p.DL > 1)
    {
        if (TB.size() > 0 && (TB.size() < p.DLMaxSize || p.DL > 2)) ss << TB.ToString("TB", p.MatW, p.MatP);
        else if (B.size() > 0 && (B.size() < p.DLMaxSize || p.DL > 2)) ss << B.ToString("B", p.MatW, p.MatP);
    }
    else if (p.DL > 0)
    {
        if (TB.size() > 0 && TB.size() < p.DLMaxSize) ss << TB.ToStringSize("TB");
        else if (B.size() > 0 && B.size() < p.DLMaxSize) ss << B.ToStringSize("B");
    }
    ss << R.ToString(p.DL, p.RtW, p.RtP, errW, errP);
    if (R.NonTiledErr >= 0 || R.TiledErr >= 0)
    {
        ss << "  " << R.Name;
        if (R.NonTiledErr >= 0) ss << "  Non-Tiled " << PassFail(R.NonTiledErr < passLimit);
        if (R.TiledErr >= 0) ss << "  Tiled " << PassFail(R.TiledErr < passLimit);
        if (ntVsTErr >= 0)
        {
            ss << "  Non-Tiled vs. Tiled " << PassFail(ntVsTErr < 2 * passLimit);
            if (errW > 0) ss.width(errW);
            if (errP > 0) ss.precision(errP);
            ss << ntVsTErr << "\n";

            if (!p.AnsFile.empty())
            {
                Matrix D(p.AnsFile);
                const double BD_error = B.AbsMaxDiff(D);
                ss << "  " << R.Name << " Validate .ans file with corresponding .in file               Error ";
                if (errW > 0) ss.width(errW);
                if (errP > 0) ss.precision(errP);
                ss << BD_error << " " << PassFail(BD_error < passLimit) << endl;
            }
        }
        else ss << "\n";
    }
    return ss.str();
}
