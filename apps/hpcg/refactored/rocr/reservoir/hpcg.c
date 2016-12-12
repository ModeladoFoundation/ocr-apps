// FF2 rights

/**
 * This is the core CG computation performed in HPCG. It excludes the use of
 * the MG preconditioning. In HPCG, the sparse matrix are encoded in CSR
 *
 * Some parts of the original code have been commented out because they are
 * specifically hard to deal with. Ideally, we should however be able to deal
 * with them.
 */

#include "micro_kernels.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATAFILE "./data/hpcg_cg_compressed.dat"
#define LEVELS 4
#define N {1124864, 140608, 17576, 2197}
#define M {104, 52, 26, 13} // must be N^1/3 and M[i+1] must be M[i] / 2
#define NNZ {29791000, 3652264, 438976, 50653}
#define TOLERANCE 0.0
#define MAX_ITER 50

#define MAXN 1124864
#define MAXNNZ 29791000
#define PRESMOOTHER 1
#define POSTSMOOTHER 1

// in HPCG, there are never more than 27 NNZ entries per row
#define MNZPR 27

// 'A' for every level
static int * restrict *nnzPerRow;
static int (* restrict *colInds)[MNZPR];
static double (* restrict *A)[MNZPR];
static double * restrict *A_diag;

static double *b;
static double * restrict *z;   // Preconditioned residual vector
static double * restrict *r;   // residual per level
static double * restrict *Axf;
static double normr;
static double normr0;
static int * restrict *f2c;
static double *x;
static double *x_check;

// sizing per level
static const int nPerLvl[LEVELS] = N;
static const int nnzPerLvl[LEVELS] = NNZ;
static const int blockSzPerLvl[LEVELS] = M;

// temp vectors for the finest level
static double *p;    // Direction vector
static double *Ap;

static double normr_check;
static double normr0_check;

// local mappable sub functions
void mapped_step1(double *x, double *normr);
void mapped_step2(int i);
void mapped_step3(int i);
void mapped_step4(double *beta, double *rtz, double oldrtz);
void mapped_step5(double *alpha, double rtz, double *pAp, double *x,
    double *normr);
void step1(double *x, double *normr);
void step2(int i);
void step3(int i);
void step4(double *beta, double *rtz, double oldrtz);
void step5(double *alpha, double rtz, double *pAp, double *x,
    double *normr);

/**
 * Reads a matrix in our custom compressed format.
 *
 * The expected format of the data is as follow:
 * - 3 ints: nb rows, nb columns, nb non zeros
 * - an unsigned char: the number of contiguous ranges of non-zero values in
 *    the considered row or 255
 * - a list of pairs of ints: inclusive first and last column index for every
 *    range of contiguous non zero element in the row. If the previous value is
 *    255, this is the same as for the previous row except that every index is
 *    incremented by one.
 * The last two fields are repeated for each row. The values are -1.0 except on
 * the diagonal, where it is 26.0.
 *
 * Pointers must point to allocated memory. The first parameters are used for
 * verification purposes only, they are values expected to be found in the file.
 */
static void readMatrix(FILE *fd, int nbRows, int nbCols, int nnz,
    int *nnzPerRow, int (*colInds)[MNZPR], double (*A)[MNZPR], double *A_diag)
{
    unsigned int mRows = 0, mCols = 0, mNnz = 0;
    unsigned int i, j;

    fread(&mRows, sizeof(mRows), 1, fd);
    fread(&mCols, sizeof(mCols), 1, fd);
    fread(&mNnz, sizeof(mNnz), 1, fd);

    if (mRows != nbRows || mCols != nbCols || mNnz != nnz) {
        fprintf(stderr, "Invalid data file format\n");
        abort();
    }

    for (i = 0; i < mRows; ++i) {
        unsigned char nranges;
        if (fread(&nranges, sizeof(nranges), 1, fd) < 1) {
            fprintf(stderr, "Unexpected EOF\n");
            abort();
        }
        if (nranges > 28 && nranges != 255) {
            fprintf(stderr, "Unexpected number of ranges: %u\n", nranges);
            abort();
        }
        if (nranges != 255) {
            unsigned char r;
            unsigned char curNnz = 0;

            int rangeVals[28];
            if (fread(rangeVals, sizeof(*rangeVals), 2 * nranges, fd) <
                2 * nranges)
            {
                fprintf(stderr, "Unexpected EOF\n");
                abort();
            }
            for (r = 0; r < nranges; r++) {
                int beg = rangeVals[2 * r];
                int end = rangeVals[2 * r + 1];
                int colInd;
                for (colInd = beg; colInd <= end; ++colInd, ++curNnz) {
                    colInds[i][curNnz] = colInd;
                    if (colInd == i) {
                        A[i][curNnz] = 26.0;
                        A_diag[i] = 26.0;
                    } else {
                        A[i][curNnz] = -1.0;
                    }
                }
            }
            if (curNnz > 27) {
                fprintf(stderr, "Unexpected number of non zeros: %d\n", curNnz);
                abort();
            }
            nnzPerRow[i] = curNnz;
        } else {
            unsigned char curNnz;
            if (i == 0) {
                fprintf(stderr, "First line cannot be compressed in dataset\n");
                abort();
            }
            nnzPerRow[i] = nnzPerRow[i - 1];
            for (curNnz = 0; curNnz < nnzPerRow[i]; ++curNnz) {
                colInds[i][curNnz] = colInds[i - 1][curNnz] + 1;
                if (colInds[i][curNnz] == i) {
                    A[i][curNnz] = 26.0;
                    A_diag[i] = 26.0;
                } else {
                    A[i][curNnz] = -1.0;
                }
            }
        }

        for (j = nnzPerRow[i]; j < MNZPR; ++j) {
            colInds[i][j] = INT_MAX;
            A[i][j] = __NAN__;
        }
    }
}

/**
 * Reads the data for the benchmark.
 *
 * The data file is expected to be made of A, followed by b, x, and then 3
 * consecutively coarser versions of A.
 *
 * All pointers must point to allocated memory when calling the function.
 */
static void readData(const char *file)
{
    int i;
    int ixc, iyc, izc, ixf, iyf, izf;
    int nxc, nyc, nzc, nxf, nyf;

    FILE *fd = fopen(file, "r");

    if (fd == NULL) {
        fprintf(stderr, "Cannot find data file %s\n", file);
        abort();
    }

    readMatrix(fd, nPerLvl[0], nPerLvl[0], nnzPerLvl[0], nnzPerRow[0],
        colInds[0], A[0], A_diag[0]);
    if (fread(b, sizeof(*b), nPerLvl[0], fd) < nPerLvl[0]) goto error;
    if (fread(x, sizeof(*x), nPerLvl[0], fd) < nPerLvl[0]) goto error;

    for (i = 1; i < LEVELS; ++i) {
        readMatrix(fd, nPerLvl[i], nPerLvl[i], nnzPerLvl[i], nnzPerRow[i],
            colInds[i], A[i], A_diag[i]);

        nxf = nyf = blockSzPerLvl[i - 1];
        nxc = nyc = nzc = blockSzPerLvl[i];

        for (izc = 0; izc < nzc; ++izc) {
            izf = 2 * izc;
            for (iyc = 0; iyc < nyc; ++iyc) {
                iyf = 2 * iyc;
                for (ixc = 0; ixc < nxc; ++ixc) {
                    int currentCoarseRow, currentFineRow;
                    ixf = 2 * ixc;
                    currentCoarseRow = izc * nxc * nyc + iyc * nxc + ixc;
                    currentFineRow = izf * nxf * nyf + iyf * nxf + ixf;
                    f2c[i][currentCoarseRow] = currentFineRow;
                }
            }
        }
    }

    fclose(fd);

    return;

error:
    fprintf(stderr, "%s: invalid file format\n", file);
    abort();
}

void initialize_once() {
    int i;

    nnzPerRow = malloc(sizeof(*nnzPerRow) * LEVELS);
    colInds = malloc(sizeof(*colInds) * LEVELS);
    A = malloc(sizeof(*A) * LEVELS);
    A_diag = malloc(sizeof(*A_diag) * LEVELS);

    z = malloc(sizeof(*z) * LEVELS);
    r = malloc(sizeof(*r) * LEVELS);
    Axf = malloc(sizeof(*Axf) * LEVELS);
    f2c = malloc(sizeof(*f2c) * LEVELS);

    for (i = 0; i < LEVELS; ++i) {
        nnzPerRow[i] = malloc(sizeof(**nnzPerRow) * MAXN);
        colInds[i] = malloc(sizeof(***colInds) * MAXN * MNZPR);
        A[i] = malloc(sizeof(***A) * MAXN * MNZPR);
        A_diag[i] = malloc(sizeof(**A_diag) * MAXN);

        z[i] = malloc(sizeof(**z) * MAXN);
        r[i] = malloc(sizeof(**r) * MAXN);
        Axf[i] = malloc(sizeof(**Axf) * MAXN);
        f2c[i] = malloc(sizeof(**f2c) * MAXN);
    }

    b = malloc(sizeof(*b) * MAXN);
    x = malloc(sizeof(*x) * MAXN);
    x_check = malloc(sizeof(*x_check) * MAXN);
    p = malloc(sizeof(*p) * MAXN);
    Ap = malloc(sizeof(*Ap) * MAXN);

    readData(DATAFILE);

    memcpy(x_check, x, sizeof(*x) * MAXN);

    nb_samples = nPerLvl[0];
}

void initialize() {
    int i, j;

    normr = __NAN__;
    normr0 = __NAN__;

    memcpy(x, x_check, sizeof(*x) * MAXN);
    for (i = 0; i < LEVELS; ++i) {
        for (j = 0; j < MAXN; ++j) {
            r[i][j] = __NAN__;
            Axf[i][j] = __NAN__;
            z[i][j] = __NAN__;
        }
    }

    for (i = 0; i < MAXN; ++i) {
        p[i] = Ap[i] = __NAN__;
    }
}

// ----- Utility functions called from the kernel -----

#pragma rstream inline
void CopyVector(const double * src, double *dst, const int n) {
    int i;
    for (i = 0; i < MAXN; ++i) {
        if (i < n) {
            dst[i] = src[i];
        }
    }
}

#pragma rstream inline
void ComputeWAXPBY(const double alpha, double *x, const double beta, double *y,
    double *w, const int n)
{
    int i;
    if (alpha == 1.0) {
        for (i = 0; i < MAXN; i++) {
            if (i < n) {
                w[i] = x[i] + beta * y[i];
            }
        }
    } else if (beta == 1.0) {
        for (i = 0; i < MAXN; i++) {
            if (i < n) {
                w[i] = alpha * x[i] + y[i];
            }
        }
    } else {
        for (i = 0; i < MAXN; i++) {
            if (i < n) {
                w[i] = alpha * x[i] + beta * y[i];
            }
        }
    }
}

#pragma rstream inline
void ComputeDotProduct(const int n, double *x, double *y, double *result)
{
    int i;
    double local_result = 0.0;
    /*
    if (y == x) {
        for (i = 0; i < n; i++) {
            local_result += x[i] * x[i];
        }
    } else {
    */
        for (i = 0; i < MAXN; i++) {
            if (i < n) {
                local_result += x[i] * y[i];
            }
        }
    //}

    *result = local_result;
}

#pragma rstream map image_of:spmvBB small_fp:x
void spmvBB_img(int *colInds, double *A, double *x, double *y) {
    int k;
    *y = *A * *colInds;
    for (k = 0; k < MAXN; ++k) {
        *y += x[k];
    }
}

void spmvBB(int *colInds, double *A, double *x, double *y)
{
    *y += *A * x[*colInds];
}

#pragma rstream inline
void ComputeSPMV(int nnzPerRow[MAXN], int (*colInds)[MNZPR],
    double (*A)[MNZPR], double *x, double *y, int n)
{
    int i, j;

    for (i = 0; i < MAXN; ++i) {
        if (i < n) {
            y[i] = 0.0;
            for (j = 0; j < MNZPR; ++j) {
                if (j < nnzPerRow[i]) {
                    spmvBB(&colInds[i][j], &A[i][j], x, &y[i]);
                }
            }
        }
    }
}

//#pragma rstream map image_of:symgsBB small_fp:x
void symgsBB_img(int *colInds, double *A, double *x, double *sum)
{
    int k;
    *sum -= *A + *colInds;
    for (k = 0; k < MAXN; ++k) {
        *sum -= x[k];
    }
}

void symgsBB(int *colInds, double *A, double *x, double *sum) {
    *sum -= *A * x[*colInds];
}

#pragma rstream inline
void ComputeSYMGS(int *nnzPerRow, int (*colInds)[MNZPR], double (*A)[MNZPR],
    double *A_diag, double *x, double *r, int n)
{
    int i, j;

    for (i = 0; i < MAXN; ++i) {
        if (i < n) {
            double sum;
            sum = r[i];
            for (j = 0; j < MNZPR; ++j) {
                if (j < nnzPerRow[i]) {
                    symgsBB(&colInds[i][j], &A[i][j], x, &sum);
                }
            }
            sum += x[i] * A_diag[i];
            x[i] = sum / A_diag[i];
        }
    }

    // back sweep
    for (i = MAXN; i >= 0; --i) {
        if (i < n) {
            double sum = r[i];
            for (j = 0; j < MNZPR; ++j) {
                if (j < nnzPerRow[i]) {
                    symgsBB(&colInds[i][j], &A[i][j], x, &sum);
                }
            }
            sum += x[i] * A_diag[i];
            x[i] = sum / A_diag[i];
        }
    }
}

//#pragma rstream map
void ComputeSYMGS_mapped(int *nnzPerRow, int (*colInds)[MNZPR], double (*A)[MNZPR],
    double *A_diag, double *x, double *r, int n)
{
    ComputeSYMGS(nnzPerRow, colInds, A, A_diag, x, r, n);
}

#pragma rstream map image_of:restrictionBB small_fp=r_lvl,r_lvl1,Axf
void restrictionBB_img(int i, double *Axf, double *r_lvl, double *r_lvl1,
    int *f2c)
{
    int j;

    for (j = 0; j < MAXN; ++j) {
        r_lvl1[i] = r_lvl[j] + Axf[j] + f2c[i];
    }
}

void restrictionBB(int i, double *Axf, double *r_lvl, double *r_lvl1,
    int *f2c)
{
    r_lvl1[i] = r_lvl[f2c[i]] - Axf[f2c[i]];

}

#pragma rstream inline
void ComputeRestriction(int lvl, double *Axf, double * restrict *r, int n,
    int *f2c)
{
    int i;

    for (i = 0; i < MAXN; ++i) {
        if (i < n) {
            restrictionBB(i, Axf, r[lvl], r[lvl + 1], f2c);
        }
    }
}

#pragma rstream map image_of:prolongationBB small_fp=z_lvl,z_lvl1
void prolongationBB_img(int i, double *z_lvl, double *z_lvl1, int *f2c) {
    int j;

    for (j = 0; j < MAXN; ++j) {
        z_lvl[j] += f2c[i] + z_lvl1[i];
    }
}


void prolongationBB(int i, double *z_lvl, double *z_lvl1, int *f2c) {
    z_lvl[f2c[i]] += z_lvl1[i];
}

#pragma rstream inline
void ComputeProlongation(int lvl, double * restrict *z, int n, int *f2c) {
    int i;

    for (i = 0; i < MAXN; ++i) {
        if (i < n) {
            prolongationBB(i, z[lvl], z[lvl + 1], f2c);
        }
    }
}

// ----- preconditioner -----

#pragma rstream inline
void ComputeMG_mapped()
{
    int i, j;

    for (i = 0; i < LEVELS - 1; ++i) {
        for (j = 0; j < MAXN; ++j) {
            if (j < nPerLvl[i]) {
                z[i][j] = 0;
            }
        }

        for (j = 0; j < PRESMOOTHER; ++j) {
            ComputeSYMGS_mapped(nnzPerRow[i], colInds[i], A[i], A_diag[i],
                z[i], r[i], nPerLvl[i]);
        }

        mapped_step2(i);
    }

    for (j = 0; j < MAXN; ++j) {
        if (j < nPerLvl[LEVELS - 1]) {
            z[LEVELS - 1][j] = 0;
        }
    }

    ComputeSYMGS_mapped(nnzPerRow[LEVELS - 1], colInds[LEVELS - 1], A[LEVELS - 1],
        A_diag[LEVELS - 1], z[LEVELS - 1], r[LEVELS - 1], nPerLvl[LEVELS - 1]);

    for (i = LEVELS - 2; i >= 0; --i) {
        mapped_step3(i);

        for (j = 0; j < POSTSMOOTHER; ++j) {
            ComputeSYMGS_mapped(nnzPerRow[i], colInds[i], A[i], A_diag[i],
                z[i], r[i], nPerLvl[i]);
        }
    }
}


#pragma rstream inline
void ComputeMG()
{
    int i, j;

    for (i = 0; i < LEVELS - 1; ++i) {
        for (j = 0; j < MAXN; ++j) {
            if (j < nPerLvl[i]) {
                z[i][j] = 0;
            }
        }

        for (j = 0; j < PRESMOOTHER; ++j) {
            ComputeSYMGS(nnzPerRow[i], colInds[i], A[i], A_diag[i],
                z[i], r[i], nPerLvl[i]);
        }

        step2(i);
    }

    for (j = 0; j < MAXN; ++j) {
        if (j < nPerLvl[LEVELS - 1]) {
            z[LEVELS - 1][j] = 0;
        }
    }

    ComputeSYMGS(nnzPerRow[LEVELS - 1], colInds[LEVELS - 1], A[LEVELS - 1],
        A_diag[LEVELS - 1], z[LEVELS - 1], r[LEVELS - 1], nPerLvl[LEVELS - 1]);

    for (i = LEVELS - 2; i >= 0; --i) {
        step3(i);

        for (j = 0; j < POSTSMOOTHER; ++j) {
            ComputeSYMGS(nnzPerRow[i], colInds[i], A[i], A_diag[i],
                z[i], r[i], nPerLvl[i]);
        }
    }
}


// ----- main kernel -----

void hpcg_mapped(double *x, double *normr, double *normr0) {
    int k;
    double rtz, oldrtz, alpha, beta, pAp;

    rtz = 0.0;
    oldrtz = 0.0;
    alpha = 0.0;
    beta = 0.0;
    pAp = 0.0;

    *normr = 0.0;

    // p is of length ncols, copy x to p for sparse MV operation
    mapped_step1(x, normr);
    *normr = sqrt(*normr);

    // Record initial residual for convergence testing
    *normr0 = *normr;

    // Start iterations
    for (k = 1; k <= MAX_ITER /*&& *normr / *normr0 > TOLERANCE*/; k++) {

        //if (doPreconditioning)
            ComputeMG_mapped(); // Apply preconditioner
        //else
            //ComputeWAXPBY(1.0, r[0], 0.0, r[0], z, nPerLvl[0]); // copy r to z (no preconditioning)

        if (k == 1) {
            CopyVector(z[0], p, nPerLvl[0]); // Copy Mr to p
            ComputeDotProduct(nPerLvl[0], r[0], z[0], &rtz); // rtz = r'*z
        } else {
            oldrtz = rtz;
            mapped_step4(&beta, &rtz, oldrtz);
        }

        mapped_step5(&alpha, rtz, &pAp, x, normr);
        *normr = sqrt(*normr);
    }
}

void hpcg(double *x, double *normr, double *normr0) {
    int k;
    double rtz, oldrtz, alpha, beta, pAp;

    rtz = 0.0;
    oldrtz = 0.0;
    alpha = 0.0;
    beta = 0.0;
    pAp = 0.0;

    *normr = 0.0;

    // p is of length ncols, copy x to p for sparse MV operation
    step1(x, normr);
    *normr = sqrt(*normr);

    // Record initial residual for convergence testing
    *normr0 = *normr;

    // Start iterations
    for (k = 1; k <= MAX_ITER /*&& *normr / *normr0 > TOLERANCE*/; k++) {

        //if (doPreconditioning)
            ComputeMG(); // Apply preconditioner
        //else
            //ComputeWAXPBY(1.0, r[0], 0.0, r[0], z, nPerLvl[0]); // copy r to z (no preconditioning)

        if (k == 1) {
            CopyVector(z[0], p, nPerLvl[0]); // Copy Mr to p
            ComputeDotProduct(nPerLvl[0], r[0], z[0], &rtz); // rtz = r'*z
        } else {
            oldrtz = rtz;
            step4(&beta, &rtz, oldrtz);
        }

        step5(&alpha, rtz, &pAp, x, normr);
        *normr = sqrt(*normr);
    }
}


#pragma rstream map
void mapped_step1(double *x, double *normr) {
    step1(x, normr);
}

//
// some pragmas are commented out because they do not lead to performance
// improvements with Intel OCR
//
//#pragma rstream map
void mapped_step2(int i) {
    step2(i);
}

//#pragma rstream map
void mapped_step3(int i) {
    step3(i);
}

//#pragma rstream map
void mapped_step4(double *beta, double *rtz, double oldrtz) {
    step4(beta, rtz, oldrtz);
}

#pragma rstream map
void mapped_step5(double *alpha, double rtz, double *pAp, double *x,
    double *normr)
{
    step5(alpha, rtz, pAp, x, normr);
}

#pragma rstream inline
void step1(double *x, double *normr) {
    CopyVector(x, p, nPerLvl[0]);
    ComputeSPMV(nnzPerRow[0], colInds[0], A[0], p, Ap, nPerLvl[0]); // Ap = A*p
    ComputeWAXPBY(1.0, b, -1.0, Ap, r[0], nPerLvl[0]); // r = b - Ax (x stored in p)
    ComputeDotProduct(nPerLvl[0], r[0], r[0], normr);
}

#pragma rstream inline
void step2(int i) {
    ComputeSPMV(nnzPerRow[i], colInds[i], A[i], z[i], Axf[i + 1], nPerLvl[i]);
    ComputeRestriction(i, Axf[i + 1], r, nPerLvl[i + 1], f2c[i + 1]);
}

#pragma rstream inline
void step3(int i) {
    ComputeProlongation(i, z, nPerLvl[i + 1], f2c[i + 1]);
}

#pragma rstream inline
void step4(double *beta, double *rtz, double oldrtz) {
    ComputeDotProduct(nPerLvl[0], r[0], z[0], rtz); // rtz = r'*z
    *beta = *rtz / oldrtz;
    ComputeWAXPBY(1.0, z[0], *beta, p, p, nPerLvl[0]); // p = beta*p + z
}

#pragma rstream inline
void step5(double *alpha, double rtz, double *pAp, double *x,
    double *normr)
{
    ComputeSPMV(nnzPerRow[0], colInds[0], A[0], p, Ap, nPerLvl[0]); // Ap = A*p
    ComputeDotProduct(nPerLvl[0], p, Ap, pAp); // alpha = p'*Ap
    *alpha = rtz / *pAp;
    ComputeWAXPBY(1.0, x, *alpha, p, x, nPerLvl[0]); // x = x + alpha*p
    ComputeWAXPBY(1.0, r[0], -*alpha, Ap, r[0], nPerLvl[0]); // r = r - alpha*Ap
    ComputeDotProduct(nPerLvl[0], r[0], r[0], normr);
}

void kernel()
{
    hpcg_mapped(x, &normr, &normr0);
}

void show() {
}

int check() {
    int i, j;

    hpcg(x_check, &normr_check, &normr0_check);

    printf("Computed residual: %f (%f / %f)\n", normr / normr0, normr, normr0);
    printf("Expected residual: %f\n", normr_check / normr0_check);
    if (!fequal(normr0, normr0_check)) return 1;
    if (!fequal(normr, normr_check)) return 1;
    for (j = 0; j < nPerLvl[0]; ++j) {
        if (!fequal(x[j], x_check[j])) {
            printf("Different x[%d]: %f vs. %f\n", j, x[j], x_check[j]);
            return 1;
        }
    }

    return 0;
}

double flops_per_trial() {
  return 0;
}

int nb_samples;
char const * function_name = "hpcg";
