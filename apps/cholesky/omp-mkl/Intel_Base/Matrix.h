/*
Intel Base Kernel Library: Matrix class

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#pragma once
#include <Intel_Base.h>
#include <limits.h>
#include <string>
#ifndef _WIN32
#include <float.h>
#endif
using namespace std;

// 2D MKL double Matrix container that supports row and column major format
class Matrix
{
private:
    int p_len; // rows * cols

public:
    bool row_order;
    int rows, cols;
    double *p; // Vector that contains matrix data in row major order if row_order is true, else column major order

    Matrix();
    Matrix(int numRows, int numCols, bool rowOrder = true);
    Matrix(double *a, int numRows = 0, int numCols = 0, bool rowOrder = true);
    Matrix(const Matrix &B);
    Matrix(const Matrix *B);
    Matrix(string filename, int numRows = 0, int numCols = 0, bool rowOrder = true);
    ~Matrix();

    int size() const { return p_len; }
    void clear();

    double &operator[](const int &i);
    const double &operator[](const int &i) const;

    double &operator()(const int &matrixRow, const int &matrixCol);
    const double &operator()(const int &matrixRow, const int &matrixCol) const;

    Matrix &SetUpper(const double &value = 0); // Sets the upper triangle of this to value
    Matrix &SetLower(const double &value = 0); // Sets the lower triangle of this to value

    double *ReadFile(string filename, int numRows = 0, int numCols = 0, bool rowOrder = true);
    bool WriteFile(string filename) const;
    string ToStringSize(string name) const;
    string ToString(string name, int width = 7, int precision = 0) const;
    
    static Matrix Diag(const int &N); // Return a diagonal square matrix of order N
    static Matrix Pascal(const int &N); // Return a symmetric Pascal matrix, formed with binomial coefficients
    static Matrix RandUniformInteger(const int &numRows, const int &numCols, const int &a = 0, const int &b = INT_MAX, const int &seed = 44);// Return a random uniform discrete integer matrix in [a, b)
    static Matrix RandUniform(const int &numRows, const int &numCols, const double &a = 0, const double &b = 1, const int &seed = 44); // Return a random uniform matrix in [a, b)
    static Matrix RandGaussian(const int &numRows, const int &numCols, const double &mean = 0, const double &std = 1, const int &seed = 44); // Return a random Gaussian matrix in with mean and standard deviation std
    static Matrix RandGaussianSPD(const int &N, const double &mean = 0, const double &std = 1, int seed = 44); // Return a random Gaussian matrix in with mean and standard deviation std
    bool IsSquare() const; // Determine whether this is a square matrix (i.e. rows == cols)
    bool IsSymmetric() const; // Determine whether this is a symmetric matrix
    bool IsSymmetricPositiveDefinite() const; // Determine whether this is symmetric positive definite, which implies all of its eigenvalues are gt 0
    bool IsSymmetricPositiveSemiDefinite() const; // Determine whether this is symmetric positive definite, which implies all of its eigenvalues are ge 0
    Matrix SymmetricEigenValues() const; // Calculate the eigenvalues of this, assuming it is symmetric
    Matrix NonSymmetricEigenValues() const; // Calculate the eigenvalues of this, assuming it is non-symmetric
    double Determinant() const; // Calculate the determinant of this
    Matrix operator~() const; // Transpose
    Matrix &operator=(const Matrix &B);
    static double Epsilon(); // Floating point precision for nearness calculations
    Matrix &SetNearZero(); // Set absolute values of this that are less than 1e-11 to zero
    double Sum(int stride = 1) const; // Sum all values in this
    double L1Norm(int stride = 1) const; // Sum all absolute values in this
    double L2Norm(int stride = 1) const; // Sum all values in this, then take sqrt
    double AbsSumDiff(const Matrix &B) const; // Sum all absolute (this - B) values
    double AbsMaxDiff(const Matrix &B) const; // Maximum of all absolute (this - B) values
    bool operator==(const Matrix &B) const;
    bool operator!=(const Matrix &B) const;
    Matrix operator+(const Matrix &B) const;
    Matrix operator-(const Matrix &B) const;
    Matrix operator*(const Matrix &B) const;
    Matrix operator+=(const Matrix &B);
    Matrix operator-=(const Matrix &B);
    Matrix Multiply(const Matrix &B, bool B_transpose = false) const; // Perform operation this * B or ~B if B_transpose is true

    Matrix Cholesky(const int &numThreads = 0) const;
    Matrix FFT1DMagRows(const int &numTasks = 0, const int &numThreadsPerTask = 0) const;
    Matrix FFT2DMagRows(const int &numTasks = 0, const int &numThreadsPerTask = 0) const;
};

