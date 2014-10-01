/*
Intel_BenchMark Application: Intel BenchMark command-line tool

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#ifdef _WIN32
#include "stdafx.h"
#endif
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <BenchMarkParam.h>
#include <BenchMarkResult.h>
#include <BenchMarkReturn.h>
#include <BinaryFile.h>
#include <GU.h>
#include <SysInfo.h>
#include <TileMatrix.h>
#include <omp.h>
#include <mkl.h>
using namespace std;
using namespace Intel_Base;

const int &ds_count = 7; // 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384

void usage()
{
    cout << "\n"
        << "  Usage: Intel_BenchMark -b BenchType [options]\n"
        << "\n  where:\n"
        << "    -b  BenchType   -- Benchmark to Run\n"
        << "                       all     : -ds DataSize -ts TileSize (runs 1dfft, 2dfft, cholesky)\n"
        << "                       allsweep: -ds DataSize (runs all for ds to ds * 2^" << ds_count << " and ts from 4 to ds/2)\n"
        << "                       1dfft   : -ds DataSize -ts TileSize\n"
        << "                       2dfft   : -ds DataSize -ts TileSize\n"
        << "                       cholesky: -ds DataSize -ts TileSize\n\n"
        << "                       binfile : none\n"
        << "                       genutil : none\n"
        << "                       matrix  : -ds DataSize (Matrix unit tests)\n"
        << "                       tmatrix : -ds DataSize -ts TileSize (TileMatrix unit tests)\n"
        << "                       sysinfo : none\n"
        << "                       timing  : -ts NumSeconds\n"
        << "\n  options:\n"
        << "    -ds DataSize    -- Data size\n"
        << "    -ts TileSize    -- Process tile size (1 default for non-tiled implementation, else gt 1 for tiled implementation)\n\n"

        << "    -ns NumSockets  -- Number of sockets (parallel tasks) to use for benchmark, else 0 to detect sockets (0 default)\n"
        << "    -nt NumThreads  -- Number of threads/task to use for benchmark, else 0 to detect cores/socket (0 default)\n"
        << "    -r  Runs        -- Number of repetitions of benchmark, which are averaged for time (2 default)\n\n"

        << "    -dl DebugLevel  -- Debugging output level {0: none, 1: summary, 2: detail, 3: verbose} (1 default)\n"
        << "    -fi InFile      -- Input Matrix file name, input to benchmark (optional)\n"
        << "    -fo OutFile     -- Output Matrix file name, result of benchmark (optional)\n"
        << "    -fa AnsFile     -- Answer Matrix file name, precalculated for corresponding InFile should match OutFile (optional)\n"
        << "    -fr RepFile     -- Output binary report file name, appends record if exists (optional)\n"
        << "    -frt TxtFile    -- Output text comma separated value report file name, reads RepFile to generate (optional)\n"
        << flush;
}

BenchMarkResult test_cholesky(BenchMarkParam &bmp)
{
    bmp.Name = "Cholesky";
    if (bmp.TileSize > 0 && (bmp.CheckFiles() || bmp.DataSize > 0) && bmp.DataSize % bmp.TileSize == 0)
    {
        if (bmp.A.size() == 0)
        {
            if (bmp.InFile.empty())
            {
                if (bmp.DataSize < 33)
                {
                    bmp.A = Matrix::Pascal(bmp.DataSize);
                    if (!bmp.A.IsSymmetricPositiveDefinite()) bmp.A = Matrix::Diag(bmp.DataSize);
                }
                else bmp.A = Matrix::Diag(bmp.DataSize);
            }
            else bmp.A = Matrix(bmp.InFile);
        }

        BenchMarkReturn bmr(bmp);
        if (bmp.A.size() == 0 || !bmp.A.IsSquare())
        {
            cerr << bmp.Name << " Error: A matrix must be non-empty, square, and positive definite" << endl;  exit(1);
        }
        bmp.TA = TileMatrix(bmp.A.row_order ? bmp.A : ~bmp.A, bmp.A.rows / bmr.R.TileSize, bmp.A.cols / bmr.R.TileSize);
        cout << bmp.ToString();

        // Do Non-Tiled Runs
        bmr.R.NonTiledSec = 0;
        for (int run = 0; run < bmp.Runs; ++run)
        {
            const double sec_start = TimeNowInSeconds();

            bmr.B = bmp.A.Cholesky(bmp.NumTasks * bmp.NumThreadsPerTask);

            bmr.R.NonTiledSec += TimeNowInSeconds() - sec_start;
        }
        bmr.R.NonTiledSec /= (double)bmp.Runs;
        bmr.R.NonTiledErr = bmp.A.AbsMaxDiff(bmr.B.Multiply(bmr.B, true)); // Same as B * ~B, but faster because avoids temporary transpose matrix

        // Do Tiled Runs
        bmr.R.TiledSec = 0;
        for (int run = 0; run < bmp.Runs; ++run)
        {
            bmr.TB = bmp.TA; // Make a copy since modified below

            bmr.R.TiledSec += TileMatrix::Cholesky(bmr.TB, bmp.NumTasks, bmp.NumThreadsPerTask);
        }
        bmr.R.TiledSec /= (double)bmp.Runs;
        bmr.R.TiledErr = bmp.TA.AbsMaxDiff(bmr.TB.Multiply(bmr.TB, true)); // Same as B * ~B, but faster because avoids temporary transpose matrix
        bmr.R.UpdateSpeedUpNtVT();

        const double ntVsTErr = bmr.R.NonTiledSec >= 0 && bmr.R.TiledSec >= 0 ? bmr.B.AbsMaxDiff(bmr.TB) : -1;
        cout << bmr.ToString(bmp, Matrix::Epsilon(), ntVsTErr);

        if (bmr.R.NonTiledSec >= 0)
        {
            if (!bmp.OutFile.empty())
            {
                string out_file_ok = bmr.B.WriteFile(bmp.OutFile) ? "SUCCESS" : "FAIL";
                if (bmp.DL > 1)  cout << "  OutFile " << bmp.OutFile << " write " << out_file_ok << endl;
            }
        }
        return bmr.R;
    }
    else
    {
        cerr << "  Error: Must have TileSize > 0 and DataSize % TileSize == 0 and InFile exists or DataSize > 0" << endl; exit(1);
    }
}

void test_timing(const BenchMarkParam &bmp)
{
    cout << "  " << bmp.Name << "  " << bmp.TileSize << " second sleep timing test" << endl;
    unsigned long long clocks_start = TimeNowInClocks(), clocks_end;
    SleepSeconds(bmp.TileSize);
    clocks_end = TimeNowInClocks();
    double clock_freq = FreqMaxInHertz(), clocks_duration = (clocks_end - clocks_start) / clock_freq,
        clock_error = fabs(clocks_duration - bmp.TileSize);
    cout
        << "\n     clocks_start " << clocks_start << " (freq " << clock_freq << ")\n       clocks_end " << clocks_end
        << "\n  clocks_duration " << setw(bmp.RtW) << setprecision(bmp.RtP) << clocks_duration << " seconds"
        << "\n      clock_error " << setw(bmp.RtW) << setprecision(bmp.RtP) << clock_error << " seconds" << endl;

    double sec_start = TimeNowInSeconds();
    SleepSeconds(bmp.TileSize);
    double sec_end = TimeNowInSeconds(), sec_duration = sec_end - sec_start, sec_error = fabs(sec_duration - bmp.TileSize);
    cout
        << "\n        sec_start " << sec_start
        << "\n          sec_end " << sec_end
        << "\n     sec_duration " << setw(bmp.RtW) << setprecision(bmp.RtP) << sec_duration << " seconds"
        << "\n        sec_error " << setw(bmp.RtW) << setprecision(bmp.RtP) << sec_error << " seconds" << endl;

    if (sec_error <= clock_error)
        cout << "\n  TimeNowInSeconds() is more accurate (" << PercentSpeedup(clock_error, sec_error) << "%)" << endl;
    else
        cout << "\n  TimeNowInClocks() is more accurate (" << PercentSpeedup(sec_error, clock_error) << "%)" << endl;
}

void test_sysinfo(const BenchMarkParam &bmp)
{
    SysInfo si;
    cout << "  " << bmp.Name << ":\n" << si.ToString() << endl;
#ifdef _WIN32
    //cin.get();
#endif
}

const int _fft_row_multiple = 1; // Multiplier of columns so that rows of FFT benchmarks

BenchMarkResult test_1dfft(BenchMarkParam &bmp)
{
    bmp.Name = "1D FFT";
    if (bmp.TileSize > 0 && (bmp.CheckFiles() || bmp.DataSize > 0) && bmp.DataSize % bmp.TileSize == 0)
    {
        if (bmp.A.size() == 0)
        {
            if (bmp.InFile.empty())
            {
                bmp.A = Matrix::RandUniformInteger(bmp.DataSize * _fft_row_multiple, bmp.DataSize, 0, 100);
            }
            else bmp.A = Matrix(bmp.InFile);
        }
        bmp.MatW = 5;

        BenchMarkReturn bmr(bmp);
        if (bmp.A.size() == 0)
        {
            cerr << bmp.Name << " Error: A matrix must be non-empty" << endl;  exit(1);
        }
        bmp.TA = TileMatrix(bmp.A.row_order ? bmp.A : ~bmp.A, bmp.A.rows / bmr.R.TileSize, bmp.A.cols / bmr.R.TileSize);
        cout << bmp.ToString();

        // Do Non-Tiled Runs
        bmr.R.NonTiledSec = 0;
        for (int run = 0; run < bmp.Runs; ++run)
        {
            const double sec_start = TimeNowInSeconds();

            bmr.B = bmp.A.FFT1DMagRows(bmp.NumTasks, bmp.NumThreadsPerTask);

            bmr.R.NonTiledSec += TimeNowInSeconds() - sec_start;
        }
        bmr.R.NonTiledSec /= (double)bmp.Runs;
        bmr.R.NonTiledErr = -1;
        //bmr.B.ToString("B");

        // Do Tiled Runs
        bmr.R.TiledSec = 0;
        for (int run = 0; run < bmp.Runs; ++run)
        {
            bmr.TB = bmp.TA; // Make a copy since modified below

            bmr.R.TiledSec += TileMatrix::FFT1DMagRows(bmr.TB, bmp.NumTasks, bmp.NumThreadsPerTask);
        }
        bmr.R.TiledSec /= (double)bmp.Runs;
        bmr.R.TiledErr = -1; //bmr.B.AbsMaxDiff(bmr.TB);
        bmr.R.UpdateSpeedUpNtVT();

        cout << bmr.ToString(bmp, Matrix::Epsilon(), -1);

        if (bmr.R.NonTiledSec >= 0)
        {
            if (!bmp.OutFile.empty())
            {
                string out_file_ok = bmr.B.WriteFile(bmp.OutFile) ? "SUCCESS" : "FAIL";
                if (bmp.DL > 1)  cout << "  OutFile " << bmp.OutFile << " write " << out_file_ok << endl;
            }
        }
        return bmr.R;
    }
    else
    {
        cerr << "  Error: Must have TileSize > 0 and DataSize % TileSize == 0 and InFile exists or DataSize > 0" << endl; exit(1);
    }
}

BenchMarkResult test_2dfft(BenchMarkParam &bmp)
{
    bmp.Name = "2D FFT";
    if (bmp.TileSize > 0 && (bmp.CheckFiles() || bmp.DataSize > 0) && bmp.DataSize % bmp.TileSize == 0)
    {
        if (bmp.A.size() == 0)
        {
            if (bmp.InFile.empty())
            {
                bmp.A = Matrix::RandUniformInteger(bmp.DataSize * _fft_row_multiple, bmp.DataSize, 0, 100);
                //Matrix A = Matrix::RandGaussian(bmp.DataSize * _fft_row_multiple, bmp.DataSize);
            }
            else bmp.A = Matrix(bmp.InFile);
        }
        bmp.MatW = 5;

        BenchMarkReturn bmr(bmp);
        if (bmp.A.size() == 0)
        {
            cerr << bmp.Name << " Error: A matrix must be non-empty" << endl;  exit(1);
        }
        bmp.TA = TileMatrix(bmp.A.row_order ? bmp.A : ~bmp.A, bmp.A.rows / bmr.R.TileSize, bmp.A.cols / bmr.R.TileSize);
        cout << bmp.ToString();

        // Do Non-Tiled Runs
        bmr.R.NonTiledSec = 0;
        for (int run = 0; run < bmp.Runs; ++run)
        {
            const double sec_start = TimeNowInSeconds();

            bmr.B = bmp.A.FFT2DMagRows(bmp.NumTasks, bmp.NumThreadsPerTask);

            bmr.R.NonTiledSec += TimeNowInSeconds() - sec_start;
        }
        bmr.R.NonTiledSec /= (double)bmp.Runs;
        bmr.R.NonTiledErr = -1;

        // Do Tiled Runs
        bmr.R.TiledSec = 0;
        for (int run = 0; run < bmp.Runs; ++run)
        {
            bmr.TB = bmp.TA; // Make a copy since modified below

            bmr.R.TiledSec += TileMatrix::FFT2DMagRows(bmr.TB, bmp.NumTasks, bmp.NumThreadsPerTask);
        }
        bmr.R.TiledSec /= (double)bmp.Runs;
        bmr.R.TiledErr = -1;
        bmr.R.UpdateSpeedUpNtVT();

        cout << bmr.ToString(bmp, Matrix::Epsilon(), -1);

        if (bmr.R.NonTiledSec >= 0)
        {
            if (!bmp.OutFile.empty())
            {
                string out_file_ok = bmr.B.WriteFile(bmp.OutFile) ? "SUCCESS" : "FAIL";
                if (bmp.DL > 1)  cout << "  OutFile " << bmp.OutFile << " write " << out_file_ok << endl;
            }
        }
        return bmr.R;
    }
    else
    {
        cerr << "  Error: Must have TileSize > 0 and DataSize % TileSize == 0 and InFile exists or DataSize > 0" << endl; exit(1);
    }
}

void test_matrix_unit(const string &name, const Matrix &A)
{
    cout << A.ToString(name) << "  Square " << A.IsSquare() << "  Symmetric " << A.IsSymmetric()
        << "  Determinant " << A.Determinant() << "  SPD " << A.IsSymmetricPositiveDefinite() << "  SPSD " << A.IsSymmetricPositiveSemiDefinite() << endl;
    Matrix A_seig = A.SymmetricEigenValues();
    cout << A_seig.ToString(name + " Symmetric EigenValues") << endl;
    Matrix A_nseig = A.NonSymmetricEigenValues();
    cout << A_nseig.ToString(name + " NonSymmetric EigenValues") << "\n" << endl;
}

void test_matrix(const BenchMarkParam &bmp)
{
    if (bmp.DataSize > 0)
    {
        cout << "  " << bmp.Name << " test:  DataSize " << bmp.DataSize << endl;

        test_matrix_unit("Diagonal", Matrix::Diag(bmp.DataSize));
        test_matrix_unit("Pascal", Matrix::Pascal(bmp.DataSize));
        test_matrix_unit("RandUniform", Matrix::RandUniform(bmp.DataSize, bmp.DataSize, -2.0, -1.0));
        test_matrix_unit("RandGaussian", Matrix::RandGaussian(bmp.DataSize, bmp.DataSize, -2.0, 1));
        test_matrix_unit("RandGaussianSPD", Matrix::RandGaussianSPD(bmp.DataSize, -2.0, 1));
    }
    else cerr << "  Error: Must have DataSize > 0" << endl;
}

void test_tmatrix(const BenchMarkParam &bmp)
{
    if (bmp.DataSize > 0 && bmp.TileSize > 0 && bmp.DataSize % bmp.TileSize == 0)
    {
        cout << "  " << bmp.Name << " test:  DataSize " << bmp.DataSize << "  TileSize " << bmp.TileSize << endl;
        const int rows = bmp.DataSize / bmp.TileSize;
        Matrix A(rows * bmp.TileSize, (rows + 1) * bmp.TileSize);
        for (int i = 0; i < A.size(); ++i) A[i] = i;
        cout << A.ToString("A") << endl;
        TileMatrix TA(A, rows, rows + 1);
        cout << TA.ToString("TA") << endl;
        cout << "A.AbsMaxDiff(TA) " << A.AbsMaxDiff(TA) << endl;
    }
    else cerr << "  Error: Must have DataSize > 0 and TileSize > 0 and DataSize % TileSize = 0" << endl;
}

class MemRec
{
public:
    char type = ' ';
    unsigned int total = 0;
    float frequency = 0;

    MemRec() {}

    MemRec(char type_in, unsigned int total_in, float frequency_in)
    {
        type = type_in;
        total = total_in;
        frequency = frequency_in;
    }

    // Return record length in bytes
    int Size()
    {
        return sizeof(type) + sizeof(total) + sizeof(frequency);
    }

    // Convert record properties to a separator separated list of values
    string ToString(const string &separator = " ")
    {
        ostringstream os;
        os << type << separator << total << separator << frequency;
        return os.str();
    }
};

void test_binaryfile(const BenchMarkParam &bmp)
{
    BinaryFile<MemRec> bf = BinaryFile<MemRec>(INTEL_BASE_KERNEL_VERSION, "BinaryFile.dat");
    cout << "  " << bmp.Name << " test  bf.HeaderSize " << bf.HeaderSize() << "  bf.RecordSize " << bf.RecordSize() << endl;
    if (bf.WriteHeader())
    {
        vector<MemRec> recs = vector<MemRec>(5);
        for (int i = 0; i < recs.capacity(); ++i)
        {
            recs[i] = MemRec((char)((int)'A' + i), i, (i + 1) * 11.22);
            cout << "  Write recs[" << i << "] " << recs[i].ToString() << endl;
            bf.Write(recs[i]);
        }
        //bf.Write(recs);
        if (bf.WriteFooter() && bf.OpenRead() && bf.ReadFooter() > 0)
        {
            cout << "   Read Footer Records " << bf.RecordCount() << flush;
            if (bf.ReadHeader())
            {
                cout << "  Read Header Version " << setprecision(8) << bf.FileVersion() << "  Magic " << bf.FileMagic() << endl;
                for (int i = 0; i < bf.RecordCount(); ++i)
                {
                    MemRec rec;
                    bf >> rec;
                    cout << "   Read recs[" << i << "] " << rec.ToString() << endl;
                }
                cout << "bf.ToString(1, 55)\n" << bf.ToString(1, 55) << endl;
                cout << "bf.ToString(3, 1)\n" << bf.ToString(3, 1) << endl;
                cout << "bf.ToString()\n" << bf.ToString() << endl;
                bf.Close();

                recs = bf.ReadMany();
                for (int i = 0; i < recs.size(); ++i)
                {
                    cout << "   ReadMany recs[" << i << "] " << recs[i].ToString() << endl;
                }
            }
            cout << "  File [" << bf.FilePath() << "]  Size " << FileSize(bf.FilePath()) << " -- Now deleting it." << endl;
            //cout << "WriteTextFile " << PassFail(bf.WriteTextFile("BinaryFile.txt")) << endl;
            FileDelete(bf.FilePath());
        }
    }
}

void test_generalutil(const BenchMarkParam &bmp)
{
    cout << "  " << bmp.Name << " test" << endl;
    {
        string str = "  \t \t\t   \tHi\t guys!    ";
        cout << "    str [" << str << "]  Trim(str) [" << TrimStr(str) << "]  ReduceStr(str) [" << ReduceStr(str) << "]" << endl;
    }
}

/*
* Append bmr to bmp.RepFile
* @param bmr                Benchmark Result
* @param bmp                Benchmark Paramters
* @return                   True on successful write
*/
bool Write_RepFile(const BenchMarkResult &bmr, const BenchMarkParam &bmp)
{
    if (!bmp.RepFile.empty() && bmr.DataSize > 0)
    {
        const bool &write_ok = bmr.WriteRecord(bmp.RepFile, true, bmp.DL);
        if (write_ok && bmp.DL > 1)
        {
            BinaryFile<BenchMarkResult> bf = BinaryFile<BenchMarkResult>(INTEL_BASE_KERNEL_VERSION, bmp.RepFile);
            if (bf.OpenRead() && bf.ReadFooter() > 0)
            {
                cout << "  RepFile [" << bf.FilePath() << "] Records " << bf.RecordCount()
                    << "  Size " << FileSize(bf.FilePath()) << " bytes" << endl;

                if (bmp.DL > 2 && bf.ReadHeader())
                {
                    cout << "    Read Header Version " << setprecision(8) << bf.FileVersion() << "  Magic " << bf.FileMagic() << endl;
                    //cout << "Records:\n" << bf.ToString(0, 0, ",") << endl;
                }
                bf.Close();
            }
        }
        return write_ok;
    }
    return false;
}

/*
* Write out bmp.TxtFile from bmp.RepFile
* @param bmp                Benchmark Paramters
* @return                   True on successful write
*/
bool Write_TxtFile(const BenchMarkParam &bmp)
{
    if (!bmp.RepFile.empty() && !bmp.TxtFile.empty())
    {
        const bool &write_ok = BenchMarkResult::WriteTextFile(bmp.RepFile, bmp.TxtFile, ",", bmp.DL);
        if (bmp.DL > 0) cout << "  TxtFile [" << bmp.TxtFile << "] Write " << (write_ok ? "SUCCESS" : "FAIL") << endl;
        return write_ok;
    }
    else return false;
}

/*
* Run 1D FFT, 2D FFT, and Cholesky BenchMarks, including call to Write_RepFile to append BenchMarkResult
*/
void test_all(BenchMarkParam &bmp)
{
    {
        BenchMarkResult bmr = test_1dfft(bmp); Write_RepFile(bmr, bmp);
    }
    cout << endl;

    {
        BenchMarkResult bmr = test_2dfft(bmp); Write_RepFile(bmr, bmp);
    }
    cout << endl;

    {
        bmp.A = Matrix();
        BenchMarkResult bmr = test_cholesky(bmp); Write_RepFile(bmr, bmp);
    }
    cout << endl;
}

int main(int argc, char* argv[])
{
    try
    {
        cout
            << " Intel_BenchMark Release " << setprecision(8) << INTEL_BASE_KERNEL_VERSION << "  (Compiled " << __DATE__ << " " << __TIME__ << ")\n"
            << "_____________________________________________________________________________________________________________________" << endl;
        if (argc < 2) usage();
        else
        {
            BenchMarkParam bmp;
            string bench_type;
            int i = 1;
            while (i < argc)
            {
                string si = string(argv[i]);
                if (si == "-b")
                {
                    if (i + 1 < argc) bench_type = string(argv[++i]);
                    else
                    {
                        cout << "  Must specify -b BenchType\n";  return 1;
                    }
                }
                else if (si == "-ds")
                {
                    if (i + 1 < argc) bmp.DataSize = atoi(argv[++i]);
                    else
                    {
                        cout << "  Must specify -ds DataSize\n";  return 1;
                    }
                }
                else if (si == "-ns")
                {
                    if (i + 1 < argc) bmp.NumTasks = atoi(argv[++i]);
                    else
                    {
                        cout << "  Must specify -ns NumSockets\n";  return 1;
                    }
                }
                else if (si == "-nt")
                {
                    if (i + 1 < argc) bmp.NumThreadsPerTask = atoi(argv[++i]);
                    else
                    {
                        cout << "  Must specify -nt NumThreads\n";  return 1;
                    }
                }
                else if (si == "-ts")
                {
                    if (i + 1 < argc) bmp.TileSize = atoi(argv[++i]);
                    else
                    {
                        cout << "  Must specify -ts TileSize\n";  return 1;
                    }
                }
                else if (si == "-r")
                {
                    if (i + 1 < argc) bmp.Runs = atoi(argv[++i]);
                    else
                    {
                        cout << "  Must specify -r Runs\n";  return 1;
                    }
                }
                else if (si == "-dl")
                {
                    if (i + 1 < argc) bmp.DL = atoi(argv[++i]);
                    else
                    {
                        cout << "  Must specify -dl DebugLevel\n";  return 1;
                    }
                }
                else if (si == "-dlmax")
                {
                    if (i + 1 < argc) bmp.DLMaxSize = atoi(argv[++i]);
                    else
                    {
                        cout << "  Must specify -dlmax DebugLevelMaxSize\n";  return 1;
                    }
                }
                else if (si == "-fr")
                {
                    if (i + 1 < argc) bmp.RepFile = string(argv[++i]);
                    else
                    {
                        cout << "  Must specify -fr RepFile\n";  return 1;
                    }
                }
                else if (si == "-frt")
                {
                    if (i + 1 < argc) bmp.TxtFile = string(argv[++i]);
                    else
                    {
                        cout << "  Must specify -frt TxtFile\n";  return 1;
                    }
                }
                else if (si == "-fi")
                {
                    if (i + 1 < argc) bmp.InFile = string(argv[++i]);
                    else
                    {
                        cout << "  Must specify -fi InFile\n";  return 1;
                    }
                }
                else if (si == "-fo")
                {
                    if (i + 1 < argc) bmp.OutFile = string(argv[++i]);
                    else
                    {
                        cout << "  Must specify -fo OutFile\n";  return 1;
                    }
                }
                else if (si == "-fa")
                {
                    if (i + 1 < argc) bmp.AnsFile = string(argv[++i]);
                    else
                    {
                        cout << "  Must specify -fa AnsFile\n";  return 1;
                    }
                }
                else ++i;
            }

            const bool default_sockets_threads = bmp.NumTasks == 0 && bmp.NumThreadsPerTask == 0;
            if (bmp.NumTasks == 0 || bmp.NumThreadsPerTask == 0)
            {
                SysInfo si;
                if (bmp.NumTasks == 0) bmp.NumTasks = si.CpuNumSockets();
                if (bmp.NumThreadsPerTask == 0) bmp.NumThreadsPerTask = si.CpuCoresPerSocket();
            }
            /*cout << "  Using NumSockets " << bmp.NumTasks << "  NumThreads " << bmp.NumThreadsPerTask;
            if (default_sockets_threads) cout << " (Default)";
            cout << endl;*/

            BenchMarkResult bmr;
            const bool &generate_txtfile = !bmp.RepFile.empty() && !bmp.TxtFile.empty();

            if (bmp.Runs < 1)
            {
                cerr << "  Error: Runs " << bmp.Runs << " must be > 0" << endl;
                usage();
            }
            else if (bench_type.empty() && !generate_txtfile)
            {
                cerr << "  Error: BenchType must be specified" << endl;
                usage();
            }
            else if (bench_type == "all")
            {
                test_all(bmp);
                Write_TxtFile(bmp);
                return 0;
            }
            else if (bench_type == "allsweep")
            {
                const int &ds_begin = bmp.DataSize, &ds_end = ds_begin * (int)pow(2, ds_count);
                for (int ds = ds_begin; ds <= ds_end; ds *= 2)
                {
                    bmp.DataSize = ds;
                    for (int ts =  ds / 2; ts >= 4; ts /= 2)
                    {
                        bmp.TileSize = ts;
                        if (bmp.TileSize > 1 && bmp.DataSize % bmp.TileSize == 0)
                        {
                            test_all(bmp);
                        }
                    }
                }
                Write_TxtFile(bmp);
                return 0;
            }
            else if (bench_type == "1dfft")
            {
                bmr = test_1dfft(bmp);
            }
            else if (bench_type == "2dfft")
            {
                bmr = test_2dfft(bmp);
            }
            else if (bench_type == "cholesky")
            {
                bmr = test_cholesky(bmp);
            }
            else if (bench_type == "timing")
            {
                bmp.Name = "Timing";
                test_timing(bmp);
            }
            else if (bench_type == "sysinfo")
            {
                bmp.Name = "System Information";
                test_sysinfo(bmp);
            }
            else if (bench_type == "matrix")
            {
                bmp.Name = "Matrix";
                test_matrix(bmp);
            }
            else if (bench_type == "tmatrix")
            {
                bmp.Name = "TileMatrix";
                test_tmatrix(bmp);
            }
            else if (bench_type == "binfile")
            {
                bmp.Name = "BinaryFile";
                test_binaryfile(bmp);
            }
            else if (bench_type == "genutil")
            {
                bmp.Name = "General Utilities";
                test_generalutil(bmp);
            }
            else if (!generate_txtfile)
            {
                cerr << "  Error: BenchType " << bench_type << " is not supported" << endl;
                usage();
            }
            
            Write_RepFile(bmr, bmp);
            Write_TxtFile(bmp);
        }
    }
    catch (exception e)
    {
        cerr << "Error: " << e.what() << endl;
    }
    return 0;
}
