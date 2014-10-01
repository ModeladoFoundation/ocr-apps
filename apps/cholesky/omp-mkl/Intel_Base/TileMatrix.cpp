/*
Intel Base Kernel Library: TileMatrix class

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#include "GU.h"
#include "TileMatrix.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <math.h>
#include <mkl.h>
using namespace std;
using namespace Intel_Base;

// Constructor
TileMatrix::TileMatrix()
{
    tile_rows = tile_cols = matrix_rows = matrix_cols = t_len = 0;
    t = NULL;
}

// Constructor: tileRows * tileCols tiles, each tile is a Matrix(matrixRows, matrixCols)
TileMatrix::TileMatrix(const int &tileRows, const int &tileCols, const int &matrixRows, const int &matrixCols)
{
    if (matrixRows > 1 && matrixCols > 1)
    {
        tile_rows = tileRows;
        tile_cols = tileCols;
        matrix_rows = matrixRows;
        matrix_cols = matrixCols;
    }
    else
    {
        tile_rows = 1;
        tile_cols = 1;
        matrix_rows = tileRows;
        matrix_cols = tileCols;
    }
    t_len = tile_rows * tile_cols;
    t = (Matrix *)mkl_malloc(sizeof(Matrix) * t_len, INTEL_BB);
    for (int i = 0; i < t_len; ++i) t[i] = new Matrix(matrix_rows, matrix_cols, true);
}

// Constructor: Copy B into this
TileMatrix::TileMatrix(const TileMatrix &B)
{
    tile_rows = B.tile_rows;
    tile_cols = B.tile_cols;
    matrix_rows = B.matrix_rows;
    matrix_cols = B.matrix_cols;
    t_len = tile_rows * tile_cols;
    t = (Matrix *)mkl_malloc(sizeof(Matrix) * t_len, INTEL_BB);
    for (int i = 0; i < tile_rows; ++i) // Tile Row
    {
        int row_index = tile_cols * i;
        for (int j = 0; j < tile_cols; ++j) // Tile Column
        {
            int k = row_index + j;
            t[k] = new Matrix(B[k]);
        }
    }
}

// Copy operator, which frees memory in this if non-empty before assignment
TileMatrix &TileMatrix::operator=(const TileMatrix &B)
{
    clear();
    tile_rows = B.tile_rows;
    tile_cols = B.tile_cols;
    matrix_rows = B.matrix_rows;
    matrix_cols = B.matrix_cols;
    t_len = tile_rows * tile_cols;
    t = (Matrix *)mkl_malloc(sizeof(Matrix) * t_len, INTEL_BB);
    for (int i = 0; i < tile_rows; ++i) // Tile Row
    {
        int row_index = tile_cols * i;
        for (int j = 0; j < tile_cols; ++j) // Tile Column
        {
            int k = row_index + j;
            t[k] = new Matrix(B[k]);
        }
    }
    return *this;
}

// Constructor: Split Matrix a into tileRows * tileCols tiles
TileMatrix::TileMatrix(const Matrix &B, const int &tileRows, const int &tileCols)
{
    if (tileRows <= 0 && B.rows % tileRows != 0) ErrorExit("TileMatrix::TileMatrix", BuildArgErrNE("Matrix B.rows % tileRows", B.rows % tileRows, "", 0));
    else if (tileCols <= 0 && B.cols % tileCols != 0) ErrorExit("TileMatrix::TileMatrix", BuildArgErrNE("Matrix B.cols % tileCols != 0", B.cols % tileCols != 0, "", 0));
    tile_rows = tileRows;
    tile_cols = tileCols;
    matrix_rows = B.rows / tileRows;
    matrix_cols = B.cols / tileCols;
    t_len = tile_rows * tile_cols;
    //cout << ToStringSize("") << "  t_len " << t_len << endl;
    t = (Matrix *)mkl_malloc(sizeof(Matrix) * t_len, INTEL_BB);

    for (int i = 0; i < tile_rows; ++i) // Tile Row
    {
        int row_index = tile_cols * i, B_row_i = matrix_rows * i;

        for (int j = 0; j < tile_cols; ++j) // Tile Column
        {
            //cout << "  i " << i << "  j " << j << endl;
            int col_index = row_index + j, B_col_i = matrix_cols * j;

            t[col_index] = new Matrix(matrix_rows, matrix_cols, true);

            for (int m_i = 0; m_i < matrix_rows; ++m_i) // Matrix Row
            {
                int m_row_index = matrix_cols * m_i;
                //cout << "TA(" << i << ", " << j << ", " << m_i << ", " << 0 << ") " << B(B_row_i + m_i, B_col_i) << endl;
                for (int m_j = 0; m_j < matrix_cols; ++m_j) // Matrix Column
                {
                    t[col_index][m_row_index + m_j] = B(B_row_i + m_i, B_col_i + m_j);
                }
            }
        }
    }
}

// Convert this TileMatrix to its non-tiled Matrix equivalent
Matrix TileMatrix::ToMatrix() const
{
    Matrix B(tile_rows * matrix_rows, tile_cols * matrix_cols, true);
    for (int i = 0; i < tile_rows; ++i) // Tile Row
    {
        int row_index = tile_cols * i, B_row_i = matrix_rows * i;

        for (int j = 0; j < tile_cols; ++j) // Tile Column
        {
            int col_index = row_index + j, B_col_i = matrix_cols * j;

            for (int m_i = 0; m_i < matrix_rows; ++m_i) // Matrix Row
            {
                int m_row_index = matrix_cols * m_i;
                //cout << "TA(" << i << ", " << j << ", " << m_i << ", " << 0 << ") " << t[col_index][m_row_index] << endl;
                for (int m_j = 0; m_j < matrix_cols; ++m_j) // Matrix Column
                {
                    B(B_row_i + m_i, B_col_i + m_j) = t[col_index][m_row_index + m_j];
                }
            }
        }
    }
    return B;
}

// Convert this to a Non-Tiled Matrix
TileMatrix::operator Matrix() const
{
    return ToMatrix();
}

// Destructor
TileMatrix::~TileMatrix()
{
    clear();
}

// Clear memory of t safely
void TileMatrix::clear()
{
    if (t_len > 0)
    {
        for (int i = 0; i < t_len; ++i)
        {
            t[i].clear();
        }
        if (t != NULL) mkl_free(t);
        tile_rows = tile_cols = matrix_rows = matrix_cols = t_len = 0;
        t = NULL;
    }
}

// 1D tile indexer
Matrix &TileMatrix::operator[](const int &i)
{
    if (i >= t_len) ErrorExit("TileMatrix::operator[]", BuildArgErrGE("Matrix index", i, "t_len", t_len));
    return t[i];
}

// 1D tile indexer
const Matrix &TileMatrix::operator[](const int &i) const
{
    if (i >= t_len) ErrorExit("TileMatrix::operator[]", BuildArgErrGE("Matrix index", i, "t_len", t_len));
    return t[i];
}

// 2D tile indexer
Matrix &TileMatrix::operator()(const int &tileRow, const int &tileCol)
{
    if (tileRow >= tile_rows) ErrorExit("TileMatrix::operator()", BuildArgErrGE("Matrix row index", tileRow, "tile_rows", tile_rows));
    else if (tileCol >= tile_cols) ErrorExit("TileMatrix::operator()", BuildArgErrGE("Matrix col index", tileCol, "tile_cols", tile_cols));
    return t[tileRow * tile_cols + tileCol];
}

// 2D tile indexer
const Matrix &TileMatrix::operator()(const int &tileRow, const int &tileCol) const
{
    if (tileRow >= tile_rows) ErrorExit("TileMatrix::operator()", BuildArgErrGE("Matrix row index", tileRow, "tile_rows", tile_rows));
    else if (tileCol >= tile_cols) ErrorExit("TileMatrix::operator()", BuildArgErrGE("Matrix col index", tileCol, "tile_cols", tile_cols));
    return t[tileRow * tile_cols + tileCol];
}

// 4D tile matrix indexer
double &TileMatrix::operator()(const int &tileRow, const int &tileCol, const int &matrixRow, const int &matrixCol)
{
    if (tileRow >= tile_rows) ErrorExit("TileMatrix::operator()", BuildArgErrGE("Matrix tileRow", tileRow, "tile_rows", tile_rows));
    else if (tileCol >= tile_cols) ErrorExit("TileMatrix::operator()", BuildArgErrGE("Matrix tileCol", tileCol, "tile_cols", tile_cols));
    else if (matrixRow >= matrix_rows) ErrorExit("TileMatrix::operator()", BuildArgErrGE("Matrix matrixRow", matrixRow, "matrix_rows", matrix_rows));
    else if (matrixCol >= matrix_cols) ErrorExit("TileMatrix::operator()", BuildArgErrGE("Matrix matrixCol", matrixCol, "matrix_cols", matrix_cols));
    return t[tileRow * tile_cols + tileCol][matrixRow * matrix_cols + matrixCol];
}

// 4D tile matrix indexer
const double &TileMatrix::operator()(const int &tileRow, const int &tileCol, const int &matrixRow, const int &matrixCol) const
{
    if (tileRow >= tile_rows) ErrorExit("TileMatrix::operator()", BuildArgErrGE("Matrix tileRow", tileRow, "tile_rows", tile_rows));
    else if (tileCol >= tile_cols) ErrorExit("TileMatrix::operator()", BuildArgErrGE("Matrix tileCol", tileCol, "tile_cols", tile_cols));
    else if (matrixRow >= matrix_rows) ErrorExit("TileMatrix::operator()", BuildArgErrGE("Matrix matrixRow", matrixRow, "matrix_rows", matrix_rows));
    else if (matrixCol >= matrix_cols) ErrorExit("TileMatrix::operator()", BuildArgErrGE("Matrix matrixCol", matrixCol, "matrix_cols", matrix_cols));
    return t[tileRow * tile_cols + tileCol][matrixRow * matrix_cols + matrixCol];
}

// Sets the upper triangle of this to value
TileMatrix &TileMatrix::SetUpper(const double &value /*= 0*/)
{
    for (int i = 0; i < tile_rows; ++i) // Tile Row
    {
        int row_index = tile_cols * i;

        for (int j = i; j < tile_cols; ++j) // Tile Column
        {
            int col_index = row_index + j;

            for (int m_i = 0; m_i < matrix_rows; ++m_i) // Matrix Row
            {
                int m_row_index = matrix_cols * m_i;

                for (int m_j = i == j ? m_i + 1 : 0; m_j < matrix_cols; ++m_j) // Matrix Column
                {
                    t[col_index].p[m_row_index + m_j] = value;
                }
            }
        }
    }
    return *this;
}

// Sets the lower triangle of this to value
TileMatrix &TileMatrix::SetLower(const double &value /*= 0*/)
{
    for (int i = 0; i < tile_rows; ++i) // Tile Row
    {
        int row_index = tile_cols * i;

        for (int j = 0; j <= i && j < tile_cols; ++j) // Tile Column
        {
            int col_index = row_index + j;

            for (int m_i = 0; m_i < matrix_rows; ++m_i) // Matrix Row
            {
                int m_row_index = matrix_cols * m_i;

                for (int m_j = 0; m_j <= i == j ? m_i + 1 : 0 && m_j < matrix_cols; ++m_j) // Matrix Column
                {
                    t[col_index].p[m_row_index + m_j] = value;
                }
            }
        }
    }
    return *this;

}

// Return a human-readable string that has the size properties of this (not data)
string TileMatrix::ToStringSize(string name) const
{
    ostringstream ss;
    if (!name.empty()) ss << name << " ";
    ss << "TileMatrix: ( " << tile_rows << " x " << tile_cols << " ) ( " << matrix_rows << " x " << matrix_cols << " ) RowMajor" << endl;
    return ss.str();
}

// Return a human-readable string that has the properties of this
string TileMatrix::ToString(string name, int width /*= 7*/, int precision /*= 0*/) const
{
    ostringstream ss;
    if (!name.empty()) ss << name << " ";
    ss << "TileMatrix: ( " << tile_rows << " x " << tile_cols << " ) ( " << matrix_rows << " x " << matrix_cols << " ) RowMajor" << endl;
    for (int i = 0; i < tile_rows; ++i) // Tile Row
    {
        int row_index = tile_cols * i;
        for (int m_i = 0; m_i < matrix_rows; ++m_i) // Matrix Row
        {
            int m_row_index = matrix_cols * m_i;
            for (int j = 0; j < tile_cols; ++j) // Tile Column
            {
                const Matrix &tile = t[row_index + j];

                for (int m_j = 0; m_j < matrix_cols; ++m_j) // Matrix Column
                {
                    if (width > 0) ss.width(width);
                    if (precision > 0) ss.precision(precision);
                    ss << tile[m_row_index + m_j] << " ";
                }
            }
            ss << endl;
        }
    }
    return ss.str();
}

// this(tileRow,tileCol).ToString('name(tileRow, tileCol)', width, precision)
string TileMatrix::ToStringSM(const int &tileRow, const int &tileCol, const string &name, int width /*= 7*/, int precision /*= 0*/) const
{
    const TileMatrix &A = *this;
    ostringstream ss;
    if (!name.empty()) ss << name;
    ss << "(" << tileRow << ", " << tileCol << ")";
    return A(tileRow, tileCol).ToString(ss.str(), width, precision);
}

// Determine whether this is a square TileMatrix (i.e. tile_rows == tile_cols && matrix_rows == matrix_cols)
bool  TileMatrix::IsSquare() const
{
    return t_len > 0 && tile_rows == tile_cols && matrix_rows == matrix_cols;
}

// Determine whether this is a symmetric TileMatrix
bool  TileMatrix::IsSymmetric() const
{
    if (IsSquare())
    {
        const double eps = 100 * Epsilon();
        for (int i = 0; i < tile_rows; ++i)
        {
            int row_index = tile_cols * i;
            for (int j = 0; j < tile_cols; ++j)
            {
                if (i == j)
                {
                    if (!t[row_index + j].IsSymmetric()) return false;
                }
                else
                {
                    if (t[row_index + j] != t[j * tile_cols + i]) return false;
                }
            }
        }
        return true;
    }
    else return false;
}

// Determine whether this is symmetric positive definite, which implies all of its eigenvalues are gt 0
bool  TileMatrix::IsSymmetricPositiveDefinite() const
{
    if (IsSymmetric())
    {
        Matrix D = SymmetricEigenValues();
        for (int i = 0; i < D.size(); ++i)
        {
            if (D.p[i] <= 0) return false;
        }
        return true;
    }
    else return false;
}

// Determine whether this is symmetric positive definite, which implies all of its eigenvalues are ge 0
bool  TileMatrix::IsSymmetricPositiveSemiDefinite() const
{
    if (IsSymmetric())
    {
        Matrix D = SymmetricEigenValues();
        for (int i = 0; i < D.size(); ++i)
        {
            if (D.p[i] < 0) return false;
        }
        return true;
    }
    else return false;
}

// Calculate the eigenvalues of this, assuming it is symmetric.  Uses decomposition A = Q * T * Q'
Matrix TileMatrix::SymmetricEigenValues() const
{
    Matrix A = ToMatrix();
    return A.SymmetricEigenValues();
}

// Calculate the eigenvalues of this, assuming it is non-symmetric
Matrix TileMatrix::NonSymmetricEigenValues() const
{
    Matrix A = ToMatrix();
    return A.NonSymmetricEigenValues();
}

// Calculate the determinant of this
double TileMatrix::Determinant() const
{
    if (t_len == 0) return 0;
    else
    {
        Matrix D = SymmetricEigenValues();
        double det = 1;
        for (int i = 0; i < D.size(); ++i) det *= D.p[i];
        return det;
    }
}

// TileMatrix Transpose
TileMatrix TileMatrix::operator~() const
{
    TileMatrix A(tile_cols, tile_rows, matrix_cols, matrix_rows);
    for (int i = 0; i < A.tile_rows; ++i)
    {
        int row_index = A.tile_cols * i;
        for (int j = 0; j < A.tile_cols; ++j)
        {
            A.t[j * A.tile_cols + i] = ~t[row_index + j]; // Same as A(j, i) = ~A(i, j)
        }
    }
    return A;
}

// Set absolute values of this that are less than 1e-11 to zero
TileMatrix &TileMatrix::SetNearZero()
{
    for (int i = 0; i < t_len; ++i)
    {
        t[i].SetNearZero();
    }
    return *this;
}

// Floating point precision for nearness calculations
double TileMatrix::Epsilon()
{
    return Matrix::Epsilon();
}

// Sum all values in this
double TileMatrix::Sum(int stride /*= 1*/) const
{
    double sum = 0;
    for (int i = 0; i < tile_rows; ++i) // Tile Row
    {
        int row_index = tile_cols * i;

        for (int j = 0; j < tile_cols; ++j) // Tile Column
        {
            int col_index = row_index + j;

            for (int m_i = 0; m_i < matrix_rows; ++m_i) // Matrix Row
            {
                int m_row_index = matrix_cols * m_i;
                for (int m_j = 0; m_j < matrix_cols; ++m_j) // Matrix Column
                {
                    sum += t[col_index][m_row_index + m_j];
                }
            }
        }
    }
    return sum;
}

// L1-norm (absolute difference) of p, with optional stride to skip values
double TileMatrix::L1Norm(int stride /*= 1*/) const
{
    double sum = 0;
    for (int i = 0; i < t_len; i += stride)
    {
        sum += t[i].L1Norm();
    }
    return sum;
}

// L2-norm (Euclidean) of p, with optional stride to skip values
double TileMatrix::L2Norm(int stride /*= 1*/) const
{
    return sqrt(Sum());
}

// Absolute sum of differences of this - B
double TileMatrix::AbsSumDiff(const TileMatrix &B) const
{
    if (tile_rows != B.tile_rows) ErrorExit("TileMatrix::AbsSumDiff", BuildArgErrNE("Matrix tile_rows", tile_rows, "B.tile_rows", B.tile_rows));
    else if (tile_cols != B.tile_cols) ErrorExit("TileMatrix::AbsSumDiff", BuildArgErrNE("Matrix tile_cols", tile_cols, "B.tile_cols", B.tile_cols));
    else if (matrix_rows != B.matrix_rows) ErrorExit("TileMatrix::AbsSumDiff", BuildArgErrNE("Matrix matrix_rows", matrix_rows, "B.matrix_rows", B.matrix_rows));
    else if (matrix_cols != B.matrix_cols) ErrorExit("TileMatrix::AbsSumDiff", BuildArgErrNE("Matrix matrix_cols", matrix_cols, "B.matrix_cols", B.matrix_cols));

    //cout << ToStringSize("A") << B.ToStringSize("B") << endl;
    double sum = 0;
    for (int i = 0; i < t_len; ++i)
    {
        const double diff = t[i].AbsSumDiff(B[i]);
        //if (diff > 1e-5) cout << " t[" << i << "] " << diff << "\n" << t[i].ToString("At") << B[i].ToString("Bt") << endl;
        sum += diff;
    }
    return sum;
}

// Maximum of all absolute (this - B) values
double TileMatrix::AbsMaxDiff(const TileMatrix &B) const
{
    if (tile_rows != B.tile_rows) ErrorExit("TileMatrix::AbsSumDiff", BuildArgErrNE("Matrix tile_rows", tile_rows, "B.tile_rows", B.tile_rows));
    else if (tile_cols != B.tile_cols) ErrorExit("TileMatrix::AbsSumDiff", BuildArgErrNE("Matrix tile_cols", tile_cols, "B.tile_cols", B.tile_cols));
    else if (matrix_rows != B.matrix_rows) ErrorExit("TileMatrix::AbsSumDiff", BuildArgErrNE("Matrix matrix_rows", matrix_rows, "B.matrix_rows", B.matrix_rows));
    else if (matrix_cols != B.matrix_cols) ErrorExit("TileMatrix::AbsSumDiff", BuildArgErrNE("Matrix matrix_cols", matrix_cols, "B.matrix_cols", B.matrix_cols));

    //cout << ToStringSize("A") << B.ToStringSize("B") << endl;
    double max = -DBL_MAX;
    for (int i = 0; i < t_len; ++i)
    {
        const double diff = t[i].AbsSumDiff(B[i]);
        //if (diff > 1e-5) cout << " t[" << i << "] " << diff << "\n" << t[i].ToString("At") << B[i].ToString("Bt") << endl;
        if (diff > max) max = diff;
    }
    return max;
}

// Perform operation this == B within (this - b).() lt Epsilon()
bool TileMatrix::operator==(const TileMatrix &B) const
{
    return AbsSumDiff(B) < Epsilon();
}

// Perform operation this == B within (this - b).() ge Epsilon()
bool TileMatrix::operator!=(const TileMatrix &B) const
{
    return AbsSumDiff(B) >= Epsilon();
}

// Perform operation this + B
TileMatrix TileMatrix::operator+(const TileMatrix &B) const
{
    if (tile_rows != B.tile_rows) ErrorExit("TileMatrix::operator+", BuildArgErrNE("Matrix tile_rows", tile_rows, "B.tile_rows", B.tile_rows));
    else if (tile_cols != B.tile_cols) ErrorExit("TileMatrix::operator+", BuildArgErrNE("Matrix tile_cols", tile_cols, "B.tile_cols", B.tile_cols));
    else if (matrix_rows != B.matrix_rows) ErrorExit("TileMatrix::operator+", BuildArgErrNE("Matrix matrix_rows", matrix_rows, "B.matrix_rows", B.matrix_rows));
    else if (matrix_cols != B.matrix_cols) ErrorExit("TileMatrix::operator+", BuildArgErrNE("Matrix matrix_cols", matrix_cols, "B.matrix_cols", B.matrix_cols));

    TileMatrix A(tile_rows, tile_cols, matrix_rows, matrix_cols);
    for (int i = 0; i < t_len; ++i) A.t[i] = t[i] + B.t[i];
    return A;
}

// Perform operation this += B
TileMatrix &TileMatrix::operator+=(const TileMatrix &B)
{
    if (tile_rows != B.tile_rows) ErrorExit("TileMatrix::operator+=", BuildArgErrNE("Matrix tile_rows", tile_rows, "B.tile_rows", B.tile_rows));
    else if (tile_cols != B.tile_cols) ErrorExit("TileMatrix::operator+=", BuildArgErrNE("Matrix tile_cols", tile_cols, "B.tile_cols", B.tile_cols));
    else if (matrix_rows != B.matrix_rows) ErrorExit("TileMatrix::operator+=", BuildArgErrNE("Matrix matrix_rows", matrix_rows, "B.matrix_rows", B.matrix_rows));
    else if (matrix_cols != B.matrix_cols) ErrorExit("TileMatrix::operator+=", BuildArgErrNE("Matrix matrix_cols", matrix_cols, "B.matrix_cols", B.matrix_cols));
    for (int i = 0; i < t_len; ++i) t[i] += B.t[i];
    return *this;
}

// Perform operation this - B
TileMatrix TileMatrix::operator-(const TileMatrix &B) const
{
    if (tile_rows != B.tile_rows) ErrorExit("TileMatrix::operator-", BuildArgErrNE("Matrix tile_rows", tile_rows, "B.tile_rows", B.tile_rows));
    else if (tile_cols != B.tile_cols) ErrorExit("TileMatrix::operator-", BuildArgErrNE("Matrix tile_cols", tile_cols, "B.tile_cols", B.tile_cols));
    else if (matrix_rows != B.matrix_rows) ErrorExit("TileMatrix::operator-", BuildArgErrNE("Matrix matrix_rows", matrix_rows, "B.matrix_rows", B.matrix_rows));
    else if (matrix_cols != B.matrix_cols) ErrorExit("TileMatrix::operator-", BuildArgErrNE("Matrix matrix_cols", matrix_cols, "B.matrix_cols", B.matrix_cols));

    TileMatrix A(tile_rows, tile_cols, matrix_rows, matrix_cols);
    for (int i = 0; i < t_len; ++i) A.t[i] = t[i] - B.t[i];
    return A;
}

// Perform operation this -= B
TileMatrix &TileMatrix::operator-=(const TileMatrix &B)
{
    if (tile_rows != B.tile_rows) ErrorExit("TileMatrix::operator-=", BuildArgErrNE("Matrix tile_rows", tile_rows, "B.tile_rows", B.tile_rows));
    else if (tile_cols != B.tile_cols) ErrorExit("TileMatrix::operator-=", BuildArgErrNE("Matrix tile_cols", tile_cols, "B.tile_cols", B.tile_cols));
    else if (matrix_rows != B.matrix_rows) ErrorExit("TileMatrix::operator-=", BuildArgErrNE("Matrix matrix_rows", matrix_rows, "B.matrix_rows", B.matrix_rows));
    else if (matrix_cols != B.matrix_cols) ErrorExit("TileMatrix::operator-=", BuildArgErrNE("Matrix matrix_cols", matrix_cols, "B.matrix_cols", B.matrix_cols));

    for (int i = 0; i < t_len; ++i) t[i] -= B.t[i];
    return *this;
}

// Perform operation this * B
TileMatrix TileMatrix::operator*(const TileMatrix &B) const
{
    if (tile_cols != B.tile_rows) ErrorExit("TileMatrix::operator*", BuildArgErrNE("Matrix tile_cols", tile_cols, "B.tile_rows", B.tile_rows));
    else if (matrix_cols != B.matrix_rows) ErrorExit("TileMatrix::operator*", BuildArgErrNE("Matrix matrix_cols", matrix_cols, "B.matrix_rows", B.matrix_rows));

    return TileMatrix(ToMatrix() * B.ToMatrix(), tile_rows, B.tile_cols);
}

// Perform operation this * B or ~B if B_transpose is true
TileMatrix TileMatrix::Multiply(const TileMatrix &B, bool B_transpose /*= false*/) const
{
    if (tile_cols != B.tile_rows) ErrorExit("TileMatrix::Multiply", BuildArgErrNE("Matrix tile_cols", tile_cols, "B.tile_rows", B.tile_rows));
    else if (matrix_cols != B.matrix_rows) ErrorExit("TileMatrix::Multiply", BuildArgErrNE("Matrix matrix_cols", matrix_cols, "B.matrix_rows", B.matrix_rows));

    return TileMatrix(ToMatrix().Multiply(B.ToMatrix(), B_transpose), tile_rows, B.tile_cols);
}

/*
* Calculate an MKL Non-Tiled (if tile_rows and tile_cols = 1) or Tiled (if tile_rows or tile_cols > 1) Cholesky Decomposition where this = L * ~L
*
* @param L                  Input matrix to decompose, and set to output on exit
* @param numTasks           Number of parallel tasks for tiled decomposition, else 0 for number of sockets
* @param numThreadsPerTask  Number of parallel threads per task, else 0 for number of cores/socket
* @return                   Number of seconds kernel took to run
*/
double TileMatrix::Cholesky(TileMatrix &L, const int &numTasks, const int &numThreadsPerTask)
{
    if (!L.IsSquare()) ErrorExit("TileMatrix::Cholesky", BuildArgErrNE("TileMatrix tile_rows", L.tile_rows, "tile_cols", L.tile_cols));
    const int total_threads = numTasks * numThreadsPerTask, num_tasks = numTasks > 0 ? numTasks : 1;
    const double sec_start = TimeNowInSeconds();
    if (L.tile_size() > 1)
    {
        // Tiled Implementation
        if (numThreadsPerTask > 0) mkl_set_num_threads(numThreadsPerTask);
        for (int tile_col = 0; tile_col < L.tile_cols; ++tile_col)
        {
            Matrix &L_col_col = L(tile_col, tile_col);
            LAPACKE_dpotrf(LAPACK_ROW_MAJOR, 'L', L.matrix_cols, L_col_col.p, L.matrix_rows);
            //cout << L.ToStringSM(tile_col, tile_col, "A: L") << endl;

#pragma omp parallel for num_threads(num_tasks) shared(tile_col, L)
            for (int tile_row = tile_col + 1; tile_row < L.tile_rows; ++tile_row)
            {
                // Solve Ax = alpha B where alpha = 1 and A is tile L(tile_col, tile_col) and B is L(tile_row, tile_col)
                cblas_dtrsm(CblasColMajor, CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, L.matrix_cols, L.matrix_cols, 1.0,
                    L_col_col.p, L.matrix_rows,              // A matrix to solve for x in A x = B
                    L(tile_row, tile_col).p, L.matrix_rows); // B matrix, solution put here
                //cout << L.ToStringSM(tile_row, tile_col, "B: L") << endl;
            }
#pragma omp parallel for num_threads(num_tasks) shared(tile_col, L)
            for (int tile_row = tile_col + 1; tile_row < L.tile_rows; ++tile_row)
            {
                Matrix &L_row_col = L(tile_row, tile_col);
                // Symmetric Rank-k Update C = alpha AA' + beta C, where alpha = beta = 1
                cblas_dsyrk(CblasRowMajor, CblasLower, CblasNoTrans, L.matrix_cols, L.matrix_cols, -1.0,
                    L_row_col.p, L.matrix_rows, 1.0,         // A matrix
                    L(tile_row, tile_row).p, L.matrix_rows); // C matrix, solution put here in lower triangle
                //cout << L.ToStringSM(tile_row, tile_row, "C: L") << endl;

                for (int i = tile_row + 1; i < L.tile_rows; ++i)
                {
                    // Matrix multiply alpha A B + beta C, where alpha = beta = 1
                    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, L.matrix_cols, L.matrix_cols, L.matrix_cols, -1.0,
                        L(i, tile_col).p, L.matrix_rows,  // A matrix
                        L_row_col.p, L.matrix_rows, 1.0,  // B matrix
                        L(i, tile_row).p, L.matrix_rows); // C matrix, solution put here
                    //cout << L.ToStringSM(i, tile_row, "D: L") << endl;
                }
            }
        }
        L.SetUpper(); // Set upper triangular part of L to 0, since cholesky decomposition above only sets lower part
    }
    else
    {
        // Non-Tiled Implementation
        L = TileMatrix(L.ToMatrix().Cholesky(total_threads), L.tile_rows, L.tile_cols);
    }
    return TimeNowInSeconds() - sec_start;
}

/*
* Calculate an MKL Non-Tiled (if tile_rows and tile_cols = 1) or Tiled (if tile_rows or tile_cols > 1) Cholesky Decomposition where this = L * ~L
*
* @param numTasks           Number of parallel tasks for tiled decomposition, else 0 for number of sockets
* @param numThreadsPerTask  Number of parallel threads per task, else 0 for number of cores/socket
*/
TileMatrix TileMatrix::Cholesky(const int &numTasks /*= 0*/, const int &numThreadsPerTask /*= 0*/) const
{
    if (!IsSquare()) ErrorExit("TileMatrix::Cholesky", BuildArgErrNE("TileMatrix tile_rows", tile_rows, "tile_cols", tile_cols));
    const int total_threads = numTasks * numThreadsPerTask, num_tasks = numTasks > 0 ? numTasks : 1;
    if (tile_size() > 1)
    {
        // Tiled Implementation
        if (numThreadsPerTask > 0) mkl_set_num_threads(numThreadsPerTask);
        TileMatrix L = *this; // Make a copy, since it modified inplace below
        for (int tile_col = 0; tile_col < tile_cols; ++tile_col)
        {
            Matrix &L_col_col = L(tile_col, tile_col);
            LAPACKE_dpotrf(LAPACK_ROW_MAJOR, 'L', L.matrix_cols, L_col_col.p, L.matrix_rows);
            //cout << L.ToStringSM(tile_col, tile_col, "A: L") << endl;

#pragma omp parallel for num_threads(num_tasks) shared(tile_col, L)
            for (int tile_row = tile_col + 1; tile_row < tile_rows; ++tile_row)
            {
                // Solve Ax = alpha B where alpha = 1 and A is tile L(tile_col, tile_col) and B is L(tile_row, tile_col)
                cblas_dtrsm(CblasColMajor, CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, L.matrix_cols, L.matrix_cols, 1.0,
                    L_col_col.p, L.matrix_rows,              // A matrix to solve for x in A x = B
                    L(tile_row, tile_col).p, L.matrix_rows); // B matrix, solution put here
                //cout << L.ToStringSM(tile_row, tile_col, "B: L") << endl;
            }
#pragma omp parallel for num_threads(num_tasks) shared(tile_col, L)
            for (int tile_row = tile_col + 1; tile_row < tile_rows; ++tile_row)
            {
                Matrix &L_row_col = L(tile_row, tile_col);
                // Symmetric Rank-k Update C = alpha AA' + beta C, where alpha = beta = 1
                cblas_dsyrk(CblasRowMajor, CblasLower, CblasNoTrans, L.matrix_cols, L.matrix_cols, -1.0,
                    L_row_col.p, L.matrix_rows, 1.0,         // A matrix
                    L(tile_row, tile_row).p, L.matrix_rows); // C matrix, solution put here in lower triangle
                //cout << L.ToStringSM(tile_row, tile_row, "C: L") << endl;

                for (int i = tile_row + 1; i < tile_rows; ++i)
                {
                    // Matrix multiply alpha A B + beta C, where alpha = beta = 1
                    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, L.matrix_cols, L.matrix_cols, L.matrix_cols, -1.0,
                        L(i, tile_col).p, L.matrix_rows,  // A matrix
                        L_row_col.p, L.matrix_rows, 1.0,  // B matrix
                        L(i, tile_row).p, L.matrix_rows); // C matrix, solution put here
                    //cout << L.ToStringSM(i, tile_row, "D: L") << endl;
                }
            }
        }
        return L.SetUpper(); // Set upper triangular part of L to 0, since cholesky decomposition above only sets lower part
    }
    else
    {
        // Non-Tiled Implementation
        return TileMatrix(ToMatrix().Cholesky(total_threads), tile_rows, tile_cols);
    }
}


/*
* Calculate an MKL Non-Tiled 1D Fast Fourier Transform (FFT) across every row of this, outputing the complex magnitude
*
* @param AB                 Input matrix to compute FFT, and set to output on exit
* @param numTasks           Number of parallel tasks for tiled decomposition, else 0 for number of sockets
* @param numThreadsPerTask  Number of parallel threads per task, else 0 for number of cores/socket
* @return                   Number of seconds kernel took to run, else -1 on error
*/
double TileMatrix::FFT1DMagRows(TileMatrix &AB, const int &numTasks, const int &numThreadsPerTask)
{
    if (AB.rows() < 1 || AB.cols() < 2)
    {
        cerr << "  Error in TileMatrix::FFFT1DMagRows: Must have AB.rows() " << AB.rows() << " > 0 and AB.cols() " << AB.cols() << " > 1" << endl;
        return -1;
    }
    else
    {
        Matrix A = AB.ToMatrix(), B;
        const double sec_start = TimeNowInSeconds();
        DFTI_DESCRIPTOR_HANDLE dfti_desc;
        MKL_LONG status = DftiCreateDescriptor(&dfti_desc, DFTI_DOUBLE, DFTI_REAL, 1, A.cols);
        if (status != DFTI_NO_ERROR) cerr << "  Error in TileMatrix::FFFT1DMagRows: Could not DftiCreateDescriptor, status " << status << endl;
        else
        {
            if (numThreadsPerTask > 0) DftiSetValue(dfti_desc, DFTI_THREAD_LIMIT, numThreadsPerTask);
            DftiSetValue(dfti_desc, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
            DftiSetValue(dfti_desc, DFTI_COMPLEX_STORAGE, DFTI_COMPLEX_COMPLEX);
            DftiSetValue(dfti_desc, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX); // Implies DFTI_PACKED_FORMAT is DFTI_CCE_FORMAT
            DftiSetValue(dfti_desc, DFTI_PACKED_FORMAT, DFTI_CCS_FORMAT);
            status = DftiCommitDescriptor(dfti_desc);
            if (status != DFTI_NO_ERROR) cerr << "  Error in TileMatrix::FFFT1DMagRows: Could not DftiCommitDescriptor, status " << status << endl;
            else
            {
                B = Matrix(A.rows, A.cols);
                //cout << ToStringSize("A") << B.ToStringSize("B") << endl;
                MKL_Complex16 *T = (MKL_Complex16*)MKL_malloc(A.cols * sizeof(MKL_Complex16), INTEL_BB);

#pragma omp parallel for num_threads(numTasks) shared(dfti_desc, B, T)
                for (int row = 0; row < A.rows; ++row)
                {
                    const int row_index = A.cols * row;
                    DftiComputeForward(dfti_desc, &A.p[row_index], T); // 1D FFT for Row matrix_row
                    vzAbs(A.cols, T, &B.p[row_index]); // Calculate complex magnitude
                }
                mkl_free(T);
                DftiFreeDescriptor(&dfti_desc);
                B.SetNearZero();
            }
        }
        AB = TileMatrix(B, AB.tile_rows, AB.tile_cols);
        return TimeNowInSeconds() - sec_start;
    }
}


/*
* Calculate an MKL Non-Tiled 1D Fast Fourier Transform (FFT) across every row of this, outputing the complex magnitude
*
* @param numTasks           Number of parallel tasks, else 0 for number of sockets
* @param numThreadsPerTask  Number of parallel threads per task, else 0 for number of cores
*/
TileMatrix TileMatrix::FFT1DMagRows(const int &numTasks /*= 0*/, const int &numThreadsPerTask /*= 0*/) const
{
    if (rows() < 1 || cols() < 2)
    {
        cerr << "  Error in TileMatrix::FFFT1DMagRows: Must have rows() " << rows() << " > 0 and cols() " << cols() << " > 1" << endl;
        return TileMatrix();
    }
    else
    {
        Matrix A = ToMatrix(), B;
        DFTI_DESCRIPTOR_HANDLE dfti_desc;
        MKL_LONG status = DftiCreateDescriptor(&dfti_desc, DFTI_DOUBLE, DFTI_REAL, 1, A.cols);
        if (status != DFTI_NO_ERROR) cerr << "  Error in TileMatrix::FFFT1DMagRows: Could not DftiCreateDescriptor, status " << status << endl;
        else
        {
            if (numThreadsPerTask > 0) DftiSetValue(dfti_desc, DFTI_THREAD_LIMIT, numThreadsPerTask);
            DftiSetValue(dfti_desc, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
            DftiSetValue(dfti_desc, DFTI_COMPLEX_STORAGE, DFTI_COMPLEX_COMPLEX);
            DftiSetValue(dfti_desc, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX); // Implies DFTI_PACKED_FORMAT is DFTI_CCE_FORMAT
            DftiSetValue(dfti_desc, DFTI_PACKED_FORMAT, DFTI_CCS_FORMAT);
            status = DftiCommitDescriptor(dfti_desc);
            if (status != DFTI_NO_ERROR) cerr << "  Error in TileMatrix::FFFT1DMagRows: Could not DftiCommitDescriptor, status " << status << endl;
            else
            {
                B = Matrix(A.rows, A.cols);
                //cout << ToStringSize("A") << B.ToStringSize("B") << endl;
                MKL_Complex16 *T = (MKL_Complex16*)MKL_malloc(A.cols * sizeof(MKL_Complex16), INTEL_BB);

#pragma omp parallel for num_threads(numTasks) shared(dfti_desc, B, T)
                for (int row = 0; row < A.rows; ++row)
                {
                    const int row_index = A.cols * row;
                    DftiComputeForward(dfti_desc, &A.p[row_index], T); // 1D FFT for Row matrix_row
                    vzAbs(A.cols, T, &B.p[row_index]); // Calculate complex magnitude
                }
                mkl_free(T);
                DftiFreeDescriptor(&dfti_desc);
                B.SetNearZero();
            }
        }
        return TileMatrix(B, tile_rows, tile_cols);
    }
}


/*
* Calculate an MKL Non-Tiled 2D Fast Fourier Transform (FFT) across every row of this, outputing the complex magnitude
*
* @param AB                 Input matrix to compute FFT, and set to output on exit
* @param numTasks           Number of parallel tasks for tiled decomposition, else 0 for number of sockets
* @param numThreadsPerTask  Number of parallel threads per task, else 0 for number of cores/socket
* @return                   Number of seconds kernel took to run, else -1 on error
*/
double TileMatrix::FFT2DMagRows(TileMatrix &AB, const int &numTasks, const int &numThreadsPerTask)
{
    if (AB.rows() < 1 || AB.cols() < 2)
    {
        cerr << "  Error in TileMatrix::FFFT2DMagRows: Must have AB.rows " << AB.rows() << " > 0 and AB.cols " << AB.cols() << " > 1" << endl;
        return -1;
    }
    else
    {
        DFTI_DESCRIPTOR_HANDLE dfti_desc;
        const double sec_start = TimeNowInSeconds();
        MKL_LONG fft_dim[2] = { AB.matrix_rows, AB.matrix_cols };
        MKL_LONG status = DftiCreateDescriptor(&dfti_desc, DFTI_DOUBLE, DFTI_REAL, 2, fft_dim);
        if (status != DFTI_NO_ERROR) cerr << "  Error in TileMatrix::FFFT2DMagRows: Could not DftiCreateDescriptor, status " << status << endl;
        else
        {
            if (numThreadsPerTask > 0) DftiSetValue(dfti_desc, DFTI_THREAD_LIMIT, numThreadsPerTask);
            DftiSetValue(dfti_desc, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
            DftiSetValue(dfti_desc, DFTI_COMPLEX_STORAGE, DFTI_COMPLEX_COMPLEX);
            DftiSetValue(dfti_desc, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX); // Implies DFTI_PACKED_FORMAT is DFTI_CCE_FORMAT
            DftiSetValue(dfti_desc, DFTI_PACKED_FORMAT, DFTI_CCS_FORMAT);
            status = DftiCommitDescriptor(dfti_desc);
            if (status != DFTI_NO_ERROR) cerr << "  Error in TileMatrix::FFFT2DMagRows: Could not DftiCommitDescriptor, status " << status << endl;
            else
            {
                const int T_len = AB.matrix_size();
                MKL_Complex16 *T = (MKL_Complex16*)MKL_malloc(T_len * sizeof(MKL_Complex16), INTEL_BB);

#pragma omp parallel for num_threads(numTasks) shared(dfti_desc, AB, T, T_len)
                for (int tile_row = 0; tile_row < AB.tile_rows; ++tile_row)
                {
                    const int tile_row_index = AB.tile_cols * tile_row;
                    for (int tile_col = 0; tile_col < AB.tile_cols; ++tile_col)
                    {
                        const int tile_col_index = tile_row_index + tile_col;
                        DftiComputeForward(dfti_desc, &AB.t[tile_col_index].p, T);
                        vzAbs(T_len, T, &AB[tile_col_index].p[0]); // Calculate complex magnitude
                    }
                }
                mkl_free(T);
                DftiFreeDescriptor(&dfti_desc);
                AB.SetNearZero();
            }
        }
        return TimeNowInSeconds() - sec_start;
    }
}


/*
* Calculate an MKL Tiled 2D Fast Fourier Transform (FFT) across every tile of this, outputing the complex magnitude
*
* @param numTasks           Number of parallel tasks, else 0 for number of sockets
* @param numThreadsPerTask  Number of parallel threads per task, else 0 for number of cores
*/
TileMatrix TileMatrix::FFT2DMagRows(const int &numTasks /*= 0*/, const int &numThreadsPerTask /*= 0*/) const
{
    TileMatrix B;
    if (rows() < 1 || cols() < 2)
    {
        cerr << "  Error in TileMatrix::FFFT2DMagRows: Must have rows " << rows() << " > 0 and cols " << cols() << " > 1" << endl;
    }
    else
    {
        DFTI_DESCRIPTOR_HANDLE dfti_desc;
        B = TileMatrix(tile_rows, tile_cols, matrix_rows, matrix_cols); //cout << ToStringSize("A") << B.ToStringSize("B") << endl;
        MKL_LONG fft_dim[2] = { B.matrix_rows, B.matrix_cols };
        MKL_LONG status = DftiCreateDescriptor(&dfti_desc, DFTI_DOUBLE, DFTI_REAL, 2, fft_dim);
        if (status != DFTI_NO_ERROR) cerr << "  Error in TileMatrix::FFFT2DMagRows: Could not DftiCreateDescriptor, status " << status << endl;
        else
        {
            if (numThreadsPerTask > 0) DftiSetValue(dfti_desc, DFTI_THREAD_LIMIT, numThreadsPerTask);
            DftiSetValue(dfti_desc, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
            DftiSetValue(dfti_desc, DFTI_COMPLEX_STORAGE, DFTI_COMPLEX_COMPLEX);
            DftiSetValue(dfti_desc, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX); // Implies DFTI_PACKED_FORMAT is DFTI_CCE_FORMAT
            DftiSetValue(dfti_desc, DFTI_PACKED_FORMAT, DFTI_CCS_FORMAT);
            status = DftiCommitDescriptor(dfti_desc);
            if (status != DFTI_NO_ERROR) cerr << "  Error in TileMatrix::FFFT2DMagRows: Could not DftiCommitDescriptor, status " << status << endl;
            else
            {
                const int T_len = B.matrix_size();
                MKL_Complex16 *T = (MKL_Complex16*)MKL_malloc(T_len * sizeof(MKL_Complex16), INTEL_BB);

#pragma omp parallel for num_threads(numTasks) shared(dfti_desc, B, T, T_len)
                for (int tile_row = 0; tile_row < tile_rows; ++tile_row)
                {
                    const int tile_row_index = tile_cols * tile_row;
                    for (int tile_col = 0; tile_col < tile_cols; ++tile_col)
                    {
                        const int tile_col_index = tile_row_index + tile_col;
                        DftiComputeForward(dfti_desc, &t[tile_col_index].p, T);
                        vzAbs(T_len, T, &B[tile_col_index].p[0]); // Calculate complex magnitude
                    }
                }
                mkl_free(T);
                DftiFreeDescriptor(&dfti_desc);
                B.SetNearZero();
            }
        }
    }
    return B;
}
