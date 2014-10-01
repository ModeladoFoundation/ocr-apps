/*
Intel Base Kernel Library: Matrix class

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#include "GU.h"
#include "Matrix.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <omp.h>
#include <mkl.h>
using namespace Intel_Base;
using namespace std;

// MKL version 11.0 update 1 does not yet include this function
#ifndef mkl_calloc
double* MKL_calloc(int num, size_t size, int align) /* Allocate the aligned buffer */
{
    double*cal;
    cal = (double *)mkl_malloc(size, align);
    for (int i=0; i < num; ++i) cal[i] = 0;  // Initialize memory buffer to zeros
    return cal;
}
#define mkl_calloc MKL_calloc
#endif

// Constructor
Matrix::Matrix()
{
    rows = cols = p_len = 0;
    row_order = true;
    p = NULL;
}

// Constructor
Matrix::Matrix(int numRows, int numCols, bool rowOrder /*= true*/)
{
    rows = numRows;
    cols = numCols;
    p_len = rows * cols;
    row_order = rowOrder;
    p = p_len > 0 ? (double *)mkl_calloc(p_len, sizeof(double), INTEL_BB) : NULL;
}

// Constructor
Matrix::Matrix(double *a, int numRows /*= 0*/, int numCols /*= 0*/, bool rowOrder /*= true*/)
{
    rows = numRows;
    cols = numCols;
    p_len = rows * cols;
    row_order = rowOrder;
    p = p_len > 0 ? (double *)mkl_malloc(sizeof(double)* p_len, INTEL_BB) : NULL;
    for (int i = 0; i < p_len; ++i) p[i] = a[i];
}

// Constructor
Matrix::Matrix(const Matrix &B)
{
    rows = B.rows;
    cols = B.cols;
    p_len = rows * cols;
    row_order = B.row_order;
    p = p_len > 0 ? (double *)mkl_malloc(sizeof(double)* p_len, INTEL_BB) : NULL;
    for (int i = 0; i < p_len; ++i) p[i] = B.p[i];
}

// Copy operator, which frees memory in this if non-empty before assignment
Matrix &Matrix::operator=(const Matrix &B)
{
    //clear();
    rows = B.rows;
    cols = B.cols;
    p_len = rows * cols;
    row_order = B.row_order;
    p = p_len > 0 ? (double *)mkl_malloc(sizeof(double)* p_len, INTEL_BB) : NULL;
    for (int i = 0; i < p_len; ++i) p[i] = B.p[i];
    return *this;
}

// Constructor
Matrix::Matrix(const Matrix *a)
{
    rows = a->rows;
    cols = a->cols;
    p_len = rows * cols;
    row_order = a->row_order;
    p = p_len > 0 ? (double *)mkl_malloc(sizeof(double)* p_len, INTEL_BB) : NULL;
    for (int i = 0; i < p_len; ++i) p[i] = a->p[i];
}

// Constructor
Matrix::Matrix(string filename, int numRows /*= 0*/, int numCols /*= 0*/, bool rowOrder /*= true*/)
{
    rows = cols = p_len = 0;
    p = NULL;
    ReadFile(filename, numRows, numCols, rowOrder);
}

// Destructor
Matrix::~Matrix()
{
    clear();
}

// Clear memory of p safely
void Matrix::clear()
{
    if (p_len > 0 && p != NULL) mkl_free(p);
    rows = cols = p_len = 0;
    row_order = true;
    p = NULL;
}

// 1D matrix indexer
double &Matrix::operator[](const int &i)
{
    if (i >= p_len) ErrorExit("Matrix::operator[]", BuildArgErrGE("Matrix index", i, "p_len", p_len));
    return p[i];
}

// 1D matrix indexer
const double &Matrix::operator[](const int &i) const
{
    if (i >= p_len) ErrorExit("Matrix::operator[]", BuildArgErrGE("Matrix index", i, "p_len", p_len));
    return p[i];
}

// 2D matrix indexer
double &Matrix::operator()(const int &matrixRow, const int &matrixCol)
{
    if (matrixRow >= rows) ErrorExit("Matrix::operator()", BuildArgErrGE("Matrix row index", matrixRow, "rows", rows));
    else if (matrixCol >= cols) ErrorExit("Matrix::operator()", BuildArgErrGE("Matrix col index", matrixCol, "cols", cols));
    return row_order ? p[matrixRow * cols + matrixCol] : p[matrixCol * rows + matrixRow];
}

// 2D matrix indexer
const double &Matrix::operator()(const int &matrixRow, const int &matrixCol) const
{
    if (matrixRow >= rows) ErrorExit("Matrix::operator()", BuildArgErrGE("Matrix row index", matrixRow, "rows", rows));
    else if (matrixCol >= cols) ErrorExit("Matrix::operator()", BuildArgErrGE("Matrix col index", matrixCol, "cols", cols));
    return row_order ? p[matrixRow * cols + matrixCol] : p[matrixCol * rows + matrixRow];
}

// Sets the upper triangle of this to value
Matrix &Matrix::SetUpper(const double &value /*= 0*/)
{
    for (int i = 0; i < rows; ++i)
    {
        int row_index = cols * i;
        for (int j = i + 1; j < cols; ++j)
        {
            p[row_index + j] = value;
        }
    }
    return *this;
}

// Sets the lower triangle of this to value
Matrix &Matrix::SetLower(const double &value /*= 0*/)
{
    for (int i = 0; i < rows; ++i)
    {
        int row_index = cols * i;
        for (int j = 0; j < i - 1 && j < cols; ++j)
        {
            p[row_index + j] = value;
        }
    }
    return *this;

}

/*
 * Read a text file with a header line with the number of rows and columns, row_order bool, followed by each matrix row with column values separated by a space.
 * If numRows and numCols gt 0, then use rowOrder and assume there is no header row in filename (i.e. rows cols row_order).
 */
double *Matrix::ReadFile(string filename, int numRows /*= 0*/, int numCols /*= 0*/, bool rowOrder /*= true*/)
{
    ifstream in(filename.c_str());
    if (in.is_open())
    {
        if (numRows == 0 && numCols == 0)
        {
            string smat, open_paren, x_sep, close_paren, m_major;
            in >> smat >> open_paren >> rows >> x_sep >> cols >> close_paren >> m_major;
            if (smat == "Matrix:" && open_paren == "(" && x_sep == "x" && close_paren == ")" && (m_major == "RowMajor" || m_major == "ColMajor"))
            {
                row_order = m_major == "RowMajor";
            }
            else
            {
                cerr << "smat " << smat << "  open_paren " << open_paren << "  rows " << rows << "  x_sep " << x_sep << "  cols " << cols
                    << "  close_paren " << close_paren << "  m_major " << m_major << endl;
                throw new runtime_error("ReadFile: Invalid Matrix header format (must be 'Matrix: ( # x # ) {RowMajor|ColMajor}', where # x # is Rows x Cols");
            }
        }
        else
        {
            rows = numRows;
            cols = numCols;
            row_order = rowOrder;
        }
        clear();
        p = (double *)mkl_calloc(rows * cols, sizeof(double), INTEL_BB);
        if (row_order)
        {
            for (int i = 0; i < rows; ++i)
            {
                int row_index = cols * i;
                for (int j = 0; j < cols; ++j)
                {
                    in >> p[row_index + j];
                }
            }
        }
        else
        {
            for (int j = 0; j < cols; ++j)
            {
                int col_index = rows * j;
                for (int i = 0; i < rows; ++i)
                {
                    in >> p[col_index + i];
                }
            }
        }
        in.close();
        return p;
    }
    else return NULL;
}

/*
 * Write a text file with a header line with the number of rows and columns, row_order bool, followed by each matrix row with column values separated by a space
 */
bool Matrix::WriteFile(string filename) const
{
    if (p == NULL || rows <= 0 || cols <= 0)
    {
        cerr << "WriteFile: Error, matrix is empty" << endl; return false;
    }
    ofstream out(filename.c_str());
    if (out.is_open())
    {
        out << "Matrix: ( " << rows << " x " << cols << " ) " << (row_order ? "RowMajor" : "ColMajor") << endl;
        if (row_order)
        {
            for (int i = 0; i < rows; ++i)
            {
                int row_index = cols * i;
                for (int j = 0; j < cols; ++j)
                {
                    out << p[row_index + j] << " ";
                }
                out << endl;
            }
        }
        else
        {
            for (int j = 0; j < cols; ++j)
            {
                int col_index = rows * j;
                for (int i = 0; i < rows; ++i)
                {
                    out << p[col_index + i] << " ";
                }
                out << endl;
            }
        }
        out.close();
        return true;
    }
    else return false;
}

// Return a human-readable string that has the size properties of this (not data)
string Matrix::ToStringSize(string name) const
{
    ostringstream ss;
    if (!name.empty()) ss << name << " ";
    ss << "Matrix: ( " << rows << " x " << cols << " ) " << (row_order ? "RowMajor" : "ColMajor") << endl;
    return ss.str();
}

// Return a human-readable string that has the properties of this
string Matrix::ToString(string name, int width /*= 7*/, int precision /*= 0*/) const
{
    ostringstream ss;
    if (!name.empty()) ss << name << " ";
    ss << "Matrix: ( " << rows << " x " << cols << " ) " << (row_order ? "RowMajor" : "ColMajor") << endl;
    if (row_order)
    {
        for (int i = 0; i < rows; ++i)
        {
            int row_index = cols * i;
            for (int j = 0; j < cols; ++j)
            {
                if (width > 0) ss.width(width);
                if (precision > 0) ss.precision(precision);
                ss << p[row_index + j] << " ";
            }
            ss << endl;
        }
    }
    else
    {
        for (int j = 0; j < cols; ++j)
        {
            int col_index = rows * j;
            for (int i = 0; i < rows; ++i)
            {
                if (width > 0) ss.width(width);
                if (precision > 0) ss.precision(precision);
                ss << p[col_index + i] << " ";
            }
            ss << endl;
        }
    }
    return ss.str();
}

// Return a diagonal square matrix of order N
Matrix Matrix::Diag(const int &N)
{
    Matrix A(N, N, true);
    for (int i = 0; i < N; ++i) A(i, i) = i + 1;
    return A;
}

// Return a symmetric Pascal matrix, formed with binomial coefficients
Matrix Matrix::Pascal(const int &N)
{
    // Binomial (n r) = n! / (r!(n - r)!), where n = i + j and r = i
    // So, (i+j i) = (i + j)! / (i!j!)
    Matrix A(N, N);
    double i_fact = 1;
    for (int i = 0; i < A.rows; ++i)
    {
        if (i > 1) i_fact *= i;

        double j_fact = 1, ij_fact = i_fact;
        int row_index = A.cols * i;
        for (int j = 0; j <= i; ++j)
        {
            if (j > 0) ij_fact *= i + j;
            if (j > 1) j_fact *= j;
            //cout << "i " << i << " j " << j << "  i_fact " << i_fact << "  j_fact " << j_fact << "  ij_fact " << ij_fact << " " << Intel_Base::Factorial(i + j) << "  bin " << (ij_fact / (i_fact * j_fact)) << endl; 
            if (i == 0 || j == 0) A.p[row_index + j] = A.p[j * A.cols + i] = 1;
            else if (i == 1 || j == 1) A.p[row_index + j] = A.p[j * A.cols + i] = i + 1;
            else A.p[row_index + j] = A.p[j * A.cols + i] = ij_fact / (i_fact * j_fact);
        }
    }
    return A;
}

// Return a random uniform discrete integer matrix in [a, b)
Matrix Matrix::RandUniformInteger(const int &numRows, const int &numCols, const int &a /*= 0*/, const int &b /*= INT_MAX*/, const int &seed /*= 44*/)
{
    Matrix A(numRows, numCols);
    VSLStreamStatePtr ss_ptr;
    vslNewStream(&ss_ptr, VSL_BRNG_SFMT19937, seed);
    int *p = new int[A.p_len];
    int status = viRngUniform(VSL_RNG_METHOD_UNIFORM_STD, ss_ptr, A.p_len, p, a, b);
    if (status != VSL_STATUS_OK) cerr << "Error in vRngUniform " << status << " error" << endl;
    else for (int i = 0; i < A.p_len; ++i) A.p[i] = p[i];
    delete[] p;
    vslDeleteStream(&ss_ptr);
    return A;
}

// Return a random uniform matrix
Matrix Matrix::RandUniform(const int &numRows, const int &numCols, const double &a /*= 0*/, const double &b /*= 1*/, const int &seed /*= 44*/)
{
    Matrix A(numRows, numCols);
    VSLStreamStatePtr ss_ptr;
    vslNewStream(&ss_ptr, VSL_BRNG_SFMT19937, seed);
    int status = vdRngUniform(VSL_RNG_METHOD_UNIFORM_STD, ss_ptr, A.p_len, A.p, a, b);
    if (status != VSL_STATUS_OK) cerr << "Error in vdRngUniform " << status << " error" << endl;
    vslDeleteStream(&ss_ptr);
    return A;
}

// Return a random Gaussian matrix in with mean and standard deviation std
Matrix Matrix::RandGaussian(const int &numRows, const int &numCols, const double &mean /*= 0*/, const double &std /*= 1*/, const int &seed /*= 44*/)
{
    Matrix A(numRows, numCols);
    VSLStreamStatePtr ss_ptr;
    vslNewStream(&ss_ptr, VSL_BRNG_SFMT19937, seed);
    int status = vdRngGaussian(VSL_RNG_METHOD_GAUSSIAN_BOXMULLER, ss_ptr, A.p_len, A.p, mean, std);
    if (status != VSL_STATUS_OK) cerr << "Error in vdRngGaussian " << status << " error" << endl;
    vslDeleteStream(&ss_ptr);
    return A;
}

// Return a random Gaussian matrix in with mean and standard deviation std
Matrix Matrix::RandGaussianSPD(const int &N, const double &mean /*= 0*/, const double &std /*= 1*/, int seed /*= 44*/)
{
    Matrix D = Diag(N), A = RandGaussian(N, N, mean, std, seed);
    for (int i = 0; i < A.rows; ++i)
    {
        int row_index = A.cols * i;
        for (int j = i; j < A.cols; ++j)
        {
            if (i == j) A.p[row_index + j] = i + 1;
            else if (i < 1 && j >= A.cols - 1) A.p[row_index + j] = A.p[j * A.cols + i] = 0;
            else // Assign lower triangle to upper triangle
            {
                A.p[row_index + j] = A.p[j * A.cols + i];
            }
        }
    }
    return A;
}

// Determine whether this is a square matrix (i.e. rows == cols)
bool  Matrix::IsSquare() const
{
    return p_len > 0 && rows == cols;
}

// Determine whether this is a symmetric matrix
bool  Matrix::IsSymmetric() const
{
    if (IsSquare())
    {
        const double eps = Epsilon();
        for (int i = 0; i < rows; ++i)
        {
            int row_index = cols * i;
            for (int j = 0; j < cols; ++j)
            {
                if (i != j)
                {
                    double diff = p[row_index + j] - p[j * cols + i];
                    if (diff > eps || diff < -eps) return false;
                }
            }
        }
        return true;
    }
    else return false;
}

// Determine whether this is symmetric positive definite, which implies all of its eigenvalues are gt 0
bool  Matrix::IsSymmetricPositiveDefinite() const
{
    if (IsSymmetric())
    {
        Matrix D = SymmetricEigenValues();
        for (int i = 0; i < D.p_len; ++i)
        {
            if (D.p[i] <= 0) return false;
        }
        return true;
    }
    else return false;
}

// Determine whether this is symmetric positive definite, which implies all of its eigenvalues are ge 0
bool  Matrix::IsSymmetricPositiveSemiDefinite() const
{
    if (IsSymmetric())
    {
        Matrix D = SymmetricEigenValues();
        for (int i = 0; i < D.p_len; ++i)
        {
            if (D.p[i] < 0) return false;
        }
        return true;
    }
    else return false;
}

// Calculate the eigenvalues of this, assuming it is symmetric.  Uses decomposition A = Q * T * Q'
Matrix Matrix::SymmetricEigenValues() const
{
    Matrix A(row_order ? this : ~*this); // Set copy of this to A, and make sure ends up in RowMajor order
    Matrix D(1, A.cols), E(1, A.cols - 1), T(1, A.cols);
    int status = LAPACKE_dsytrd(A.row_order ? LAPACK_ROW_MAJOR : LAPACK_COL_MAJOR, 'L', A.cols, A.p, A.rows, D.p, E.p, T.p);
    if (status != 0) cerr << "Error in LAPACKE_dsytrd " << status << " error" << endl;

    status = LAPACKE_dsterf(T.cols, D.p, E.p);
    if (status != 0) cerr << "Error in LAPACKE_dsterf " << status << " error" << endl;
    return D;
}

// Calculate the eigenvalues of this, assuming it is non-symmetric
Matrix Matrix::NonSymmetricEigenValues() const
{
    Matrix A(row_order ? this : ~*this); // Set copy of this to A, and make sure ends up in RowMajor order
    int ilo = 1, ihi = A.cols;
    Matrix scale(1, A.cols);
    int status = LAPACKE_dgebal(A.row_order ? LAPACK_ROW_MAJOR : LAPACK_COL_MAJOR, 'N', A.cols, A.p, A.rows, &ilo, &ihi, scale.p);
    if (status != 0) cerr << "Error in LAPACKE_dgebal " << status << " error" << endl;

    Matrix T(1, A.cols - 1);
    status = LAPACKE_dgehrd(A.row_order ? LAPACK_ROW_MAJOR : LAPACK_COL_MAJOR, A.cols, ilo, ihi, A.p, A.rows, T.p);
    if (status != 0) cerr << "Error in LAPACKE_dgehrd " << status << " error" << endl;

    Matrix WR(1, A.cols), WI(1, A.cols);
    status = LAPACKE_dhseqr(A.row_order ? LAPACK_ROW_MAJOR : LAPACK_COL_MAJOR, 'E', 'N', A.cols, ilo, ihi, A.p, A.rows, WR.p, WI.p, NULL, A.cols);
    if (status != 0) cerr << "Error in LAPACKE_dhseqr " << status << " error" << endl;
    return WR; // TODO: Notice we only returning real parts here, so might need to return complex double instead
}

// Calculate the determinant of this
double Matrix::Determinant() const
{
    if (p_len == 0) return 0;
    else
    {
        Matrix D = SymmetricEigenValues();
        double det = 1;
        for (int i = 0; i < D.p_len; ++i) det *= D.p[i];
        return det;
    }
}

// Matrix Transpose
Matrix Matrix::operator~() const
{
    Matrix A(cols, rows, row_order);
    if (row_order)
    {
        for (int i = 0; i < rows; ++i)
        {
            int row_index = cols * i;
            for (int j = 0; j < cols; ++j)
            {
                A.p[j * cols + i] = p[row_index + j]; // Same as A(j, i) =
            }
        }
    }
    else
    {
        for (int j = 0; j < cols; ++j)
        {
            int col_index = rows * j;
            for (int i = 0; i < rows; ++i)
            {
                A.p[i * rows + j] = p[col_index + i]; // Same as A(j, i) =
            }
        }
    }
    return A;
}

// Set absolute values of this that are less than 1e-11 to zero
Matrix &Matrix::SetNearZero()
{
    for (int i = 0; i < p_len; ++i)
    {
        if (p[i] < 1e-11 && p[i] > -1e-11) p[i] = 0;
    }
    return *this;
}

// Floating point precision for nearness calculations
double Matrix::Epsilon()
{
    return 1e-9;
}

// L1-norm (absolute difference) of p, with optional stride to skip values, equivalent to cblas_dnrm2(p_len, p, stride)
double Matrix::L1Norm(int stride /*= 1*/) const
{
    double sum = 0;
    for (int i = 0; i < p_len; i += stride)
    {
        sum += p[i] >= 0 ? p[i] : -p[i]; // Absolute value
    }
    return sum;
}

// L2-norm (Euclidean) of p, with optional stride to skip values, equivalent to cblas_dnrm2(p_len, p, stride)
double Matrix::L2Norm(int stride /*= 1*/) const
{
    return cblas_dnrm2(p_len, p, stride);
}

// Absolute sum of differences of this - B
double Matrix::AbsSumDiff(const Matrix &B) const
{
    if (rows != B.rows) ErrorExit("Matrix::AbsSumDiff", BuildArgErrNE("Matrix rows", rows, "B.rows", B.rows));
    else if (cols != B.cols) ErrorExit("Matrix::AbsSumDiff", BuildArgErrNE("Matrix cols", cols, "B.cols", B.cols));
    else if (row_order != B.row_order) ErrorExit("Matrix::AbsSumDiff", "Matrices must have same row_order.");

    double sum = 0;
    for (int i = 0; i < p_len; ++i)
    {
        double diff = p[i] - B.p[i];
        sum += diff >= 0 ? diff : -diff; // Absolute value
    }
    return sum;
}

// Maximum of all absolute (this - B) values
double Matrix::AbsMaxDiff(const Matrix &B) const
{
    if (rows != B.rows) ErrorExit("Matrix::AbsMaxDiff", BuildArgErrNE("Matrix rows", rows, "B.rows", B.rows));
    else if (cols != B.cols) ErrorExit("Matrix::AbsMaxDiff", BuildArgErrNE("Matrix cols", cols, "B.cols", B.cols));
    else if (row_order != B.row_order) ErrorExit("Matrix::AbsMaxDiff", "Matrices must have same row_order.");

    double max = -DBL_MAX;
    for (int i = 0; i < p_len; ++i)
    {
        double diff = p[i] - B.p[i];
        if (diff > max) max = diff;
    }
    return max;
}

// Perform operation this == B within (this - b).() lt Epsilon()
bool Matrix::operator==(const Matrix &B) const
{
    return AbsSumDiff(B) < Epsilon();
}

// Perform operation this == B within (this - b).() ge Epsilon()
bool Matrix::operator!=(const Matrix &B) const
{
    return AbsSumDiff(B) >= Epsilon();
}

// Perform operation this + B
Matrix Matrix::operator+(const Matrix &B) const
{
    if (rows >= B.rows) ErrorExit("Matrix::operator+", BuildArgErrGE("Matrix rows", rows, "B.rows", B.rows));
    else if (cols >= B.cols) ErrorExit("Matrix::operator+", BuildArgErrGE("Matrix cols", cols, "B.cols", B.cols));

    Matrix A(rows, cols, row_order);
    for (int i = 0; i < p_len; ++i) A.p[i] = p[i] + B.p[i];
    return A;
}

// Perform operation this += B
Matrix Matrix::operator+=(const Matrix &B)
{
    if (rows != B.rows) ErrorExit("Matrix::operator+=", BuildArgErrNE("Matrix rows", rows, "B.rows", B.rows));
    else if (cols != B.cols) ErrorExit("Matrix::operator+=", BuildArgErrNE("Matrix cols", cols, "B.cols", B.cols));

    for (int i = 0; i < p_len; ++i) p[i] += B.p[i];
    return this;
}

// Perform operation this - B
Matrix Matrix::operator-(const Matrix &B) const
{
    if (rows != B.rows) ErrorExit("Matrix::operator-", BuildArgErrNE("Matrix rows", rows, "B.rows", B.rows));
    else if (cols != B.cols) ErrorExit("Matrix::operator-", BuildArgErrNE("Matrix cols", cols, "B.cols", B.cols));

    Matrix A(rows, cols, row_order);
    for (int i = 0; i < p_len; ++i) A.p[i] = p[i] - B.p[i];
    return A;
}

// Perform operation this -= B
Matrix Matrix::operator-=(const Matrix &B)
{
    if (rows != B.rows) ErrorExit("Matrix::operator-=", BuildArgErrNE("Matrix rows", rows, "B.rows", B.rows));
    else if (cols != B.cols) ErrorExit("Matrix::operator-=", BuildArgErrNE("Matrix cols", cols, "B.cols", B.cols));

    for (int i = 0; i < p_len; ++i) p[i] -= B.p[i];
    return this;
}

// Perform operation this * B
Matrix Matrix::operator*(const Matrix &B) const
{
    if (cols != B.rows) ErrorExit("Matrix::operator*", BuildArgErrNE("Matrix cols", cols, "B.rows", B.rows));

    Matrix C(rows, B.cols, row_order); // DGEMM: alpha * op(A) * op(B) + beta*C
    cblas_dgemm(C.row_order ? CblasRowMajor : CblasColMajor, CblasNoTrans,
        B.row_order == row_order ? CblasNoTrans : CblasTrans,
        C.rows, C.cols, cols, 1.0,
        p, rows,
        B.p, B.row_order == row_order ? cols : C.cols,
        0.0, C.p, C.rows);
    return C;
}

// Perform operation this * B or ~B if B_transpose is true
Matrix Matrix::Multiply(const Matrix &B, bool B_transpose /*= false*/) const
{
    if (cols != B.rows) ErrorExit("Matrix::Multiply", BuildArgErrNE("Matrix cols", cols, "B.rows", B.rows));

    Matrix C(rows, B.cols, row_order); // DGEMM: alpha * op(A) * op(B) + beta*C
    cblas_dgemm(C.row_order ? CblasRowMajor : CblasColMajor, CblasNoTrans,
        B.row_order == row_order != B_transpose ? CblasNoTrans : CblasTrans,
        C.rows, C.cols, cols, 1.0,
        p, rows,
        B.p, B.row_order == row_order ? cols : C.cols,
        0.0, C.p, C.rows);
    return C;
}

/*
 * Calculate an MKL Non-Tiled Cholesky Decomposition where A = L * ~L
 *
 * @param numThreads     Number of parallel threads for decomposition, else 0 for number of cores
 */
Matrix Matrix::Cholesky(const int &numThreads /*= 0*/) const
{
    if (rows != cols) ErrorExit("Matrix::Cholesky", BuildArgErrNE("Matrix rows", rows, "cols", cols));
    Matrix A(row_order ? this : ~*this); // Set copy of this to A, and make sure ends up in RowMajor order
    int info = 0, total_threads = numThreads > 0 ? numThreads : 1;

    if (total_threads > 0) mkl_set_num_threads(total_threads);
    info = LAPACKE_dpotrf(A.row_order ? LAPACK_ROW_MAJOR : LAPACK_COL_MAJOR, 'L', A.cols, A.p, A.rows);
    if (info < 0) cerr << "Error in LAPACKE_dpotrf " << -info << "th argument" << endl;
    else if (info > 0) cerr << "Error in LAPACKE_dpotrf where " << info << "th minor prevented factoriation" << endl;
    return A.SetUpper(); // Set upper triangular part of A to 0, since dpotrf only sets lower part
}

/*
* Calculate an MKL Non-Tiled 1D Fast Fourier Transform (FFT) across every row of this, outputing the complex magnitude
*
* @param numTasks           Number of parallel tasks, else 0 for number of sockets
* @param numThreadsPerTask  Number of parallel threads per task, else 0 for number of cores
*/
Matrix Matrix::FFT1DMagRows(const int &numTasks /*= 0*/, const int &numThreadsPerTask /*= 0*/) const
{
    Matrix B;
    if (rows < 1 || cols < 2)
    {
        cerr << "  Error in Matrix::FFFT1DMagRows: Must have rows " << rows << " > 0 and cols " << cols << " > 1" << endl;
    }
    else
    {
        DFTI_DESCRIPTOR_HANDLE dfti_desc;
        MKL_LONG status = DftiCreateDescriptor(&dfti_desc, DFTI_DOUBLE, DFTI_REAL, 1, cols);
        if (status != DFTI_NO_ERROR) cerr << "  Error in Matrix::FFFT1DMagRows: Could not DftiCreateDescriptor, status " << status << endl;
        else
        {
            if (numThreadsPerTask > 0) DftiSetValue(dfti_desc, DFTI_THREAD_LIMIT, numThreadsPerTask);
            DftiSetValue(dfti_desc, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
            DftiSetValue(dfti_desc, DFTI_COMPLEX_STORAGE, DFTI_COMPLEX_COMPLEX);
            DftiSetValue(dfti_desc, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX); // Implies DFTI_PACKED_FORMAT is DFTI_CCE_FORMAT
            DftiSetValue(dfti_desc, DFTI_PACKED_FORMAT, DFTI_CCS_FORMAT);
            status = DftiCommitDescriptor(dfti_desc);
            if (status != DFTI_NO_ERROR) cerr << "  Error in Matrix::FFFT1DMagRows: Could not DftiCommitDescriptor, status " << status << endl;
            else
            {
                B = Matrix(rows, cols);
                //cout << ToStringSize("A") << B.ToStringSize("B") << endl;
                MKL_Complex16 *T = (MKL_Complex16*)MKL_malloc(cols * sizeof(MKL_Complex16), INTEL_BB);

#pragma omp parallel for num_threads(numTasks) shared(dfti_desc, B, T)
                for (int row = 0; row < rows; ++row)
                {
                    const int row_index = cols * row;
                    DftiComputeForward(dfti_desc, &p[row_index], T); // 1D FFT for Row matrix_row
                    vzAbs(cols, T, &B.p[row_index]); // Calculate complex magnitude
                }
                mkl_free(T);
                DftiFreeDescriptor(&dfti_desc);
                B.SetNearZero();
            }
        }
    }
    return B;
}


/*
* Calculate an MKL Non-Tiled 2D Fast Fourier Transform (FFT) across every tile of this, outputing the complex magnitude
*
* @param numTasks           Number of parallel tasks, else 0 for number of sockets
* @param numThreadsPerTask  Number of parallel threads per task, else 0 for number of cores
*/
Matrix Matrix::FFT2DMagRows(const int &numTasks /*= 0*/, const int &numThreadsPerTask /*= 0*/) const
{
    Matrix B;
    if (rows < 1 || cols < 2)
    {
        cerr << "  Error in TileMatrix::Matrix: Must have rows " << rows << " > 0 and cols " << cols << " > 1" << endl;
    }
    else
    {
        DFTI_DESCRIPTOR_HANDLE dfti_desc;
        B = Matrix(rows, cols); //cout << ToStringSize("A") << B.ToStringSize("B") << endl;
        MKL_LONG fft_dim[2] = { B.rows, B.cols };
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
            if (status != DFTI_NO_ERROR) cerr << "  Error in Matrix::FFFT2DMagRows: Could not DftiCommitDescriptor, status " << status << endl;
            else
            {
                const int T_len = B.size();
                MKL_Complex16 *T = (MKL_Complex16*)MKL_malloc(T_len * sizeof(MKL_Complex16), INTEL_BB);

                DftiComputeForward(dfti_desc, p, T);

#pragma omp parallel for num_threads(numTasks) shared(B, T)
                for (int row = 0; row < rows; ++row)
                {
                    const int row_index = cols * row;
                    vzAbs(cols, &T[row_index], &B.p[row_index]); // Calculate complex magnitude
                }

                mkl_free(T);
                DftiFreeDescriptor(&dfti_desc);
                B.SetNearZero();
            }
        }
    }
    return B;
}
