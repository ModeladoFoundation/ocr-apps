/*
Intel Base Kernel Library: TileMatrix class

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#pragma once
#include <Intel_Base.h>
#include <Matrix.h>
#include <math.h>
using namespace std;

// 2D Array of Matrix tiles container that supports row and column major format for the tiles
class TileMatrix
{
private:
    int t_len; // tile_rows * tile_cols

public:
    int tile_rows, tile_cols, matrix_rows, matrix_cols;
    Matrix *t; // Vector that contains tiles in row major format

    TileMatrix();
    TileMatrix(const int &tileRows, const int &tileCols, const int &matrixRows, const int &matrixCols); // tileRows * tileCols tiles, each tile is a Matrix(matrixRows, matrixCols)
    TileMatrix(const Matrix &B, const int &tileRows, const int &tileCols); // Split Matrix a into tileRows * tileCols tiles
    TileMatrix(const TileMatrix &B);
    ~TileMatrix();

    int rows() const { return tile_rows * matrix_rows; } // Number of row element values in this
    int cols() const { return tile_cols * matrix_cols; } // Number of column element values in this
    int size() const { return t_len * matrix_rows * matrix_cols; } // Count of all element values in this
    int tile_size() const { return tile_rows * tile_cols; } // Number of tiles in this
    int matrix_size() const { return matrix_rows * matrix_cols; } // Number of elements in tile
    void clear();

    Matrix &operator[](const int &i);
    const Matrix &operator[](const int &i) const;

    Matrix &operator()(const int &tileRow, const int &tileCol);
    const Matrix &operator()(const int &tileRow, const int &tileCol) const;

    double &operator()(const int &tileRow, const int &tileCol, const int &matrixRow, const int &matrixCol);
    const double &operator()(const int &tileRow, const int &tileCol, const int &matrixRow, const int &matrixCol) const;

    TileMatrix &SetUpper(const double &value = 0); // Sets the upper triangle of this to value
    TileMatrix &SetLower(const double &value = 0); // Sets the lower triangle of this to value

    Matrix ToMatrix() const; // Convert this TileMatrix to its non-tiled Matrix equivalent
    operator Matrix() const;

    string ToStringSize(string name) const;
    string ToString(string name, int width = 7, int precision = 0) const;
    string ToStringSM(const int &tileRow, const int &tileCol, const string &name, int width = 7, int precision = 0) const; // this(tileRow,tileCol).ToString('name(tileRow, tileCol)', width, precision)

    bool IsSquare() const; // Determine whether this is a square TileMatrix (i.e. tile_rows == tile_cols && matrix_rows == matrix_cols)
    bool IsSymmetric() const; // Determine whether this is a symmetric TileMatrix
    bool IsSymmetricPositiveDefinite() const; // Determine whether this is symmetric positive definite, which implies all of its eigenvalues are gt 0
    bool IsSymmetricPositiveSemiDefinite() const; // Determine whether this is symmetric positive definite, which implies all of its eigenvalues are ge 0
    Matrix SymmetricEigenValues() const; // Calculate the eigenvalues of this, assuming it is symmetric
    Matrix NonSymmetricEigenValues() const; // Calculate the eigenvalues of this, assuming it is non-symmetric
    double Determinant() const; // Calculate the determinant of this
    TileMatrix operator~() const; // Transpose
    TileMatrix &operator=(const TileMatrix &B);
    TileMatrix &SetNearZero(); // Set absolute values of this that are less than 1e-11 to zero
    static double Epsilon(); // Floating point precision for nearness calculations
    double Sum(int stride = 1) const; // Sum all values in this
    double L1Norm(int stride = 1) const; // Sum all absolute values in this
    double L2Norm(int stride = 1) const; // Sum all values in this, then take sqrt
    double AbsSumDiff(const TileMatrix &B) const; // Sum all absolute (this - B) values
    double AbsMaxDiff(const TileMatrix &B) const; // Maximum of all absolute (this - B) values
    bool operator==(const TileMatrix &B) const;
    bool operator!=(const TileMatrix &B) const;
    TileMatrix operator+(const TileMatrix &B) const;
    TileMatrix operator-(const TileMatrix &B) const;
    TileMatrix operator*(const TileMatrix &B) const;
    TileMatrix &operator+=(const TileMatrix &B);
    TileMatrix &operator-=(const TileMatrix &B);
    TileMatrix Multiply(const TileMatrix &B, bool B_transpose = false) const; // Perform operation this * B or ~B if B_transpose is true

    static double Cholesky(TileMatrix &L, const int &numTasks = 0, const int &numThreadsPerTask = 0);
    static double FFT1DMagRows(TileMatrix &AB, const int &numTasks = 0, const int &numThreadsPerTask = 0);
    static double FFT2DMagRows(TileMatrix &AB, const int &numTasks = 0, const int &numThreadsPerTask = 0);

    TileMatrix Cholesky(const int &numTasks = 0, const int &numThreadsPerTask = 0) const;
    TileMatrix FFT1DMagRows(const int &numTasks = 0, const int &numThreadsPerTask = 0) const;
    TileMatrix FFT2DMagRows(const int &numTasks = 0, const int &numThreadsPerTask = 0) const;
};
