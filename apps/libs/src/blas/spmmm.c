/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

// Sparse matrix-matrix multiply -- C = A * B -- Multiply two sparse matrices, yielding a sparse matrix result.
// Storage format for the input and output matrices is ELL:  See http://www.exmatex.org/cosp2.html.

// TODO:  FIXME:
//      * Introduce subblock posturing.  (This is a proposed OCR feature that, I assert, makes it possible for an application and OCR to be more cooperative in the ideal positioning of datablocks/subblocks.)
//      * Need to deal with non-canonical inputs.  For now, the code terminates with an error message.

//#define SANITYCHECKING
//#define SERIALIZEFORDEBUGGING   // FIXME:  When disabled, we presently get the error: "Warning! DataBlocks of type 'Regular' cannot be acquired by more than 64 EDTs"
//#define TILEDEBUGGING

#include "blas.h"
#include "ocr.h"
#include "ocr-types.h"
#include "ocr-posture.h"
#include "extensions/ocr-legacy.h"
#include <stdio.h>

extern ocrGuid_t   legacyContext;   // This value MUST be initialized in main.

//                 A                                 B                              C
//  +-----------------------------+      +-----------------------+      +-----------------------+
//  |                             |      |                       |      |                       |
//  |                             |      |                       |      |                       |
//  |                             |      |                       |      |                       |
//  |                             |      |                       |      |                       |
//  |                             |      |                       |      |                       |
//  |                             |      |                       |      |                       |
//  |                             |      |                       |      |                       |
//  |                             |      |                       |      |                       |
//  |                             |      |                       |      |                       |
//  |                             |      |                       |      |                       |
//  |                             |      |                       |      |                       |
//  +-----------------------------+      |                       |      +-----------------------+
//                                       |                       |
//                                       |                       |
//                                       |                       |
//                                       |                       |
//                                       |                       |
//                                       +-----------------------+
//
//
// Split workloads into tiles of A and B, implying the need to also split C into tiles with the need to collate partial result tiles of C into its final structure:
//
//                 A                                     B                                 C0                                C1                                 C(N-1)
//  +-----------------------------+          +-----------------------+         +-----------------------+         +-----------------------+            +-----------------------+
//  |    |    |    |    |    |    |          |     |     |     |     |         |     |     |     |     |         |     |     |     |     |            |     |     |     |     |
//  |    |    |    |    |    |    |          |     |     |     |     |         |     |     |     |     |         |     |     |     |     |            |     |     |     |     |
//  |-----------------------------|          |-----|-----|-----|-----|         |-----|-----|-----|-----|         |-----|-----|-----|-----|            |-----|-----|-----|-----|
//  |    |    |    |    |    |    |          |     |     |     |     |         |     |     |     |     |         |     |     |     |     |            |     |     |     |     |
//  |    |    |    |    |    |    |          |     |     |     |     |         |     |     |     |     |         |     |     |     |     |            |     |     |     |     |
//  |-----------------------------|    *     |-----|-----|-----|-----|    =>   |-----|-----|-----|-----|    +    |-----|-----|-----|-----|  ...  +    |-----|-----|-----|-----|
//  |    |    |    |    |    |    |          |     |     |     |     |         |     |     |     |     |         |     |     |     |     |            |     |     |     |     |
//  |    |    |    |    |    |    |          |     |     |     |     |         |     |     |     |     |         |     |     |     |     |            |     |     |     |     |
//  |-----------------------------|          |-----|-----|-----|-----|         |-----|-----|-----|-----|         |-----|-----|-----|-----|            |-----|-----|-----|-----|
//  |    |    |    |    |    |    |          |     |     |     |     |         |     |     |     |     |         |     |     |     |     |            |     |     |     |     |
//  |    |    |    |    |    |    |          |     |     |     |     |         |     |     |     |     |         |     |     |     |     |            |     |     |     |     |
//  +-----------------------------+          |-----|-----|-----|-----|         +-----------------------+         +-----------------------+            +-----------------------+
//                                           |     |     |     |     |
//                                           |     |     |     |     |
//                                           |-----|-----|-----|-----|
//                                           |     |     |     |     |
//                                           |     |     |     |     |
//                                           +-----------------------+
//
// Step 1:  Decide how many tiles to divide the matrices.  The number of tiles across each row and down each column will be a power of two.
// Step 2:  Assay the matrices to figure how much storage is needed by each individual tile of the two inputs.
// Step 3:  Allocate input tile space and populate it.  We need CRS of A and CCS of B.  In order to estimate size needed for AxB tiles, we also need CCS of A and CRS of B (but only
//          col/row descrs, not col/rowIdx and not val.
// Step 4:  For each AxB tile pairing, figure the size of C needed, allocate, then perform tile matrix multiply.  Collate/accumulate results.
// Step 5:  Wrapup.  Destroy temporary datablocks.
//

// CRS data typedefs:
typedef u32 rowNum_t;          // Ordinal of a row that has one or more non-zero columns in the tile.  Max matrix height is 2^32-1.  TODO (maybe):  When A, B, and C are tiled, this could squeeze into u16.
typedef u16 numNzColsInRow_t;  // Number of non-zero columns the row has in the tile.  The number of non-zero elements per row of a CRS >>>tile<<< of A, B, or C, will NOT exceed 65534, i.e. 2^16-2.
typedef u16 colIdxIdx_t;       // Index over colIdx array, used for assaying and populating CCS-format tile from CRS-format tile.  Range is limited like numNzColsInRow_t.
typedef u32 colIdx_t;          // The number of columns in the maximum-supported matrix is 2^32-1.  TODO (maybe):  When A, B, and C are tiled, this could squeeze into u16, to save space and fit in L1 more often.

// CCS data typedefs:
typedef u32 colNum_t;          // Ordinal of a column that has one or more non-zero rows in the tile.  Max matrix height is 2^32-1.  TODO (maybe):  When A, B, and C are tiled, this could squeeze into u16.
typedef u16 numNzRowsInCol_t;  // Number of non-zero rows the column has in the tile.  The number of non-zero elements per column of a CCS >>>tile<<< of A, B, or C, will NOT exceed 65534, i.e. 2^16-2.
typedef u32 rowIdx_t;          // The number of rows in the maximum-supported matrix is 2^32-1.  TODO (maybe):  When A, B, and C are tiled, this could squeeze into u16, to save space and fit in L1 more often.

typedef struct {
    u32 numRows;               // Count of how many rows in this tile have one or more non-zero element.
    u32 numNzElements;         // Total number of nonzero elements in the tile.
    u32 currRow;               // Facility to keep track of whether or not this row of the tile has already contributed any non-zero columns to the tile.
} CRSTileAssay_t;

typedef struct {
    u32 numCols;               // Count of how many columns in this tile have one or more non-zero element.
    u32 numNzElements;         // Total number of nonzero elements in the tile.
} CCSTileAssay_t;

typedef struct {
#ifdef TILEDEBUGGING
    void *             CCSptr;   // Pointer to the datablock for a tile in CCS format.  FIXME: DO NOT dereference through this pointer in productiion version.  Use it only for debugging, in development.
    void *             CRSptr;   // Pointer to the datablock for a tile in CRS format.  FIXME: DO NOT dereference through this pointer in productiion version.  Use it only for debugging, in development.
#endif
    ocrGuid_t          tileCRS;  // Guid of the datablock for a tile in CRS format.
    union {
        CRSTileAssay_t assayCRS; // Assay of tile in CRS format.  (Needed only until the tile's datablock is actually created.)
        ocrGuid_t      tileCCS;  // Guid of the datablock for a tile in CCS format.
    };
} InputTileDescr_t;

typedef struct {
    u64    longestRowOfC;              // Largest number of nonzero elements seen.
    u64    cTotalNumNzElements;        // Total number of nozero elements in matrix C.  This INCLUDES the number of excess nonzero elements that might have to be dropped to fit in a row of the ELL structure.
    u64    resultCode;                 // Error code, indicative of a row having more non-zero elements than can fit in the ELL structure provided by the caller.
} statisticsForStripOfC_t;

typedef struct {
    SpmmmIdx_t    m;                   // Number of rows in matrix A and in matrix C.
    SpmmmIdx_t    n;                   // Number of columns in matrix A, and number of rows in matrix B.
    SpmmmIdx_t    k;                   // Number of columns in matrix B and in matrix C.
    BLAS_MATH_t * aVal;                // Address of 2D array holding non-zero-valued elements of matrix A.
    BLAS_MATH_t * bVal;                // Address of 2D array holding non-zero-valued elements of matrix B.
    BLAS_MATH_t * cVal;                // Address of 2D array to receive non-zero-valued elements of matrix C in ELL format.
    SpmmmIdx_t  * aColIdx;             // Address of 2D array holding column indices of matrix A.
    SpmmmIdx_t  * bColIdx;             // Address of 2D array holding column indices of matrix B.
    SpmmmIdx_t  * cColIdx;             // Address of 2D array to receive column indices of matrix C.
    SpmmmIdx_t  * aNumNzColsInRow;     // Address of 1D array holding the count of column indices and values for each row of A.
    SpmmmIdx_t  * bNumNzColsInRow;     // Address of 1D array holding the count of column indices and values for each row of B.
    SpmmmIdx_t  * cNumNzColsInRow;     // Address of 1D array to receive count of column indices and values for each row of C.
    SpmmmIdx_t    lda;                 // Leading dimension of aVal and aIdx arrays, aka the maximum number of non-zero-valued elements any row of A can accomodate.
    SpmmmIdx_t    ldb;                 // Leading dimension of bVal and bIdx arrays, aka the maximum number of non-zero-valued elements any row of B can accomodate.
    SpmmmIdx_t    ldc;                 // Leading dimension of cVal and cColIdx arrays, aka the maximum number of non-zero-valued elements any row of C can accomodate.
    double        epsilon;             // When a result is between +/- epsilon, it is treated as zero, i.e. eliminated from the output.
    s64           aTotalNumNzElements; // Number of nonzeroes in matrix A.  If unknown, provide intelligent guess, or -1 if totally random.
    s64           bTotalNumNzElements; // Number of nonzeroes in matrix B.  If unknown, provide intelligent guess, or -1 if totally random.
    s64         * cTotalNumNzElements; // Output: Number of nonzeroes in matrix C.  If C is used as a source (A of B) for a subsequent ?spmmm call, providing this value will improve performance.
    SpmmmIdx_t  * cMaxNumNzColsInRow;  // Address at which to return the number of non-zero elements in the most populous row of matrix C.
    u64         * resultCode;          // Returns zero if totally successful, else cast resultCode to SPMMM_RETURN_CODE_t to decipher problem.
} NAME(spmmmThunk_params_t);

static ocrGuid_t NAME(spmmmThunk_task) (  // The function-call API kicks off the OCR topology through this "thunking" EDT.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    SpmmmIdx_t     matrixHeight;              // Height of full matrix.
    SpmmmIdx_t     matrixWidth;               // Width of full matrix.
    SpmmmIdx_t     numTilesAcross;            // Number of tiles across matrix.
    SpmmmIdx_t     numTilesDown;              // Number of tiles down matrix.
    SpmmmIdx_t   * numNzColsInRow;            // Address of 1D array of non-zero element counts for each row of matrix.
    SpmmmIdx_t   * colIdx;                    // Address of 2D array containing non-zero column indices.
    BLAS_MATH_t  * val;                       // Address of 2D array containing non-zero elements.  (Not used in assaying, but used when tiles are subsequently populated.)
    SpmmmIdx_t     ld;                        // Leading dimension of the aColIdx array.
    SpmmmIdx_t     firstRowNum;               // Index of topmost row for this full-width strip.  (Recall that all strips are maximally interlaced.)
    SpmmmIdx_t     numStripsToAssay;          // Do assays of this many strips.
    SpmmmIdx_t     firstCol;                  // Only place this is used is to convey leftmost column index for tile being converted to CCS format.
    CCSTileAssay_t assayCCS;                  // Only used to communicate between assayCCS_edt and populateCCS_edt.  Undefined elsewhere
    bool           isA;                       // True for factoring A, false for factoring B.
    ocrGuid_t      factorInput_template;      // Template to create EDT to split workloads of multiple strips to two children, or to factor a single strip to CRS and CCS tile format.
    ocrGuid_t      populateCRSTiles_template; // Template to create EDT to populate CRS tiles across full-width strip.
    ocrGuid_t      assayCCSTile_template;     // Template to create EDT to assay the storage requirements of a CCS tile.
    ocrGuid_t      populateCCSTile_template;  // Template to create EDT to populate CCS tile.
} NAME(spmmm_factorInput_params_t);

typedef struct {
    ocrEdtDep_t   tileCatalog;                // Datablock for collecting tile assay information, and then for recording GUID of datablocks containing tiles.
    ocrEdtDep_t   serializationTrigger;       // Optional trigger to serialize this EDT with respect to others, for easier reading of debugging dumps.  Fullfill with NULL_GUID for production runs.
} NAME(spmmm_factorInput_deps_t);

static ocrGuid_t NAME(spmmm_factorInput_task) (  // One EDT created for factoring A, another for B.  Each creates two child EDTs to distribute full-width strips, until leafs each tile a single strip.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    ocrEdtDep_t   tileCatalog;                // Datablock for collecting tile assay information, and then for recording GUID of datablocks containing tiles.
    ocrEdtDep_t   tile[1];                    // Catalog of datablocks for a full-width strip.  MUST BE LAST THING IN STRUCT.  Struct length is actually variable.
} NAME(spmmm_populateCRSTiles_deps_t);

static ocrGuid_t NAME(spmmm_populateCRSTiles_task) ( //  Populate a strip of CRS tiles.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    ocrEdtDep_t   tileCatalog;                // Datablock for collecting tile assay information, and then for recording GUID of datablocks containing tiles.
    ocrEdtDep_t   tileCRS;                    // Datablock for the tile, in CRS format.
    ocrEdtDep_t   colIdxIdx;                  // Datablock for scratch space to keep track of progress across each row of tileCRS as we look for elements to copy to tileCCS.
} NAME(spmmm_assayCCSTile_deps_t);

static ocrGuid_t NAME(spmmm_assayCCSTile_task) ( //  Assay the storage requirements for a tile in CCS format, given the tile in CRS format.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    ocrEdtDep_t   tileCatalog;                // Datablock for collecting tile assay information, and then for recording GUID of datablocks containing tiles.
    ocrEdtDep_t   tileCRS;                    // Datablock for the tile, in CRS format.
    ocrEdtDep_t   colIdxIdx;                  // Datablock for scratch space to keep track of progress across each row of tileCRS as we look for elements to copy to tileCCS.
    ocrEdtDep_t   tileCCS;                    // Storage for tile in CCS format.
} NAME(spmmm_populateCCSTile_deps_t);

static ocrGuid_t NAME(spmmm_populateCCSTile_task) ( //  Populate a CCS tiles.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    SpmmmIdx_t    m;                          // Number of rows in matrix C (and in matrix A).
    SpmmmIdx_t  * cNumNzColsInRow;            // Address of 1D array to receive count of column indices and values for each row of C.
    SpmmmIdx_t  * cColIdx;                    // Address of 2D array to receive column indices of matrix C.
    BLAS_MATH_t * cVal;                       // Address of 2D array to receive non-zero-valued elements of matrix C in ELL format.
    SpmmmIdx_t    ldc;                        // Leading dimension of cVal and cColIdx arrays, aka the maximum number of non-zero-valued elements any row of C can accomodate.
    SpmmmIdx_t    aNumTilesAcross;            // Number of tiles across full-width strip of A.
    SpmmmIdx_t    aNumTilesDown;              // Number of tiles down full-width strip of A.
    SpmmmIdx_t    bNumTilesAcross;            // Number of tiles across full-width strip of B.
    SpmmmIdx_t    bNumTilesDown;              // Number of tiles down full-width strip of B.
    SpmmmIdx_t    aTileRowNum;                // Index of [first] strip to drive matrix multiply for.
    SpmmmIdx_t    numStripsToDoMatMulFor;     // Number of strips to drive matrix multiply for.
    double        epsilon;                    // When a result is between +/- epsilon, it is treated as zero, i.e. eliminated from the output.
    ocrGuid_t     doFullMatMul_template;      // Template to create EDT to drive matrix multipy for one or more strips.
    ocrGuid_t     doStripOfC_template;
    ocrGuid_t     doMosaicOfC_template;
    ocrGuid_t     doTileOfC_template;
    ocrGuid_t     doTileOfAxB_1_template;
    ocrGuid_t     doTileOfAxB_2_template;
    ocrGuid_t     collateMosaic_template;
    ocrGuid_t     collateMosaic_prelude_template;
    ocrGuid_t     collateIntoOutput_template;
    ocrGuid_t     collateStripsOfC_template;
    s64         * cTotalNumNzElements; // Output: Number of nonzeroes in matrix C.  If C is used as a source (A of B) for a subsequent ?spmmm call, providing this value will improve performance.
    SpmmmIdx_t  * cMaxNumNzColsInRow;  // Address at which to return the number of non-zero elements in the most populous row of matrix C.
    u64         * resultCode;          // Returns zero if totally successful, else cast resultCode to SPMMM_RETURN_CODE_t to decipher problem.

} NAME(spmmm_doFullMatMul_params_t);

typedef struct {
    ocrEdtDep_t tileCatalogOfA;
    ocrEdtDep_t tileCatalogOfB;
    ocrEdtDep_t serializationTrigger1;
    ocrEdtDep_t serializationTrigger2;
} NAME(spmmm_doFullMatMul_deps_t);

static ocrGuid_t NAME(spmmm_doFullMatMul_task) (   // Drive the full matrix multiply, once the CRS tiles of A and CCS tiles of B are in hand, and the CCS assays of A and CRS assays of B.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    SpmmmIdx_t    m;                          // Number of rows in matrix C (and in matrix A).
    SpmmmIdx_t  * cNumNzColsInRow;            // Address of 1D array to receive count of column indices and values for each row of C.
    SpmmmIdx_t  * cColIdx;                    // Address of 2D array to receive column indices of matrix C.
    BLAS_MATH_t * cVal;                       // Address of 2D array to receive non-zero-valued elements of matrix C in ELL format.
    SpmmmIdx_t    ldc;                        // Leading dimension of cVal and cColIdx arrays, aka the maximum number of non-zero-valued elements any row of C can accomodate.
    SpmmmIdx_t    aNumTilesAcross;            // Number of tiles across full-width strip of A.
    SpmmmIdx_t    aNumTilesDown;              // Number of tiles down full-width strip of A.
    SpmmmIdx_t    bNumTilesAcross;            // Number of tiles across full-width strip of B.
    SpmmmIdx_t    bNumTilesDown;              // Number of tiles down full-width strip of B.
    SpmmmIdx_t    firstRowOfStripOfC;         // First row of this strip of C.  Since input matrices are partitioned in fully-interlaced fashion, this is aka the strip number.
    SpmmmIdx_t    firstColOfC;                // First column of this tile or mosaic of C.
    SpmmmIdx_t    numTilesOfMosaic;           // Number of horizontally collocated tiles of C to process as a single mosaic.
    SpmmmIdx_t    firstColOfTileOfAAndRowOfTileOfB;  // First column of this tile of A, aka first row of this tile of B.
    SpmmmIdx_t    numRowsInTileOfCWithNzElements;    // Number of rows in a partial-result tile of C (i.e. one tile of A * corresponding tile of B) that might be nonzero.
    SpmmmIdx_t    numNzElementsInTileOfC;            // Maximum number of non-zero elements possible in this partial-result tile of C.
    double        epsilon;                    // When a result is between +/- epsilon, it is treated as zero, i.e. eliminated from the output.
    ocrGuid_t     doMosaicOfC_template;       // Template to create EDT to drive the matrix multiply of a several horizontally collocated tiles of C.
    ocrGuid_t     doTileOfC_template;         // Template to create EDT to drive the matrix multiply of a single tile of C (which is an accumulation of a row of tiles of A times corresponding col of tiles of B)
    ocrGuid_t     doTileOfAxB_1_template;     // Template to create EDT that does step1 of a single tile of A times a single tile of B (which allocates space for the resultant C).
    ocrGuid_t     doTileOfAxB_2_template;     // Template to create EDT that does step2 of a single tile of A times a single tile of B (which does the multiply).
    ocrGuid_t     collateMosaic_template;     // Template to create EDT that collates/accumulates two CRS tiles and/or mosaics.
    ocrGuid_t     collateMosaic_prelude_template;  // Template to create EDT that acts as prelude to the above (to create the datablock for the new accumulator CRS mosaic.
    ocrGuid_t     collateIntoOutput_template; // Template to create an EDT that collates two CRS half-width-strip mosaics to the appropriate rows of the final ELL storage.
    ocrGuid_t     collateStripsOfC_template;  // Template to create an EDT that combines STATISTICS of strips of C.
} NAME(spmmm_doMatMulCalcs_params_t);

typedef struct {
    ocrEdtDep_t tileCatalogOfA;
    ocrEdtDep_t tileCatalogOfB;
    ocrEdtDep_t serializationTrigger;
} NAME(spmmm_doStripOfC_deps_t);

static ocrGuid_t NAME(spmmm_doStripOfC_task) (   // Drive the full matrix multiply for a single full-width strip of C.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    ocrEdtDep_t tileCatalogOfA;
    ocrEdtDep_t tileCatalogOfB;
    ocrEdtDep_t serializationTrigger;
} NAME(spmmm_doTileOrMosaicOfC_deps_t);

static ocrGuid_t NAME(spmmm_doMosaicOfC_task) ( // Drive the full matrix multiply for a mosaic of horizontally collocated tiles of C.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);

static ocrGuid_t NAME(spmmm_doTileOfC_task) (   // Drive the full matrix multiply for a single tile of C.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    ocrEdtDep_t tileCRSofA;
    ocrEdtDep_t tileCCSofA;
    ocrEdtDep_t tileCRSofB;
    ocrEdtDep_t tileCCSofB;
} NAME(spmmm_doTileOfAxB_1_deps_t);

static ocrGuid_t NAME(spmmm_doTileOfAxB_1_task) (   // Do step1 (storage allocation of C) for a single tile of A times a corresponding tile of B.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    ocrEdtDep_t tileCRSofA;       // This tile of A
    ocrEdtDep_t tileCCSofB;       // This tile of B
    ocrEdtDep_t tileCRSofAxB;     // Storage for computing this tile of C, i.e. this A times this B.
} NAME(spmmm_doTileOfAxB_2_deps_t);

static ocrGuid_t NAME(spmmm_doTileOfAxB_2_task) (   // Do step2 (actual matrix multiply calculation) for a single tile of A times a corresponding tile of B.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    ocrEdtDep_t mosaicCRSofC_0;
    ocrEdtDep_t mosaicCRSofC_1;
} NAME(spmmm_collateMosaic_prelude_deps_t);

static ocrGuid_t NAME(spmmm_collateMosaic_prelude_task) ( // Preamble for collateMosaic, needed for collation of final tiles of C into mosaic, or combining mosaics into bigger ones. Allocates storage for newAcc.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    ocrEdtDep_t mosaicCRSofC_0;
    ocrEdtDep_t mosaicCRSofC_1;
    ocrEdtDep_t newAcc;           // New Accumulation/collation of the above.
} NAME(spmmm_collateMosaic_deps_t);

static ocrGuid_t NAME(spmmm_collateMosaic_task) (   // Accumulate/collate two CRS tiles and/or mosaics into one.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    ocrEdtDep_t mosaicCRSofC_0;
    ocrEdtDep_t mosaicCRSofC_1;
    ocrEdtDep_t statisticsForStripOfC;
} NAME(spmmm_collateIntoOutput_deps_t);

static ocrGuid_t NAME(spmmm_collateIntoOutput_task) (   // Collate two half-width mosaics of C in CRS format to the appropriate rows of the ELL output construct.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    u64 junk;
} NAME(spmmm_collateStripsOfC_params_t);

typedef struct {
    ocrEdtDep_t stripOfCStats_0;
    ocrEdtDep_t stripOfCStats_1;
} NAME(spmmm_collateStripsOfC_deps_t);

static ocrGuid_t NAME(spmmm_collateStripsOfC_task) (   // Collate statistics about the strips of C, and report them out to caller (via a final summary to wrapup).
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);


typedef struct {
    ocrEdtDep_t tileCatalogOfA;
    ocrEdtDep_t tileCatalogOfB;
    ocrEdtDep_t matrixCStats;
} NAME(spmmm_wrapup_deps_t);

static ocrGuid_t NAME(spmmm_wrapup_task) (   // Return control to caller -- TODO!  FIXME!
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]);



// CRS Tile organization:
// *  Tile meta data -- numRows and numNzElements, offsets to following for arrays (from start of struct)
// *  Tile row descriptors -- 1d arrays of rowNum (the index of a row that contains nonzero elements), and numNzColsInRow (the number of nonzero elements in that row).
// *  Tile colIdx data -- 2d array of column indices for the nonzero elements.
// *  Tile val data -- 2d array of nonzero elements.
typedef struct {
    u32                numRows;                // Number of rows that have non-zero elements, aka length of the rowNum and numNzColsInRow arrays.
    u32                numNzElements;          // Total number of non-zero elements in all rows of the tile, aka length of the colIdx and val arrays.
    u32                offsetToRowNum;         // Distance from start of this struct to rowNum array.
    u32                offsetToNumNzColsInRow; // Distance from start of this struct to numNzColsInRow array.
    u32                offsetToColIdx;         // Distance from start of this struct to colIdx array.
    u32                offsetToVal;            // Distance from start of this struct to val array.
} CRSTileMeta_t;

static rowNum_t * getAddrCRSTileRowNumArray(CRSTileMeta_t * pTileMeta) {
    return (rowNum_t *) (((u64) pTileMeta) + pTileMeta->offsetToRowNum);
}

static numNzColsInRow_t * getAddrCRSTileNumNzColsInRowArray(CRSTileMeta_t * pTileMeta) {
    return (numNzColsInRow_t *) (((u64) pTileMeta) + pTileMeta->offsetToNumNzColsInRow);
}

static colIdx_t * getAddrCRSTileColIdxArray(CRSTileMeta_t * pTileMeta) {
    return (colIdx_t *) (((u64) pTileMeta) + pTileMeta->offsetToColIdx);
}

static BLAS_MATH_t * getAddrCRSTileValArray(CRSTileMeta_t * pTileMeta) {
    return (BLAS_MATH_t *) (((u64) pTileMeta) + pTileMeta->offsetToVal);
}

// CCS Tile organization:   Essentially identical to CRS Tile organization, but the terms "column" and "row" are swapped.
// *  Tile meta data -- numRows and numNzElements, offsets to following for arrays (from start of struct)
// *  Tile col descriptors -- 1d arrays of colNum (the index of a column that contains nonzero elements), and numNzRowsInCol (the number of nonzero elements in that column).
// *  Tile rowIdx data -- 2d array of row indices for the nonzero elements.
// *  Tile val data -- 2d array of nonzero elements.
typedef struct {
    u32                numCols;                // Number of columns that have non-zero elements, aka length of the colNum and numNzRowsInCol arrays.
    u32                numNzElements;          // Total number of non-zero elements in all columns of the tile, aka length of the rowIdx and val arrays.
    u32                offsetToColNum;         // Distance from start of this struct to rowNum array.
    u32                offsetToNumNzRowsInCol; // Distance from start of this struct to numNzRowsInCol array.
    u32                offsetToRowIdx;         // Distance from start of this struct to rowIdx array.
    u32                offsetToVal;            // Distance from start of this struct to val array.
} CCSTileMeta_t;

static colNum_t * getAddrCCSTileColNumArray(CCSTileMeta_t * pTileMeta) {
    return (rowNum_t *) (((u64) pTileMeta) + pTileMeta->offsetToColNum);
}

static numNzRowsInCol_t * getAddrCCSTileNumNzRowsInColArray(CCSTileMeta_t * pTileMeta) {
    return (numNzRowsInCol_t *) (((u64) pTileMeta) + pTileMeta->offsetToNumNzRowsInCol);
}

static rowIdx_t * getAddrCCSTileRowIdxArray(CCSTileMeta_t * pTileMeta) {
    return (rowIdx_t *) (((u64) pTileMeta) + pTileMeta->offsetToRowIdx);
}

static BLAS_MATH_t * getAddrCCSTileValArray(CCSTileMeta_t * pTileMeta) {
    return (BLAS_MATH_t *) (((u64) pTileMeta) + pTileMeta->offsetToVal);
}


// Externally visible function-call variety API for ?spmmm.  Drive an OCR EDT topology from this function, with the sink of the topology responsible to report results back to this function so
// they can be returned to its caller.

void NAME(spmmm) (                      // Function-type API for sparse-matrix-sparse-matrix multiply.
    SpmmmIdx_t    m,                    // Number of rows in matrix A and in matrix C.
    SpmmmIdx_t    n,                    // Number of columns in matrix A, and number of rows in matrix B.
    SpmmmIdx_t    k,                    // Number of columns in matrix B and in matrix C.
    BLAS_MATH_t * aVal,                 // Address of 2D array holding non-zero-valued elements of matrix A.
    BLAS_MATH_t * bVal,                 // Address of 2D array holding non-zero-valued elements of matrix B.
    BLAS_MATH_t * cVal,                 // Address of 2D array to receive non-zero-valued elements of matrix C in ELL format.
    SpmmmIdx_t  * aColIdx,              // Address of 2D array holding column indices of matrix A.
    SpmmmIdx_t  * bColIdx,              // Address of 2D array holding column indices of matrix B.
    SpmmmIdx_t  * cColIdx,              // Address of 2D array to receive column indices of matrix C.
    SpmmmIdx_t  * aNumNzColsInRow,      // Address of 1D array holding the count of column indices and values for each row of A.
    SpmmmIdx_t  * bNumNzColsInRow,      // Address of 1D array holding the count of column indices and values for each row of B.
    SpmmmIdx_t  * cNumNzColsInRow,      // Address of 1D array to receive count of column indices and values for each row of C.
    SpmmmIdx_t    lda,                  // Leading dimension of aVal and aIdx arrays, aka the maximum number of non-zero-valued elements any row of A can accomodate.
    SpmmmIdx_t    ldb,                  // Leading dimension of bVal and bIdx arrays, aka the maximum number of non-zero-valued elements any row of B can accomodate.
    SpmmmIdx_t    ldc,                  // Leading dimension of cVal and cColIdx arrays, aka the maximum number of non-zero-valued elements any row of C can accomodate.
    double        epsilon,              // When a result is between +/- epsilon, it is treated as zero, i.e. eliminated from the output.
    s64           aTotalNumNzElements,  // Number of nonzeroes in matrix A.  If unknown, provide intelligent guess, or -1 if totally random.
    s64           bTotalNumNzElements,  // Number of nonzeroes in matrix B.  If unknown, provide intelligent guess, or -1 if totally random.
    s64         * cTotalNumNzElements,  // Output: Number of nonzeroes in matrix C.  If C is used as a source (A of B) for a subsequent ?spmmm call, providing this value will improve performance.
    SpmmmIdx_t  * cMaxNumNzColsInRow,   // Address at which to return the number of non-zero elements in the most populous row of matrix C.
    u64         * resultCode)           // Returns zero if totally successful, else cast resultCode to SPMMM_RETURN_CODE_t to decipher problem.
{

    printf ("Entered Standard API %s function.\n", STRINGIFY(NAME(spmmm))); fflush(stdout);
    ocrGuid_t   spmmmThunk_template;
    ocrGuid_t   spmmm_doneEvent;
    ocrGuid_t   ctrlDep = NULL_GUID;
    ocrGuid_t   blockProgress_outputGuid;
    void      * blockProgress_result;
    u64         blockProgress_size;


printf ("line = %ld\n", (u64) __LINE__);
    NAME(spmmmThunk_params_t) thunkParams;
    thunkParams.m                   = m;
    thunkParams.n                   = n;
    thunkParams.k                   = k;
    thunkParams.aVal                = aVal;
    thunkParams.bVal                = bVal;
    thunkParams.cVal                = cVal;
    thunkParams.aColIdx             = aColIdx;
    thunkParams.bColIdx             = bColIdx;
    thunkParams.cColIdx             = cColIdx;
    thunkParams.aNumNzColsInRow     = aNumNzColsInRow;
    thunkParams.bNumNzColsInRow     = bNumNzColsInRow;
    thunkParams.cNumNzColsInRow     = cNumNzColsInRow;
    thunkParams.lda                 = lda;
    thunkParams.ldb                 = ldb;
    thunkParams.ldc                 = ldc;
    thunkParams.epsilon             = epsilon;
    thunkParams.aTotalNumNzElements = aTotalNumNzElements;
    thunkParams.bTotalNumNzElements = bTotalNumNzElements;
    thunkParams.cTotalNumNzElements = cTotalNumNzElements;
    thunkParams.cMaxNumNzColsInRow  = cMaxNumNzColsInRow;
    thunkParams.resultCode          = resultCode;

    ocrEdtTemplateCreate(&spmmmThunk_template, NAME(spmmmThunk_task), EDT_PARAM_UNK, EDT_PARAM_UNK);
    ocrLegacySpawnOCR(&spmmm_doneEvent, spmmmThunk_template, sizeof(NAME(spmmmThunk_params_t)) / sizeof(u64), (u64 *) &thunkParams, 1, &ctrlDep, legacyContext);
    ocrLegacyBlockProgress(spmmm_doneEvent, &blockProgress_outputGuid, &blockProgress_result, &blockProgress_size, LEGACY_PROP_WAIT_FOR_CREATE);
    ocrEventDestroy(spmmm_doneEvent);
    ocrEdtTemplateDestroy(spmmmThunk_template);
    printf ("        Standard API %s function exiting.\n", STRINGIFY(NAME(spmmm))); fflush(stdout);
} // ?spmmm


static ocrGuid_t NAME(spmmmThunk_task) (  // Top-level EDT of the spmmmm OCR topology
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{
    printf ("Entered %s.\n", STRINGIFY(NAME(spmmmThunk_task))); fflush(stdout);
    NAME(spmmmThunk_params_t) * myParams = (NAME(spmmmThunk_params_t) *) paramv;
    SpmmmIdx_t    m                   = myParams->m;                   // Number of rows in matrix A and in matrix C.
    SpmmmIdx_t    n                   = myParams->n;                   // Number of columns in matrix A, and number of rows in matrix B.
    SpmmmIdx_t    k                   = myParams->k;                   // Number of columns in matrix B and in matrix C.
    BLAS_MATH_t * aVal                = myParams->aVal;                // Address of 2D array holding non-zero-valued elements of matrix A.
    BLAS_MATH_t * bVal                = myParams->bVal;                // Address of 2D array holding non-zero-valued elements of matrix B.
    BLAS_MATH_t * cVal                = myParams->cVal;                // Address of 2D array to receive non-zero-valued elements of matrix C in ELL format.
    SpmmmIdx_t  * aColIdx             = myParams->aColIdx;             // Address of 2D array holding column indices of matrix A.
    SpmmmIdx_t  * bColIdx             = myParams->bColIdx;             // Address of 2D array holding column indices of matrix B.
    SpmmmIdx_t  * cColIdx             = myParams->cColIdx;             // Address of 2D array to receive column indices of matrix C.
    SpmmmIdx_t  * aNumNzColsInRow     = myParams->aNumNzColsInRow;     // Address of 1D array holding the count of column indices and values for each row of A.
    SpmmmIdx_t  * bNumNzColsInRow     = myParams->bNumNzColsInRow;     // Address of 1D array holding the count of column indices and values for each row of B.
    SpmmmIdx_t  * cNumNzColsInRow     = myParams->cNumNzColsInRow;     // Address of 1D array to receive count of column indices and values for each row of C.
    SpmmmIdx_t    lda                 = myParams->lda;                 // Leading dimension of aVal and aIdx arrays, aka the maximum number of non-zero-valued elements any row of A can accomodate.
    SpmmmIdx_t    ldb                 = myParams->ldb;                 // Leading dimension of bVal and bIdx arrays, aka the maximum number of non-zero-valued elements any row of B can accomodate.
    SpmmmIdx_t    ldc                 = myParams->ldc;                 // Leading dimension of cVal and cColIdx arrays, aka the maximum number of non-zero-valued elements any row of C can accomodate.
    double        epsilon             = myParams->epsilon;             // When a result is between +/- epsilon, it is eliminated from the output, i.e. set to zero.
    s64           aTotalNumNzElements = myParams->aTotalNumNzElements; // Number NZs in matrix A.  If unknown, provide guess, or -1 if totally random.
    s64           bTotalNumNzElements = myParams->bTotalNumNzElements; // Number NZs in matrix B.  If unknown, provide guess, or -1 if totally random.
    s64         * cTotalNumNzElements = myParams->cTotalNumNzElements; // Output: NZs in matrix C.  If C is used as a source (A of B) for a subsequent ?spmmm call, providing this value will improve performance.
    SpmmmIdx_t  * cMaxNumNzColsInRow  = myParams->cMaxNumNzColsInRow;  // Address at which to return the number of non-zero elements in the most populous row of matrix C.
    u64         * resultCode          = myParams->resultCode;          // Returns zero if totally successful, else cast resultCode to SPMMM_RETURN_CODE_t to decipher problem.

// Analyze input matrices A and B, and subjective sparsity information, and come up with an "ideal" number of tiles into which to split the total work.

    if (aTotalNumNzElements < 0) {     // If no objective (i.e. accurate) or even subjective (i.e. guess) about sparsity of A given, then just take a wag at it.
        aTotalNumNzElements =          // Make the (rather rash) assumption that matrix A sparsity is the lesser of...
            MIN (((m * n) >> 12),      // ... .025% of all rows and columns or ...
                 ((m * lda) >> 1));    // ... each row of the matrix having an average number of nonzero columns equal to one half of the maximum possible for the row (and every row).
    }
    if (bTotalNumNzElements < 0) {     // If no objective (i.e. accurate) or even subjective (i.e. guess) about sparsity of B given,  then just take a wag at it.
        bTotalNumNzElements =          // Make the (rather rash) assumption that matrix B sparsity is the lesser of...
            MIN (((n * k) >> 12),      // ... .025% of all rows and columns or ...
                 ((n * ldb) >> 1));    // ... each row of the matrix having an average number of nonzero columns equal to one half of the maximum possible for the row (and every row).
    }

#define TargetSubtileByteBudget 16000       // TODO: Tune!  Shoot for most subtiles fitting within TG 64KB L1 scratch pad (other architectures can cope with same budget), allowing for A, B, C, and other stuff.
// FIXME:  Not yet using the indications of how many non-zero elements there are in the input.  Also, not yet returning cTotalNumNzElements, cMaxNumNzColsInRow, or resultCode.

    u64 numBytesPerNonZero = sizeof(rowNum_t) +         // Since we expect the matrices to be very sparse indeed, it is likely that most non-zero elements will be the ONLY such element in
                             sizeof(numNzColsInRow_t) + // the row of the CRS tile.  Therefore, we will assume that we need one rowNum_t and one numNzColsInRow_t for each NZ, as well as its
                             sizeof(colIdx_t) +         // colIdx_t and BLAS_MATH_t.  The storate requirement for the representation of the CCS tiles of B will be pretty much the same.
                             sizeof(BLAS_MATH_t);
    u64 numNzElementsPerTile = TargetSubtileByteBudget / numBytesPerNonZero; // Budget of average number of nonzero elements per tile.
    u64 aTargetNumTiles = aTotalNumNzElements / numNzElementsPerTile;        // Number of tiles to shoot for, to approximate that budget.
    u64 bTargetNumTiles = bTotalNumNzElements / numNzElementsPerTile;        // Number of tiles to shoot for, to approximate that budget.
    aTargetNumTiles = (aTargetNumTiles * 8 / 5);                             // Only the uppermost non-zero bit is of interest.  E.g, round a budget of 1280 to 2559 to 2048 (a nice power of 2).
    bTargetNumTiles = (bTargetNumTiles * 8 / 5);
    u64 aNumTilesAcross = 16;                                                // Now jockey budgets into appropriate arrangment of horizontal and vertical tiling.  Tile at least 16-way in each direction.
    u64 aNumTilesDown   = 16;
    u64 bNumTilesAcross = 16;
    u64 bNumTilesDown   = 16;
    aTargetNumTiles >>= (4+4);
    bTargetNumTiles >>= (4+4);
    while (aTargetNumTiles > 1 && bTargetNumTiles > 1) {                                           // While there is more budget of both A tiles and B tiles to utilize...
        if (n / aNumTilesAcross > m / aNumTilesDown || n / bNumTilesDown > k / bNumTilesAcross) {  // Favor more granularity down columns of A and across rows of B:
            aNumTilesDown   <<= 1;
            bNumTilesAcross <<= 1;
        } else {                                                                                   // Favor more granularity across rows of A and down columns of B:
            aNumTilesAcross <<= 1;
            bNumTilesDown   <<= 1;
        }
        aTargetNumTiles >>= 1;
        bTargetNumTiles >>= 1;
    }
    while (aTargetNumTiles > 1 || aNumTilesDown * 65534L < m) {                                    // While there is more budget of A tiles to distribute:
        aNumTilesDown <<= 1;
        aTargetNumTiles >>= 1;
    }
    while (bTargetNumTiles > 1 || bNumTilesAcross * 65534L < n) {                                   // While there is more budget of B tiles to distribute:
        bNumTilesAcross <<= 1;
        bTargetNumTiles >>= 1;
    }
    while (aNumTilesAcross * 65534L < n || bNumTilesDown * 65534L < k) {                           // (No tile can be taller or wider than 65534 elements.)
        aNumTilesAcross <<= 1;
        bNumTilesDown   <<= 1;
    }
    u64 cNumTilesAcross = bNumTilesAcross;
    u64 cNumTilesDown   = aNumTilesDown;
    printf ("Workload to be split into\n");
    printf ("   %4ld tiles wide by %4ld tiles high for matrix A\n", (u64) aNumTilesAcross, (u64) aNumTilesDown);
    printf ("   %4ld tiles wide by %4ld tiles high for matrix B\n", (u64) bNumTilesAcross, (u64) bNumTilesDown);
    printf ("   %4ld tiles wide by %4ld tiles high for matrix C\n", (u64) cNumTilesAcross, (u64) cNumTilesDown); fflush(stdout);

// Create the EDT templates we will need down the full topology.

    ocrGuid_t factorInput_template;
    ocrGuid_t doFullMatMul_template;
    ocrGuid_t wrapup_template;
    ocrGuid_t populateCRSTiles_template;
    ocrGuid_t assayCCSTile_template;
    ocrGuid_t populateCCSTile_template;
    ocrGuid_t doStripOfC_template;
    ocrGuid_t doMosaicOfC_template;
    ocrGuid_t doTileOfC_template;
    ocrGuid_t doTileOfAxB_1_template;
    ocrGuid_t doTileOfAxB_2_template;
    ocrGuid_t collateMosaic_template;
    ocrGuid_t collateMosaic_prelude_template;
    ocrGuid_t collateIntoOutput_template;
    ocrGuid_t collateStripsOfC_template;


    ocrEdtTemplateCreate(&factorInput_template,  NAME(spmmm_factorInput_task),  sizeof(NAME(spmmm_factorInput_params_t))  / sizeof(u64), sizeof(NAME(spmmm_factorInput_deps_t))  / sizeof(ocrEdtDep_t));
    u64 sizeofParams = sizeof(NAME(spmmm_doFullMatMul_params_t)) / sizeof(u64);
    ocrEdtTemplateCreate(&doFullMatMul_template,     NAME(spmmm_doFullMatMul_task),     sizeofParams, sizeof(NAME(spmmm_doFullMatMul_deps_t))    / sizeof(ocrEdtDep_t));
    ocrEdtTemplateCreate(&wrapup_template,           NAME(spmmm_wrapup_task),           sizeofParams, sizeof(NAME(spmmm_wrapup_deps_t))          / sizeof(ocrEdtDep_t));
    sizeofParams = sizeof(NAME(spmmm_factorInput_params_t)) / sizeof(u64);
    ocrEdtTemplateCreate(&populateCRSTiles_template, NAME(spmmm_populateCRSTiles_task), sizeofParams, EDT_PARAM_UNK);
    ocrEdtTemplateCreate(&assayCCSTile_template,     NAME(spmmm_assayCCSTile_task),     sizeofParams, sizeof(NAME(spmmm_assayCCSTile_deps_t))    / sizeof(ocrEdtDep_t));
    ocrEdtTemplateCreate(&populateCCSTile_template,  NAME(spmmm_populateCCSTile_task),  sizeofParams, sizeof(NAME(spmmm_populateCCSTile_deps_t)) / sizeof(ocrEdtDep_t));
    sizeofParams = sizeof(NAME(spmmm_doMatMulCalcs_params_t))/sizeof(u64);
    ocrEdtTemplateCreate(&doStripOfC_template,            NAME(spmmm_doStripOfC_task),            sizeofParams, sizeof(NAME(spmmm_doStripOfC_deps_t))            / sizeof(ocrEdtDep_t));
    ocrEdtTemplateCreate(&doMosaicOfC_template,           NAME(spmmm_doMosaicOfC_task),           sizeofParams, sizeof(NAME(spmmm_doTileOrMosaicOfC_deps_t))     / sizeof(ocrEdtDep_t));
    ocrEdtTemplateCreate(&doTileOfC_template,             NAME(spmmm_doTileOfC_task),             sizeofParams, sizeof(NAME(spmmm_doTileOrMosaicOfC_deps_t))     / sizeof(ocrEdtDep_t));
    ocrEdtTemplateCreate(&doTileOfAxB_1_template,         NAME(spmmm_doTileOfAxB_1_task),         sizeofParams, sizeof(NAME(spmmm_doTileOfAxB_1_deps_t))         / sizeof(ocrEdtDep_t));
    ocrEdtTemplateCreate(&doTileOfAxB_2_template,         NAME(spmmm_doTileOfAxB_2_task),         sizeofParams, sizeof(NAME(spmmm_doTileOfAxB_2_deps_t))         / sizeof(ocrEdtDep_t));
    ocrEdtTemplateCreate(&collateMosaic_template,         NAME(spmmm_collateMosaic_task),         sizeofParams, sizeof(NAME(spmmm_collateMosaic_deps_t))         / sizeof(ocrEdtDep_t));
    ocrEdtTemplateCreate(&collateMosaic_prelude_template, NAME(spmmm_collateMosaic_prelude_task), sizeofParams, sizeof(NAME(spmmm_collateMosaic_prelude_deps_t)) / sizeof(ocrEdtDep_t));
    ocrEdtTemplateCreate(&collateIntoOutput_template,     NAME(spmmm_collateIntoOutput_task),     sizeofParams, sizeof(NAME(spmmm_collateIntoOutput_deps_t))     / sizeof(ocrEdtDep_t));
    ocrEdtTemplateCreate(&collateStripsOfC_template,      NAME(spmmm_collateStripsOfC_task),      sizeofParams, sizeof(NAME(spmmm_collateStripsOfC_deps_t))      / sizeof(ocrEdtDep_t));


// Create datablocks for the tile catalog.  Each element of the catalog itself will be used first for assaying the storage needs of the tile, then storing the GUIDs for the datablocks of those tiles.

    u64 * addr;            // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.)

    ocrGuid_t tileCatalogOfA;
    ocrGuid_t tileCatalogOfB;
    ocrDbCreate (&tileCatalogOfA, (void**) &addr, aNumTilesAcross * aNumTilesDown * sizeof(InputTileDescr_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    ocrDbCreate (&tileCatalogOfB, (void**) &addr, bNumTilesAcross * bNumTilesDown * sizeof(InputTileDescr_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);

// Create EDTs, providing them with their params, but withholding their dependencies until the next step.

    NAME(spmmm_factorInput_params_t) factorA_params;
    factorA_params.matrixHeight         = m;                    // Height of full matrix.
    factorA_params.matrixWidth          = n;                    // Width of full matrix.
    factorA_params.numTilesAcross       = aNumTilesAcross;      // Number of tiles across matrix.
    factorA_params.numTilesDown         = aNumTilesDown;        // Number of tiles down matrix.
    factorA_params.numNzColsInRow       = aNumNzColsInRow;      // Address of 1D array of non-zero element counts for each row of matrix.
    factorA_params.colIdx               = aColIdx;              // Address of 2D array containing non-zero column indices.
    factorA_params.val                  = aVal;                 // Address of 2D array containing non-zero elements.  (Not used in assaying, but used when tiles are subsequently populated.)
    factorA_params.ld                   = lda;                  // Leading dimension of the aColIdx array.
    factorA_params.firstRowNum          = 0;                    // Index of topmost row for this full-width strip.  (Recall that all strips are maximally interlaced.)
    factorA_params.numStripsToAssay     = aNumTilesDown;        // Do assays of this many strips.
    factorA_params.firstCol             = -9999;                // Only place this is used is to convey leftmost column index for tile being converted to CCS format.
    factorA_params.isA                  = true;                 // True for factoring A, false for factoring B.
    factorA_params.factorInput_template      = factorInput_template;      // Template to create EDT to split workloads of multiple strips to two children, or to factor a single strip to CRS and CCS tile format.
    factorA_params.populateCRSTiles_template = populateCRSTiles_template; // Template to create EDT to populate CRS tiles across full-width strip.
    factorA_params.assayCCSTile_template     = assayCCSTile_template;     // Template to create EDT to assay the storage requirements of a CCS tile.
    factorA_params.populateCCSTile_template  = populateCCSTile_template;  // Template to create EDT to populate CCS tile.

    NAME(spmmm_factorInput_params_t) factorB_params;
    factorB_params.matrixHeight         = n;                    // Height of full matrix.
    factorB_params.matrixWidth          = k;                    // Width of full matrix.
    factorB_params.numTilesAcross       = bNumTilesAcross;      // Number of tiles across matrix.
    factorB_params.numTilesDown         = bNumTilesDown;        // Number of tiles down matrix.
    factorB_params.numNzColsInRow       = bNumNzColsInRow;      // Address of 1D array of non-zero element counts for each row of matrix.
    factorB_params.colIdx               = bColIdx;              // Address of 2D array containing non-zero column indices.
    factorB_params.val                  = bVal;                 // Address of 2D array containing non-zero elements.  (Not used in assaying, but used when tiles are subsequently populated.)
    factorB_params.ld                   = ldb;                  // Leading dimension of the aColIdx array.
    factorB_params.firstRowNum          = 0;                    // Index of topmost row for this full-width strip.  (Recall that all strips are maximally interlaced.)
    factorB_params.numStripsToAssay     = bNumTilesDown;        // Do assays of this many strips.
    factorB_params.firstCol             = -9999;                // Only place this is used is to convey leftmost column index tile being converted to CCS format.
    factorB_params.isA                  = false;                // True for factoring A, false for factoring B.
    factorB_params.factorInput_template      = factorInput_template;      // Template to create EDT to split workloads of multiple strips to two children, or to factor a single strip to CRS and CCS tile format.
    factorB_params.populateCRSTiles_template = populateCRSTiles_template; // Template to create EDT to populate CRS tiles across full-width strip.
    factorB_params.assayCCSTile_template     = assayCCSTile_template;     // Template to create EDT to assay the storage requirements of a CCS tile.
    factorB_params.populateCCSTile_template  = populateCCSTile_template;  // Template to create EDT to populate CCS tile.

    ocrGuid_t factorA_edt;
    ocrGuid_t factorB_edt;
    ocrGuid_t factorA_doneEvent;
    ocrGuid_t factorB_doneEvent;
    ocrEdtCreate (&factorA_edt, factorInput_template, EDT_PARAM_DEF, ((u64 *) (&factorA_params)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &factorA_doneEvent);
    ocrEdtCreate (&factorB_edt, factorInput_template, EDT_PARAM_DEF, ((u64 *) (&factorB_params)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &factorB_doneEvent);


    NAME(spmmm_doFullMatMul_params_t) doFullMatMul_params;
    doFullMatMul_params.m                      = m;                     // Number of rows in matrix C (and in matrix A).
    doFullMatMul_params.cNumNzColsInRow        = cNumNzColsInRow;       // Address of 1D array to receive count of column indices and values for each row of C.
    doFullMatMul_params.cColIdx                = cColIdx;               // Address of 2D array to receive column indices of matrix C.
    doFullMatMul_params.cVal                   = cVal;                  // Address of 2D array to receive non-zero-valued elements of matrix C in ELL format.
    doFullMatMul_params.ldc                    = ldc;                   // Leading dimension of cVal and cColIdx arrays, aka the maximum number of non-zero-valued elements any row of C can accomodate.
    doFullMatMul_params.aNumTilesAcross        = aNumTilesAcross;       // Number of tiles across full-width strip of A.
    doFullMatMul_params.aNumTilesDown          = aNumTilesDown;         // Number of tiles down full-width strip of A.
    doFullMatMul_params.bNumTilesAcross        = bNumTilesAcross;       // Number of tiles across full-width strip of B.
    doFullMatMul_params.bNumTilesDown          = bNumTilesDown;         // Number of tiles down full-width strip of B.
    doFullMatMul_params.aTileRowNum            = 0;                     // Index of [first] strip to drive matrix multiply for.
    doFullMatMul_params.numStripsToDoMatMulFor = aNumTilesDown;         // Number of strips to drive matrix multiply for.
    doFullMatMul_params.cTotalNumNzElements    = cTotalNumNzElements;   // Output: NZs in matrix C.  If C is used as a source (A of B) for a subsequent ?spmmm call, providing this value will improve performance.
    doFullMatMul_params.cMaxNumNzColsInRow     = cMaxNumNzColsInRow;    // Address at which to return the number of non-zero elements in the most populous row of matrix C.
    doFullMatMul_params.resultCode             = resultCode;            // Returns zero if totally successful, else cast resultCode to SPMMM_RETURN_CODE_t to decipher problem.
    doFullMatMul_params.epsilon                = epsilon;               // When a result is between +/- epsilon, it is eliminated from the output, i.e. set to zero.
    doFullMatMul_params.doFullMatMul_template  = doFullMatMul_template; // Template to create EDT to drive matrix multipy for one or more strips.
    doFullMatMul_params.doStripOfC_template    = doStripOfC_template;
    doFullMatMul_params.doMosaicOfC_template   = doMosaicOfC_template;
    doFullMatMul_params.doTileOfC_template     = doTileOfC_template;
    doFullMatMul_params.doTileOfAxB_1_template = doTileOfAxB_1_template;
    doFullMatMul_params.doTileOfAxB_2_template = doTileOfAxB_2_template;
    doFullMatMul_params.collateMosaic_template = collateMosaic_template;
    doFullMatMul_params.collateMosaic_prelude_template = collateMosaic_prelude_template;
    doFullMatMul_params.collateIntoOutput_template     = collateIntoOutput_template;
    doFullMatMul_params.collateStripsOfC_template      = collateStripsOfC_template;

    ocrGuid_t doFullMatMul_edt;
    ocrGuid_t doFullMatMul_statsOut;
    ocrGuid_t wrapup_edt;
    ocrEdtCreate (&doFullMatMul_edt, doFullMatMul_template, EDT_PARAM_DEF, ((u64 *) (&doFullMatMul_params)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &doFullMatMul_statsOut);
    ocrEdtCreate (&wrapup_edt,       wrapup_template,       EDT_PARAM_DEF, ((u64 *) (&doFullMatMul_params)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

// Now provide each child EDT with its dependencies, in reverse order.

    ADD_DEPENDENCE(tileCatalogOfA,            wrapup_edt,       NAME(spmmm_wrapup_deps_t),       tileCatalogOfA,        RO);
    ADD_DEPENDENCE(tileCatalogOfB,            wrapup_edt,       NAME(spmmm_wrapup_deps_t),       tileCatalogOfB,        RO);
    ADD_DEPENDENCE(doFullMatMul_statsOut,     wrapup_edt,       NAME(spmmm_wrapup_deps_t),       matrixCStats,          RO);

    ADD_DEPENDENCE(tileCatalogOfA,            doFullMatMul_edt, NAME(spmmm_doFullMatMul_deps_t), tileCatalogOfA,        RO);
    ADD_DEPENDENCE(tileCatalogOfB,            doFullMatMul_edt, NAME(spmmm_doFullMatMul_deps_t), tileCatalogOfB,        RO);
    ADD_DEPENDENCE(factorA_doneEvent,         doFullMatMul_edt, NAME(spmmm_doFullMatMul_deps_t), serializationTrigger1, RO);
    ADD_DEPENDENCE(factorB_doneEvent,         doFullMatMul_edt, NAME(spmmm_doFullMatMul_deps_t), serializationTrigger2, RO);

    ADD_DEPENDENCE(tileCatalogOfB,            factorB_edt,      NAME(spmmm_factorInput_deps_t),  tileCatalog,          RW);
#ifdef SERIALIZEFORDEBUGGING
    ADD_DEPENDENCE(NULL_GUID,                 factorB_edt,      NAME(spmmm_factorInput_deps_t),  serializationTrigger,  RO);
#else
    ADD_DEPENDENCE(factorA_doneEvent,         factorB_edt,      NAME(spmmm_factorInput_deps_t),  serializationTrigger,  RO);
#endif
    ADD_DEPENDENCE(tileCatalogOfA,            factorA_edt,      NAME(spmmm_factorInput_deps_t),  tileCatalog,          RW);
    ADD_DEPENDENCE(NULL_GUID,                 factorA_edt,      NAME(spmmm_factorInput_deps_t),  serializationTrigger,  RO);


    printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmmThunk_task))); fflush(stdout);
} // ?spmmmThunk_task


static ocrGuid_t NAME(spmmm_factorInput_task) (  // One EDT created for factoring A, another for B.  Each creates two child EDTs to distribute full-width strips, until leafs each tile a single strip.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{
    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_factorInput_task))); fflush(stdout);
    NAME(spmmm_factorInput_params_t) * myParams = (NAME(spmmm_factorInput_params_t) *) paramv;
    NAME(spmmm_factorInput_deps_t)   * myDeps   = (NAME(spmmm_factorInput_deps_t)   *) depv;
    SpmmmIdx_t    matrixHeight         = myParams->matrixHeight;         // Height of full matrix.
    SpmmmIdx_t    matrixWidth          = myParams->matrixWidth;          // Width of full matrix.
    SpmmmIdx_t    numTilesAcross       = myParams->numTilesAcross;       // Number of tiles across matrix.
    SpmmmIdx_t    numTilesDown         = myParams->numTilesDown;         // Number of tiles down matrix.
    SpmmmIdx_t  * numNzColsInRow       = myParams->numNzColsInRow;       // Address of 1D array of non-zero element counts for each row of matrix.
    SpmmmIdx_t  * colIdx               = myParams->colIdx;               // Address of 2D array containing non-zero column indices.
    BLAS_MATH_t * val                  = myParams->val;                  // Address of 2D array containing non-zero elements.  (Not used in assaying, but used when tiles are subsequently populated.)
    SpmmmIdx_t    ld                   = myParams->ld;                   // Leading dimension of the aColIdx array.
    SpmmmIdx_t    firstRowNum          = myParams->firstRowNum;          // Index of topmost row for this full-width strip.  (Recall that all strips are maximally interlaced.)
    SpmmmIdx_t    numStripsToAssay     = myParams->numStripsToAssay;     // Do assays of this many strips.
    SpmmmIdx_t    firstCol             = myParams->firstCol;             // Only place this is used is to convey leftmost column index for tile being converted to CCS format.
    bool          isA                  = myParams->isA;                  // True for factoring A, false for factoring B.
    ocrGuid_t     factorInput_template = myParams->factorInput_template; // Template to create EDT to split workloads of multiple strips to two children, or to factor a single strip to CRS and CCS tile format.
    ocrGuid_t     populateCRSTiles_template = myParams->populateCRSTiles_template; // Template to create EDT to populate CRS tiles across full-width strip.


    if (numStripsToAssay > 1) {
// This EDT's workload is multiple full-width strips.  Split them between two child EDTs.
        myParams->numStripsToAssay = (numStripsToAssay >>= 1);
        ocrGuid_t gTileCatalog = myDeps->tileCatalog.guid;               // Datablock for collecting tile assay information, and then for recording GUID of datablocks containing tiles.
        ocrGuid_t upperStrips_edt;
        ocrGuid_t upperStrips_doneEvent;
        ocrGuid_t lowerStrips_edt;
        ocrEdtCreate (&upperStrips_edt, factorInput_template, EDT_PARAM_DEF, ((u64 *) (myParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &upperStrips_doneEvent);
        myParams->firstRowNum += myParams->numStripsToAssay;
        ocrEdtCreate (&lowerStrips_edt, factorInput_template, EDT_PARAM_DEF, ((u64 *) (myParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

        ADD_DEPENDENCE(gTileCatalog,          lowerStrips_edt, NAME(spmmm_factorInput_deps_t), tileCatalog, RW);
        ADD_DEPENDENCE(gTileCatalog,          upperStrips_edt, NAME(spmmm_factorInput_deps_t), tileCatalog, RW);
#ifdef SERIALIZEFORDEBUGGING
        ADD_DEPENDENCE(upperStrips_doneEvent, lowerStrips_edt, NAME(spmmm_factorInput_deps_t), serializationTrigger, RW);
#else
        ADD_DEPENDENCE(NULL_GUID,             lowerStrips_edt, NAME(spmmm_factorInput_deps_t), serializationTrigger, RW);
#endif
        ADD_DEPENDENCE(NULL_GUID,             upperStrips_edt, NAME(spmmm_factorInput_deps_t), serializationTrigger, RW);
    } else {
// This EDT's workload is single full-width strip.  Start the factoring process.

// Adjust pointers and pitch as appropriate for the strip that this instance is responsible to assay and factor.

        InputTileDescr_t * pTileCatalog = myDeps->tileCatalog.ptr;       // Datablock for collecting tile assay information, and then for recording GUID of datablocks containing tiles.
        numNzColsInRow   += firstRowNum;
        colIdx           += firstRowNum * ld;
        pTileCatalog     += firstRowNum * numTilesAcross;
        u64 pitch         = ld * numTilesDown;

//  Initialize assay:

        u64 i, rowNum, tileOfCol;
        for (i = 0; i < numTilesAcross; i++) {
            pTileCatalog[i].assayCRS.numRows       = 0;
            pTileCatalog[i].assayCRS.numNzElements = 0;
            pTileCatalog[i].assayCRS.currRow       = 0xFFFFFFFFL;
        }


//  Calculate assay:

        for (rowNum = firstRowNum; rowNum < matrixHeight; rowNum += numTilesDown) {  // For each row that is in this task instance's full-width strip:
            for (i = 0; i < *numNzColsInRow; i++) {
                tileOfCol = colIdx[i] & (numTilesAcross - 1);
                if (pTileCatalog[tileOfCol].assayCRS.currRow != rowNum) { // If this is the first nz column seen for this row of the tile to which the column belongs, note storage requirement of row descr.
                    pTileCatalog[tileOfCol].assayCRS.currRow = rowNum;
                    pTileCatalog[tileOfCol].assayCRS.numRows++;
                }
                pTileCatalog[tileOfCol].assayCRS.numNzElements++;
            }
            numNzColsInRow += numTilesDown;
            colIdx         += pitch;
        }

// Allocate CRS tile storage indicated by assay for each tile across the strip, and create EDT to populate nonzero tiles:

        for (i = 0; i < numTilesAcross; i++) {
            if (pTileCatalog[i].assayCRS.numRows != 0) {
                // Create the datablock to host this tile of A in CRS format.
                u64 storageRequirement = sizeof(CRSTileMeta_t);
                storageRequirement += 7LL;
                storageRequirement &= ~(7LL);
                storageRequirement += pTileCatalog[i].assayCRS.numRows * sizeof(rowNum_t);            // Size of the array of rowNum.  (rowNum contains the row number of each row that has some non-zeros).
                storageRequirement += 7LL;
                storageRequirement &= ~(7LL);
                storageRequirement += pTileCatalog[i].assayCRS.numRows * sizeof(numNzColsInRow_t);    // Size of the array of numNzColsInRow.  (This is the number of non-zeros in the row identified by rowNum).
                storageRequirement += 7LL;
                storageRequirement &= ~(7LL);
                storageRequirement += pTileCatalog[i].assayCRS.numNzElements * sizeof(colIdx_t);      // Size of the array of colIdx.  (These are the column indices of the nonzero elements.)
                storageRequirement += 7LL;
                storageRequirement &= ~(7LL);
                storageRequirement += pTileCatalog[i].assayCRS.numNzElements * sizeof(BLAS_MATH_t);  // Size of the array of val.  (These are the values of the nonzero elements.)
                storageRequirement += 7LL;
                storageRequirement &= ~(7LL);
                ocrGuid_t tile;
#ifdef TILEDEBUGGING
                ocrDbCreate (&pTileCatalog[i].tileCRS, (void**) &pTileCatalog[i].CRSptr, storageRequirement, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
#else
                u64 * addr;            // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.)
                ocrDbCreate (&pTileCatalog[i].tileCRS, (void**) &addr, storageRequirement, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
#endif
            } else { // This tile of A is entirely empty.  Just set its GUID in the tileCatalog to NULL_GUID.  No matrix multiplies are necessary for this tile of A, regardless of contents of tiles of B.
                pTileCatalog[i].tileCRS = NULL_GUID;
                pTileCatalog[i].tileCCS = NULL_GUID;
#ifdef TILEDEBUGGING
                pTileCatalog[i].CRSptr = NULL;
                pTileCatalog[i].CCSptr = NULL;
#endif
            }
        }

// Create EDT to populate the strip of tiles in CRS format.  That EDT will create further EDTs to assay and populate the same tiles in CCS format.

        ocrGuid_t  populateCRSTiles_edt;
        u64 numberOfDependences = sizeof(NAME(spmmm_populateCRSTiles_deps_t))/sizeof(ocrEdtDep_t)+numTilesAcross-1;
        ocrEdtCreate (&populateCRSTiles_edt, populateCRSTiles_template, EDT_PARAM_DEF, ((u64 *) (myParams)), numberOfDependences, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);

        for (i = 0; i < numTilesAcross; i++) {
            ADD_DEPENDENCE(pTileCatalog[i].tileCRS, populateCRSTiles_edt, NAME(spmmm_populateCRSTiles_deps_t), tile[i],  RW);
        }
        // Above, we have given EDT its dependences for the CRS tile datablocks.  Now release the catalog, and give that to the EDT too, so that it can use it to assay CCS tiles and then catalog their guids.
        ocrGuid_t gTileCatalog = myDeps->tileCatalog.guid;  // Datablock for collecting tile assay information, and then for recording GUID of datablocks containing tiles.
        ocrDbRelease(gTileCatalog);
        ADD_DEPENDENCE(gTileCatalog, populateCRSTiles_edt, NAME(spmmm_populateCRSTiles_deps_t), tileCatalog, RW);
    }

    //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_factorInput_task))); fflush(stdout);
    return NULL_GUID;
} // ?spmmm_factorInput_task


static ocrGuid_t NAME(spmmm_populateCRSTiles_task) ( //  Populate the CRS tiles across a full-width strip of matrix A.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{
    u64 i, rowNum, tileOfCol;

    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_populateCRSTiles_task))); fflush(stdout);
    NAME(spmmm_factorInput_params_t)    * myParams = (NAME(spmmm_factorInput_params_t)    *) paramv;
    NAME(spmmm_populateCRSTiles_deps_t) * myDeps   = (NAME(spmmm_populateCRSTiles_deps_t) *) depv;
    SpmmmIdx_t    matrixHeight         = myParams->matrixHeight;         // Height of full matrix.
    SpmmmIdx_t    matrixWidth          = myParams->matrixWidth;          // Width of full matrix.
    SpmmmIdx_t    numTilesAcross       = myParams->numTilesAcross;       // Number of tiles across matrix.
    SpmmmIdx_t    numTilesDown         = myParams->numTilesDown;         // Number of tiles down matrix.
    SpmmmIdx_t  * numNzColsInRow       = myParams->numNzColsInRow;       // Address of 1D array of non-zero element counts for each row of matrix.
    SpmmmIdx_t  * colIdx               = myParams->colIdx;               // Address of 2D array containing non-zero column indices.
    BLAS_MATH_t * val                  = myParams->val;                  // Address of 2D array containing non-zero elements.  (Not used in assaying, but used when tiles are subsequently populated.)
    SpmmmIdx_t    ld                   = myParams->ld;                   // Leading dimension of the aColIdx array.
    SpmmmIdx_t    firstRowNum          = myParams->firstRowNum;          // Index of topmost row for this full-width strip.  (Recall that all strips are maximally interlaced.)
    SpmmmIdx_t    numStripsToAssay     = myParams->numStripsToAssay;     // Do assays of this many strips.
    SpmmmIdx_t    firstCol             = myParams->firstCol;             // Only place this is used is to convey leftmost column index for tile being converted to CCS format.
    bool          isA                  = myParams->isA;                  // True for factoring A, false for factoring B.
    ocrGuid_t     factorInput_template = myParams->factorInput_template; // Template to create EDT to split workloads of multiple strips to two children, or to factor a single strip to CRS and CCS tile format.
    ocrGuid_t     assayCCSTile_template = myParams->assayCCSTile_template; // Template to assay storage requirements of a CCS tile.

    InputTileDescr_t * pTileCatalog = myDeps->tileCatalog.ptr;       // Datablock for consulting tile assay information, and then for recording GUID of datablocks containing tiles.
    ocrEdtDep_t      * ptile        = &myDeps->tile[0];              // Pointer into appropirate part of datablock for cataloging the GUIDs of tiles created for the CRS of A. NULL_GUID for CRS of B.

// TODO:  Posture the portion of the assayCatalog modified by this task.

// Adjust pointers and pitch as appropriate for the strip that this instance is responsible to assay and catalog.

    numNzColsInRow   += firstRowNum;
    colIdx           += firstRowNum * ld;
    val              += firstRowNum * ld;
    pTileCatalog     += firstRowNum * numTilesAcross;
    SpmmmIdx_t pitch  = ld * numTilesDown;

    for (i = 0; i < numTilesAcross; i++) {
        CRSTileMeta_t * tileMeta = (CRSTileMeta_t *) (ptile[i].ptr);
        if (tileMeta == NULL) continue;
        pTileCatalog[i].assayCRS.currRow = 0xFFFFFFFFL;
        tileMeta->numRows        = 0;
        tileMeta->numNzElements  = 0;
        tileMeta->offsetToRowNum         = ((((u64) sizeof(CRSTileMeta_t))+7LL)&(~7LL));
        tileMeta->offsetToNumNzColsInRow = tileMeta->offsetToRowNum         + (((((u64) sizeof(rowNum_t))         * pTileCatalog[i].assayCRS.numRows)      +7LL)&(~7LL));
        tileMeta->offsetToColIdx         = tileMeta->offsetToNumNzColsInRow + (((((u64) sizeof(numNzColsInRow_t)) * pTileCatalog[i].assayCRS.numRows)      +7LL)&(~7LL));
        tileMeta->offsetToVal            = tileMeta->offsetToColIdx         + (((((u64) sizeof(colIdx_t))         * pTileCatalog[i].assayCRS.numNzElements)+7LL)&(~7LL));
    }

    for (rowNum = firstRowNum; rowNum < matrixHeight; rowNum += numTilesDown) {  // For each row that is in this task instance's full-width strip:
        for (i = 0; i < *numNzColsInRow; i++) {
            tileOfCol = colIdx[i] & (numTilesAcross - 1);
            CRSTileMeta_t * tileMeta   = (CRSTileMeta_t *) (ptile[tileOfCol].ptr);
            rowNum_t         * pRowNum         = getAddrCRSTileRowNumArray(tileMeta);
            numNzColsInRow_t * pNumNzColsInRow = getAddrCRSTileNumNzColsInRowArray(tileMeta);
            SpmmmIdx_t       * pColIdx         = getAddrCRSTileColIdxArray(tileMeta);
            BLAS_MATH_t      * pVal            = getAddrCRSTileValArray(tileMeta);
            pRowNum         += tileMeta->numRows;
            pNumNzColsInRow += tileMeta->numRows;
            pColIdx         += tileMeta->numNzElements;
            pVal            += tileMeta->numNzElements;
            if (pTileCatalog[tileOfCol].assayCRS.currRow != rowNum) {   // If this is the first nonzero column seen for this row of the tile to which the column belongs, note storage requirement of row descr.
                pTileCatalog[tileOfCol].assayCRS.currRow = rowNum;
                *pRowNum++ = rowNum;
                *pNumNzColsInRow++ = 1;
                tileMeta->numRows++;
#ifdef SANITYCHECKING
                if (tileMeta->numRows > pTileCatalog[tileOfCol].assayCRS.numRows) {
                    printf ("ERROR:  numRows added to tile exceeds count of rows found in assay.\n");
                    ocrShutdown();
                }
#endif
            } else {
                pNumNzColsInRow[-1]++;
            }
            tileMeta->numNzElements++;
#ifdef SANITYCHECKING
            if (tileMeta->numNzElements > pTileCatalog[tileOfCol].assayCRS.numNzElements) {
                printf ("ERROR:  numNzElements added to tile exceeds count of nonzero elements found in assay.\n");
                ocrShutdown();
            }
#endif
            *pColIdx = colIdx[i];
            *pVal    = val[i];
        }
        numNzColsInRow += numTilesDown;
        colIdx         += pitch;
        val            += pitch;
    }

// Now create EDTs to assay the storage requirements for these tiles in CCS format.

    // Above, we have fleshed out the CRS tiles.  Now release the catalog, and give it to the EDTs we are creating, so that they can use it to assay CCS tiles and then catalog their guids.
    ocrGuid_t gTileCatalog = myDeps->tileCatalog.guid;  // Datablock for collecting tile assay information, and then for recording GUID of datablocks containing tiles.
    ocrDbRelease(gTileCatalog);

    for (i = 0; i < numTilesAcross; i++) {
        CRSTileMeta_t * tileMeta = (CRSTileMeta_t *) (ptile[i].ptr);
        if (tileMeta == NULL) continue;

        ocrGuid_t colIdxIdx;
        u64 * addr;            // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.)
        ocrDbCreate (&colIdxIdx, (void**) &addr, sizeof(colIdxIdx_t) * tileMeta->numRows, DB_PROP_NONE, NULL_GUID, NO_ALLOC);   // Scratch space for keeping track of progress across non-zero rows of CRS tile.
        ocrDbRelease(ptile[i].guid); // We wrote to the tile's CRS datablock, above.  Release it now, and pass it as a dependency to the CCS-assay EDT.
        ocrGuid_t assayCCSTile_edt;
        myParams->firstCol = i;
        ocrEdtCreate (&assayCCSTile_edt, assayCCSTile_template, EDT_PARAM_DEF, ((u64 *) (myParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);
        ADD_DEPENDENCE(gTileCatalog,  assayCCSTile_edt, NAME(spmmm_assayCCSTile_deps_t), tileCatalog, RW);
        ADD_DEPENDENCE(ptile[i].guid, assayCCSTile_edt, NAME(spmmm_assayCCSTile_deps_t), tileCRS,      RO);
        ADD_DEPENDENCE(colIdxIdx,     assayCCSTile_edt, NAME(spmmm_assayCCSTile_deps_t), colIdxIdx,   RW);
    }

    //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_populateCRSTiles_task))); fflush(stdout);
    return NULL_GUID;
} // ?spmmm_populateCRSTiles_task



static ocrGuid_t NAME(spmmm_assayCCSTile_task) ( //  Assay the storage needs for the CCS representation of a tile that is already in CRS format.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{

    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_assayCCSTile_task))); fflush(stdout);
    NAME(spmmm_factorInput_params_t) * myParams = (NAME(spmmm_factorInput_params_t) *) paramv;
    NAME(spmmm_assayCCSTile_deps_t)  * myDeps   = (NAME(spmmm_assayCCSTile_deps_t)  *) depv;
    SpmmmIdx_t    numTilesAcross       = myParams->numTilesAcross;       // Number of tiles across matrix.
    SpmmmIdx_t    firstRowNum          = myParams->firstRowNum;          // Index of topmost row for this full-width strip.  (Recall that all strips are maximally interlaced.)
    SpmmmIdx_t    firstCol             = myParams->firstCol;             // Only place this is used is to convey leftmost column index for tile being converted to CCS format.
    bool          isA                  = myParams->isA;                  // True for factoring A, false for factoring B.
    ocrGuid_t     populateCCSTile_template = myParams->populateCCSTile_template; // Template to populate a CCS tile.

    InputTileDescr_t * pTileDescr   = myDeps->tileCatalog.ptr;       // Datablock for recording GUID of datablock to contain CCS-format tile (to be passed down to populateCCSTile_edt
    CRSTileMeta_t    * pCRSTileMeta = myDeps->tileCRS.ptr;           // Datablock with tile in CRS format.
    colIdxIdx_t      * pColIdxIdx   = myDeps->colIdxIdx.ptr;         // Datablock of scratch space to keep track of progress across rows of CRS tile, as we find elements to move to CCS tile.
    ocrGuid_t          gColIdxIdx   = myDeps->colIdxIdx.guid;

// TODO:  Posture the portion of the pTileCatalog modified by this task.

// Adjust pointers as appropriate for the tile being operated upon.

    pTileDescr += firstRowNum * numTilesAcross + firstCol;

//  Initialize assay:

    CCSTileAssay_t assayCCS = {0};

//  Calculate assay:

    SpmmmIdx_t currColIdx;
    SpmmmIdx_t nextColIdx = 0;                                                                   // Init the next column number that we will seek after the current one.

    {
        u64 rowIdxIdx;
        for (rowIdxIdx = 0; rowIdxIdx < pCRSTileMeta->numRows; rowIdxIdx++) {                    // For each row in the CRS-formated tile...
            pColIdxIdx[rowIdxIdx] = 0;
        }
    }

   numNzColsInRow_t * pNumNzColsInRow = getAddrCRSTileNumNzColsInRowArray(pCRSTileMeta);
    do {
        bool isFirstForCurCol = true;
        currColIdx = nextColIdx;                                                                 // Init the current column number that we will search for in this pass over the rows.
        nextColIdx = 0xFFFFFFFFL;                                                                // Init the next column number that we will search for in the NEXT pass over the rows.
        colIdx_t         * pCRSColIdx      = getAddrCRSTileColIdxArray(pCRSTileMeta);
        u64 rowIdxIdx;
        for (rowIdxIdx = 0; rowIdxIdx < pCRSTileMeta->numRows; rowIdxIdx++) {                    // For each row in the CRS-formated tile...
            if (pColIdxIdx[rowIdxIdx] < pNumNzColsInRow[rowIdxIdx] &&                            // If there are more nonzero elements left in the current row, and...
                pCRSColIdx[pColIdxIdx[rowIdxIdx]] <= currColIdx) {                               // ... if it is the current column that we are seeking.
                if (pCRSColIdx[pColIdxIdx[rowIdxIdx]] < currColIdx) {                            // Scream bloody hell if the non-zero elements are NOT in canonical order!  TODO: canonize them!
                    printf("In %s, matrix %c is NOT in canonical order!\n", STRINGIFY(NAME(spmmm_assayForCCS_task)), isA ? 'A' : 'B');
                    ocrShutdown();
                }
                if (isFirstForCurCol) {
                    assayCCS.numCols++;                                                          // Bump count of nonzero columns (that have at least one non-zero).
                    isFirstForCurCol = false;
                }
                assayCCS.numNzElements++;                                                        // Account for size required by rowIdx and val.
                pColIdxIdx[rowIdxIdx]++;
            }
            if (pColIdxIdx[rowIdxIdx] < pNumNzColsInRow[rowIdxIdx]) {                            // If there are still non-zero columns remaining in the row beyond the column number we are seeking...
                nextColIdx = MIN(nextColIdx, pCRSColIdx[pColIdxIdx[rowIdxIdx]]);                 // ... then note whether it is the next column number that we should seek (i.e. in the next pass over the rows).
            }
            pCRSColIdx += pNumNzColsInRow[rowIdxIdx];
        }
    } while (nextColIdx != 0xFFFFFFFFL);                                                         // Quit when there are no more nonzero columns in any of the rows of this strip.

// Allocate tile storage indicated by assay:

    u64 storageRequirement = sizeof(CCSTileMeta_t);
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    storageRequirement += assayCCS.numCols * sizeof(colNum_t);            // Size of the array of colNum.  (colNum contains the column number of each column that has some non-zeros).
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    storageRequirement += assayCCS.numCols * sizeof(numNzRowsInCol_t);    // Size of the array of numNzRowsInCol.  (This is the number of non-zeros in the column identified by colNum).
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    if (!isA) {   // For CCS of A, we only need colNum and numNzRowsInCol arrays;  for CCS of B, we also need rowIdx and val arrays.
        storageRequirement += assayCCS.numNzElements * sizeof(rowIdx_t);      // Size of the array of rowIdx.  (These are the row indices of the nonzero elements.)
        storageRequirement += 7LL;
        storageRequirement &= ~(7LL);
        storageRequirement += assayCCS.numNzElements * sizeof(BLAS_MATH_t);  // Size of the array of val.  (These are the values of the nonzero elements.)
        storageRequirement += 7LL;
        storageRequirement &= ~(7LL);
    }
#ifdef TILEDEBUGGING
    ocrDbCreate (&pTileDescr->tileCCS, (void**) &pTileDescr->CCSptr, storageRequirement, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
#else
    u64 * addr;            // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.)
    ocrDbCreate (&pTileDescr->tileCCS, (void**) &addr, storageRequirement, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
#endif

    ocrGuid_t gTileCatalog = myDeps->tileCatalog.guid;
    ocrGuid_t gTileCRS     = myDeps->tileCRS.guid;
    ocrDbRelease(gTileCatalog);
    ocrDbRelease(gTileCRS);
    ocrDbRelease(gColIdxIdx);
    ocrGuid_t populateCCSTile_edt;
    myParams->assayCCS = assayCCS;
    ocrEdtCreate (&populateCCSTile_edt, populateCCSTile_template, EDT_PARAM_DEF, ((u64 *) (myParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, NULL);
    ADD_DEPENDENCE(gTileCatalog,        populateCCSTile_edt, NAME(spmmm_populateCCSTile_deps_t), tileCatalog, RW);
    ADD_DEPENDENCE(gTileCRS,            populateCCSTile_edt, NAME(spmmm_populateCCSTile_deps_t), tileCRS,      RO);
    ADD_DEPENDENCE(gColIdxIdx,          populateCCSTile_edt, NAME(spmmm_populateCCSTile_deps_t), colIdxIdx,   RW);
    ADD_DEPENDENCE(pTileDescr->tileCCS, populateCCSTile_edt, NAME(spmmm_populateCCSTile_deps_t), tileCCS,      RO);

    //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_assayCCSTile_task))); fflush(stdout);
    return NULL_GUID;
} // ?spmmm_assayCCSTile_task



static ocrGuid_t NAME(spmmm_populateCCSTile_task) ( //  Populate a CCS tile
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{

    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_populateCCSTile_task))); fflush(stdout);
    NAME(spmmm_factorInput_params_t) * myParams = (NAME(spmmm_factorInput_params_t) *) paramv;
    NAME(spmmm_populateCCSTile_deps_t)  * myDeps   = (NAME(spmmm_populateCCSTile_deps_t)  *) depv;
    SpmmmIdx_t     numTilesAcross       = myParams->numTilesAcross;       // Number of tiles across matrix.
    SpmmmIdx_t     firstRowNum          = myParams->firstRowNum;          // Index of topmost row for this full-width strip.  (Recall that all strips are maximally interlaced.)
    SpmmmIdx_t     firstCol             = myParams->firstCol;             // Only place this is used is to convey leftmost column index for tile being converted to CCS format.
    bool           isA                  = myParams->isA;                  // True for factoring A, false for factoring B.
    CCSTileAssay_t assayCCS             = myParams->assayCCS;

    InputTileDescr_t * pTileDescr   = myDeps->tileCatalog.ptr;       // Datablock for recording GUID of datablock to contain CCS-format tile (to be passed down to populateCCSTile_edt
    CRSTileMeta_t    * pCRSTileMeta = myDeps->tileCRS.ptr;           // Datablock with tile in CRS format.
    CCSTileMeta_t    * pCCSTileMeta = myDeps->tileCCS.ptr;           // Datablock with tile in CCS format, which we are to populate.
    colIdxIdx_t      * pColIdxIdx   = myDeps->colIdxIdx.ptr;         // Datablock of scratch space to keep track of progress across rows of CRS tile, as we find elements to move to CCS tile.
    ocrGuid_t          gColIdxIdx   = myDeps->colIdxIdx.guid;

// TODO:  Posture the portion of the pTileCatalog modified by this task.

// Adjust pointers as appropriate for the tile being operated upon.

    pTileDescr += firstRowNum * numTilesAcross + firstCol;

//  Initialize CCS tile contents:

    pCCSTileMeta->numCols        = 0;
    pCCSTileMeta->numNzElements  = 0;
    pCCSTileMeta->offsetToColNum         = ((((u64) sizeof(CRSTileMeta_t))+7LL)&(~7LL));
    pCCSTileMeta->offsetToNumNzRowsInCol = pCCSTileMeta->offsetToColNum         + (((((u64) sizeof(colNum_t))         * assayCCS.numCols)      +7LL)&(~7LL));
    pCCSTileMeta->offsetToRowIdx         = pCCSTileMeta->offsetToNumNzRowsInCol + (((((u64) sizeof(numNzRowsInCol_t)) * assayCCS.numCols)      +7LL)&(~7LL));
    pCCSTileMeta->offsetToVal            = pCCSTileMeta->offsetToRowIdx         + (((((u64) sizeof(rowIdx_t))         * assayCCS.numNzElements)+7LL)&(~7LL));
    colNum_t         * pColNum           = getAddrCCSTileColNumArray(pCCSTileMeta);
    numNzRowsInCol_t * pNumNzRowsInCol   = getAddrCCSTileNumNzRowsInColArray(pCCSTileMeta);
    SpmmmIdx_t       * pCCSRowIdx        = getAddrCCSTileRowIdxArray(pCCSTileMeta);
    BLAS_MATH_t      * pCCSVal           = getAddrCCSTileValArray(pCCSTileMeta);
    u64 numCols = 0;

//  Populate tile

    SpmmmIdx_t currColIdx;
    SpmmmIdx_t nextColIdx = 0;                                                                   // Init the next column number that we will seek after the current one.
    {
        u64 rowIdxIdx;
        for (rowIdxIdx = 0; rowIdxIdx < pCRSTileMeta->numRows; rowIdxIdx++) {                    // For each row in the CRS-formated tile...
            pColIdxIdx[rowIdxIdx] = 0;                                                           // Re-init the index over the colIdx array back to beginning (after having been trashed by assayCCS_edt).
        }
    }

    rowNum_t         * pRowNum         = getAddrCRSTileRowNumArray(pCRSTileMeta);
    numNzColsInRow_t * pNumNzColsInRow = getAddrCRSTileNumNzColsInRowArray(pCRSTileMeta);
    do {
        bool isFirstForCurCol = true;
        currColIdx = nextColIdx;                                                                 // Init the current column number that we will search for in this pass over the rows.
        nextColIdx = 0xFFFFFFFFL;                                                                // Init the next column number that we will search for in the NEXT pass over the rows.
        colIdx_t         * pCRSColIdx      = getAddrCRSTileColIdxArray(pCRSTileMeta);
        BLAS_MATH_t      * pCRSVal         = getAddrCRSTileValArray(pCRSTileMeta);
        u64 rowIdxIdx;
        for (rowIdxIdx = 0; rowIdxIdx < pCRSTileMeta->numRows; rowIdxIdx++) {                    // For each row in the CRS-formated tile...
            u64 rowNum = pRowNum[rowIdxIdx];
            if (pColIdxIdx[rowIdxIdx] < pNumNzColsInRow[rowIdxIdx] &&                            // If there are more nonzero elements left in the current row, and...
                pCRSColIdx[pColIdxIdx[rowIdxIdx]] == currColIdx) {                               // ... if it is the current column that we are seeking.
                if (isFirstForCurCol) {
                    pColNum[numCols] = currColIdx;
                    pNumNzRowsInCol[numCols] = 1;
                    numCols++;                                                                   // Bump count of nonzero columns (that have at least one non-zero).
                    isFirstForCurCol = false;
                } else {
                    pNumNzRowsInCol[numCols-1]++;
                }
                if (! isA) {                                                                     // RowIdx and Val are only wanted for B tiles, not A.
                    *pCCSRowIdx++ = rowNum;
                    *pCCSVal++    = pCRSVal[pColIdxIdx[rowIdxIdx]];
                    pCCSTileMeta->numNzElements++;
                }
                pColIdxIdx[rowIdxIdx]++;
            }
            if (pColIdxIdx[rowIdxIdx] < pNumNzColsInRow[rowIdxIdx]) {                          // If there are still non-zero columns remaining in the row beyond the column number we are seeking...
                nextColIdx = MIN(nextColIdx, pCRSColIdx[pColIdxIdx[rowIdxIdx]]);                 // ... then note whether it is the next column number that we should seek (i.e. in the next pass over the rows).
            }
            pCRSColIdx += pNumNzColsInRow[rowIdxIdx];
            pCRSVal    += pNumNzColsInRow[rowIdxIdx];
        }
    } while (nextColIdx != 0xFFFFFFFFL);                                                         // Quit when there are no more nonzero columns in any of the rows of this strip.
    pCCSTileMeta->numCols = numCols;
    ocrDbRelease(gColIdxIdx);
    ocrDbDestroy(gColIdxIdx);

    //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_populateCCSTile_task))); fflush(stdout);
    return NULL_GUID;
} // ?spmmm_populateCCSTiles_task


static ocrGuid_t NAME(spmmm_doFullMatMul_task) (   // Drive the full matrix multiply, once the CRS tiles of A and CCS tiles of B are in hand, and the CCS assays of A and CRS assays of B.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{
    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_doFullMatMul_task))); fflush(stdout);

    NAME(spmmm_doFullMatMul_params_t) * myParams = (NAME(spmmm_doFullMatMul_params_t) *) paramv;
    NAME(spmmm_doFullMatMul_deps_t)   * myDeps   = (NAME(spmmm_doFullMatMul_deps_t)   *) depv;
    SpmmmIdx_t    m                  = myParams->m;                    // Number of rows in matrix C (and in matrix A).
    SpmmmIdx_t  * cNumNzColsInRow    = myParams->cNumNzColsInRow;      // Address of 1D array of count on non-zero elements in each row of C.
    SpmmmIdx_t  * cColIdx            = myParams->cColIdx;              // Address of 2D array of column indices of non-zero elements of matrix C.
    BLAS_MATH_t * cVal               = myParams->cVal;                 // Address of 2D array of non-zero elements of matrix C.
    SpmmmIdx_t    ldc                = myParams->ldc;                  // Leading dimension of cVal and cColIdx arrays, aka the maximum number of non-zero-valued elements any row of C can accomodate.
    SpmmmIdx_t    aNumTilesAcross    = myParams->aNumTilesAcross;      // SpmmmIdx_t    aNumTilesAcross;     // Number of tiles across full-width strip of A.
    SpmmmIdx_t    aNumTilesDown      = myParams->aNumTilesDown;        // SpmmmIdx_t    aNumTilesDown;       // Number of tiles down full-width strip of A.
    SpmmmIdx_t    bNumTilesAcross    = myParams->bNumTilesAcross;      // SpmmmIdx_t    bNumTilesAcross;     // Number of tiles across full-width strip of B.
    SpmmmIdx_t    bNumTilesDown      = myParams->bNumTilesDown;        // SpmmmIdx_t    bNumTilesDown;       // Number of tiles down full-width strip of B.
    SpmmmIdx_t    aTileRowNum        = myParams->aTileRowNum;          // SpmmmIdx_t    aTileRowNum;         // Index of [first] strip to drive matrix multiply for.
    SpmmmIdx_t    numStripsToDoMatMulFor = myParams->numStripsToDoMatMulFor; // SpmmmIdx_t numStripsToDoMatMulFor; // Number of strips to drive matrix multiplye for.
    ocrGuid_t     doFullMatMul_template  = myParams->doFullMatMul_template;  // Template to create EDT to drive matrix multipye for one or more strips.
    ocrGuid_t     collateStripsOfC_template = myParams->collateStripsOfC_template;
    ocrGuid_t     gTileCatalogOfA = myDeps->tileCatalogOfA.guid;       // GUID of datablock containing array of GUIDs, one for each CRS-format tile of matrix A.  (NULL_GUID for tiles entirely zero-valued.)
    ocrGuid_t     gTileCatalogOfB = myDeps->tileCatalogOfB.guid;       // GUID of datablock containing array of GUIDs, one for each CRS-format tile of matrix B.  (NULL_GUID for tiles entirely zero-valued.)

    SpmmmIdx_t cTileRowNum = aTileRowNum;

    if (numStripsToDoMatMulFor > 1) {
        myParams->numStripsToDoMatMulFor = (numStripsToDoMatMulFor >>= 1);
        ocrGuid_t doUpperFullMatMulEdt;
        ocrGuid_t doUpperFullMatMulStatsOut;
        ocrEdtCreate (&doUpperFullMatMulEdt, doFullMatMul_template, EDT_PARAM_DEF, ((u64 *) (myParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &doUpperFullMatMulStatsOut);

        myParams->aTileRowNum += numStripsToDoMatMulFor;
        ocrGuid_t doLowerFullMatMulEdt;
        ocrGuid_t doLowerFullMatMulStatsOut;
        ocrEdtCreate (&doLowerFullMatMulEdt, doFullMatMul_template, EDT_PARAM_DEF, ((u64 *) (myParams)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &doLowerFullMatMulStatsOut);

        ocrGuid_t collateStripsOfCEdt;
        ocrGuid_t collateStripsOfCDoneEvent;
        NAME(spmmm_collateStripsOfC_params_t) collateStripsOfC_params;
        ocrEdtCreate (&collateStripsOfCEdt, collateStripsOfC_template, EDT_PARAM_DEF, ((u64 *) &collateStripsOfC_params), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &collateStripsOfCDoneEvent);

        ADD_DEPENDENCE(doUpperFullMatMulStatsOut,  collateStripsOfCEdt, NAME(spmmm_collateStripsOfC_deps_t), stripOfCStats_0,    RO);
        ADD_DEPENDENCE(doLowerFullMatMulStatsOut,  collateStripsOfCEdt, NAME(spmmm_collateStripsOfC_deps_t), stripOfCStats_1,    RO);

        ADD_DEPENDENCE(gTileCatalogOfA,            doLowerFullMatMulEdt, NAME(spmmm_doFullMatMul_deps_t), tileCatalogOfA,        RO);
        ADD_DEPENDENCE(gTileCatalogOfB,            doLowerFullMatMulEdt, NAME(spmmm_doFullMatMul_deps_t), tileCatalogOfB,        RO);
#ifdef SERIALIZEFORDEBUGGING
        ADD_DEPENDENCE(doUpperFullMatMulStatsOut,  doLowerFullMatMulEdt, NAME(spmmm_doFullMatMul_deps_t), serializationTrigger1, RO);
#else
        ADD_DEPENDENCE(NULL_GUID,                  doLowerFullMatMulEdt, NAME(spmmm_doFullMatMul_deps_t), serializationTrigger1, RO);
#endif
        ADD_DEPENDENCE(NULL_GUID,                  doLowerFullMatMulEdt, NAME(spmmm_doFullMatMul_deps_t), serializationTrigger2, RO);
        ADD_DEPENDENCE(gTileCatalogOfA,            doUpperFullMatMulEdt, NAME(spmmm_doFullMatMul_deps_t), tileCatalogOfA,        RO);
        ADD_DEPENDENCE(gTileCatalogOfB,            doUpperFullMatMulEdt, NAME(spmmm_doFullMatMul_deps_t), tileCatalogOfB,        RO);
        ADD_DEPENDENCE(NULL_GUID,                  doUpperFullMatMulEdt, NAME(spmmm_doFullMatMul_deps_t), serializationTrigger1, RO);
        ADD_DEPENDENCE(NULL_GUID,                  doUpperFullMatMulEdt, NAME(spmmm_doFullMatMul_deps_t), serializationTrigger2, RO);
        //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_doFullMatMul_task))); fflush(stdout);
        return collateStripsOfCDoneEvent;

    } else {
        NAME(spmmm_doMatMulCalcs_params_t) doStripOfC_params;
        doStripOfC_params.m                                = m;
        doStripOfC_params.cNumNzColsInRow                  = cNumNzColsInRow;
        doStripOfC_params.cColIdx                          = cColIdx;
        doStripOfC_params.cVal                             = cVal;
        doStripOfC_params.ldc                              = ldc;
        doStripOfC_params.aNumTilesAcross                  = aNumTilesAcross;
        doStripOfC_params.aNumTilesDown                    = aNumTilesDown;
        doStripOfC_params.bNumTilesAcross                  = bNumTilesAcross;
        doStripOfC_params.bNumTilesDown                    = bNumTilesDown;
        doStripOfC_params.firstRowOfStripOfC               = cTileRowNum;
        doStripOfC_params.firstColOfC                      = -9999;  // Filled in by doStripOfC_task, for doMosaicOfC_task, and doMosaicOfC_task for doTileOfC_task.
        doStripOfC_params.numTilesOfMosaic                 = -9999;  // Filled in by doMosaicOfC_task.
        doStripOfC_params.firstColOfTileOfAAndRowOfTileOfB = -9999;  // Filled in by doTileOfC_task, for doTileOfAxB_<n>_task's.
        doStripOfC_params.numRowsInTileOfCWithNzElements   = -9999;  // Filled in by doTileAxB_1_task, for doTileOfAxB_2_task.
        doStripOfC_params.numNzElementsInTileOfC           = -9999;  // Filled in by doTileAxB_1_task, for doTileOfAxB_2_task.
        doStripOfC_params.epsilon                          = myParams->epsilon;
        doStripOfC_params.doMosaicOfC_template             = myParams->doMosaicOfC_template;
        doStripOfC_params.doTileOfC_template               = myParams->doTileOfC_template;
        doStripOfC_params.doTileOfAxB_1_template           = myParams->doTileOfAxB_1_template;
        doStripOfC_params.doTileOfAxB_2_template           = myParams->doTileOfAxB_2_template;
        doStripOfC_params.collateMosaic_template           = myParams->collateMosaic_template;
        doStripOfC_params.collateMosaic_prelude_template   = myParams->collateMosaic_prelude_template;
        doStripOfC_params.collateIntoOutput_template       = myParams->collateIntoOutput_template;
        doStripOfC_params.collateStripsOfC_template        = myParams->collateStripsOfC_template;

        ocrGuid_t doStripOfC_StatsOut;
        ocrGuid_t doStripOfC_edt;
        ocrEdtCreate (&doStripOfC_edt, myParams->doStripOfC_template, EDT_PARAM_DEF, ((u64 *) (&doStripOfC_params)), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &doStripOfC_StatsOut);


        ADD_DEPENDENCE(gTileCatalogOfA,       doStripOfC_edt, NAME(spmmm_doStripOfC_deps_t), tileCatalogOfA,        RO);
        ADD_DEPENDENCE(gTileCatalogOfB,       doStripOfC_edt, NAME(spmmm_doStripOfC_deps_t), tileCatalogOfB,        RO);
        ADD_DEPENDENCE(NULL_GUID,             doStripOfC_edt, NAME(spmmm_doStripOfC_deps_t), serializationTrigger,  RO);
        //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_doFullMatMul_task))); fflush(stdout);
        return doStripOfC_StatsOut;
    }

} // ?spmmm_doFullMatMul_task


static ocrGuid_t NAME(spmmm_doStripOfC_task) (  // Drive full matrix multiply across a single full-width strip of C (by multiplying the corresponding full-width strip of A by successive full-height strips of B).
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{
    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_doStripOfC_task))); fflush(stdout);
    NAME(spmmm_doMatMulCalcs_params_t) * myParams = (NAME(spmmm_doMatMulCalcs_params_t) *) paramv;
    NAME(spmmm_doStripOfC_deps_t)      * myDeps   = (NAME(spmmm_doStripOfC_deps_t)      *) depv;
    SpmmmIdx_t    m                              = myParams->m;
    SpmmmIdx_t  * cNumNzColsInRow                = myParams->cNumNzColsInRow;
    SpmmmIdx_t    firstRowOfStripOfC             = myParams->firstRowOfStripOfC;
    SpmmmIdx_t    aNumTilesDown                  = myParams->aNumTilesDown;
    SpmmmIdx_t    bNumTilesAcross                = myParams->bNumTilesAcross;
    ocrGuid_t     doMosaicOfC_template           = myParams->doMosaicOfC_template;
    ocrGuid_t     doTileOfC_template             = myParams->doTileOfC_template;
    ocrGuid_t     collateMosaic_prelude_template = myParams->collateMosaic_prelude_template;
    ocrGuid_t     collateIntoOutput_template     = myParams->collateIntoOutput_template;
    ocrGuid_t     gTileCatalogOfA                = myDeps->tileCatalogOfA.guid;
    ocrGuid_t     gTileCatalogOfB                = myDeps->tileCatalogOfB.guid;

    SpmmmIdx_t    cNumTilesAcross                = bNumTilesAcross;

// Split the workload of processing the full-width strip of C into two half-sized workloads, called the left and right "mosaics", each comprised of the full processing of half the tiles of C across the strip.

    myParams->numTilesOfMosaic = cNumTilesAcross >> 1;
    myParams->firstColOfC = 0;
    ocrGuid_t leftMosaic_edt;
    ocrGuid_t leftMosaic_datablock;
    ocrEdtCreate (&leftMosaic_edt,  doMosaicOfC_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &leftMosaic_datablock);

    myParams->firstColOfC = myParams->numTilesOfMosaic;
    ocrGuid_t rightMosaic_edt;
    ocrGuid_t rightMosaic_datablock;
    ocrEdtCreate (&rightMosaic_edt, doMosaicOfC_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &rightMosaic_datablock);

    ocrGuid_t collateIntoOutput_edt;
    ocrGuid_t collateIntoOutput_StatsOut;
    ocrGuid_t statisticsForStripOfC;
    u64 * addr;            // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.)
    ocrDbCreate (&statisticsForStripOfC, (void**) &addr, sizeof(statisticsForStripOfC_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    ocrEdtCreate (&collateIntoOutput_edt, collateIntoOutput_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &collateIntoOutput_StatsOut);

// Plumb the dependences, in reverse order.

    ADD_DEPENDENCE(leftMosaic_datablock,  collateIntoOutput_edt, NAME(spmmm_collateIntoOutput_deps_t), mosaicCRSofC_0,        RO);
    ADD_DEPENDENCE(rightMosaic_datablock, collateIntoOutput_edt, NAME(spmmm_collateIntoOutput_deps_t), mosaicCRSofC_1,        RO);
    ADD_DEPENDENCE(statisticsForStripOfC, collateIntoOutput_edt, NAME(spmmm_collateIntoOutput_deps_t), statisticsForStripOfC, RW);

    ADD_DEPENDENCE(gTileCatalogOfA,       rightMosaic_edt,       NAME(spmmm_doTileOrMosaicOfC_deps_t), tileCatalogOfA,        RO);
    ADD_DEPENDENCE(gTileCatalogOfB,       rightMosaic_edt,       NAME(spmmm_doTileOrMosaicOfC_deps_t), tileCatalogOfB,        RO);
#ifdef SERIALIZEFORDEBUGGING
    ADD_DEPENDENCE(leftMosaic_datablock,  rightMosaic_edt,       NAME(spmmm_doTileOrMosaicOfC_deps_t), serializationTrigger,  RO);
#else
    ADD_DEPENDENCE(NULL_GUID,             rightMosaic_edt,       NAME(spmmm_doTileOrMosaicOfC_deps_t), serializationTrigger,  RO);
#endif

    ADD_DEPENDENCE(gTileCatalogOfA,       leftMosaic_edt,        NAME(spmmm_doTileOrMosaicOfC_deps_t), tileCatalogOfA,        RO);
    ADD_DEPENDENCE(gTileCatalogOfB,       leftMosaic_edt,        NAME(spmmm_doTileOrMosaicOfC_deps_t), tileCatalogOfB,        RO);
    ADD_DEPENDENCE(NULL_GUID,             leftMosaic_edt,        NAME(spmmm_doTileOrMosaicOfC_deps_t), serializationTrigger,  RO);

    //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_doStripOfC_task))); fflush(stdout);
    return collateIntoOutput_StatsOut;
} // ?spmmm_doStripOfC_task


static ocrGuid_t NAME(spmmm_doMosaicOfC_task) ( // Drive full matrix multiply across a single partial-width mosaic of C, splitting it if it is more than one tile, or else driving the multiply of a single tile.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{
    NAME(spmmm_doMatMulCalcs_params_t) * myParams = (NAME(spmmm_doMatMulCalcs_params_t) *) paramv;
    NAME(spmmm_doStripOfC_deps_t)      * myDeps   = (NAME(spmmm_doStripOfC_deps_t)      *) depv;
    SpmmmIdx_t    m                              = myParams->m;
    SpmmmIdx_t  * cNumNzColsInRow                = myParams->cNumNzColsInRow;
    SpmmmIdx_t    firstRowOfStripOfC             = myParams->firstRowOfStripOfC;
    SpmmmIdx_t    aNumTilesAcross                = myParams->aNumTilesAcross;
    SpmmmIdx_t    aNumTilesDown                  = myParams->aNumTilesDown;
    SpmmmIdx_t    bNumTilesAcross                = myParams->bNumTilesAcross;
    SpmmmIdx_t    firstColOfC                    = myParams->firstColOfC;
    SpmmmIdx_t    numTilesOfMosaic               = myParams->numTilesOfMosaic;
    ocrGuid_t     doTileOfC_template             = myParams->doTileOfC_template;
    ocrGuid_t     collateMosaic_prelude_template = myParams->collateMosaic_prelude_template;
    ocrGuid_t     collateIntoOutput_template     = myParams->collateIntoOutput_template;
    ocrGuid_t     doMosaicOfC_template           = myParams->doMosaicOfC_template;
    ocrGuid_t     gTileCatalogOfA                = myDeps->tileCatalogOfA.guid;
    ocrGuid_t     gTileCatalogOfB                = myDeps->tileCatalogOfB.guid;

    SpmmmIdx_t    cNumTilesAcross                = bNumTilesAcross;
    //printf ("Entered %s function. C[%ld][%ld:%ld]\n", STRINGIFY(NAME(spmmm_doMosaicOfC_task)), (u64) firstRowOfStripOfC, (u64) firstColOfC, (u64) (firstColOfC+numTilesOfMosaic-1)); fflush(stdout);

    ocrGuid_t resultDatablock;
    if (numTilesOfMosaic > 1) {

// Split the workload of processing the multi-tile mosaic of C into two half-sized workloads, called the left and right mosaics.

        myParams->numTilesOfMosaic = (numTilesOfMosaic >>= 1);
        ocrGuid_t leftMosaic_edt;
        ocrGuid_t leftMosaic_datablock;
        ocrEdtCreate (&leftMosaic_edt,  doMosaicOfC_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &leftMosaic_datablock);

        myParams->firstColOfC += numTilesOfMosaic;
        ocrGuid_t rightMosaic_edt;
        ocrGuid_t rightMosaic_datablock;
        ocrEdtCreate (&rightMosaic_edt, doMosaicOfC_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &rightMosaic_datablock);

        ocrGuid_t combinedResult_edt;
        ocrEdtCreate (&combinedResult_edt, collateMosaic_prelude_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &resultDatablock);

// Plumb the dependences, in reverse order.

        ADD_DEPENDENCE(leftMosaic_datablock,  combinedResult_edt, NAME(spmmm_collateMosaic_prelude_deps_t), mosaicCRSofC_0,       RO);
        ADD_DEPENDENCE(rightMosaic_datablock, combinedResult_edt, NAME(spmmm_collateMosaic_prelude_deps_t), mosaicCRSofC_1,       RO);

        ADD_DEPENDENCE(gTileCatalogOfA,       rightMosaic_edt,    NAME(spmmm_doTileOrMosaicOfC_deps_t),     tileCatalogOfA,       RO);
        ADD_DEPENDENCE(gTileCatalogOfB,       rightMosaic_edt,    NAME(spmmm_doTileOrMosaicOfC_deps_t),     tileCatalogOfB,       RO);
#ifdef SERIALIZEFORDEBUGGING
        ADD_DEPENDENCE(leftMosaic_datablock,  rightMosaic_edt,    NAME(spmmm_doTileOrMosaicOfC_deps_t),     serializationTrigger, RO);
#else
        ADD_DEPENDENCE(NULL_GUID,             rightMosaic_edt,    NAME(spmmm_doTileOrMosaicOfC_deps_t),     serializationTrigger, RO);
#endif

        ADD_DEPENDENCE(gTileCatalogOfA,       leftMosaic_edt,     NAME(spmmm_doTileOrMosaicOfC_deps_t),     tileCatalogOfA,       RO);
        ADD_DEPENDENCE(gTileCatalogOfB,       leftMosaic_edt,     NAME(spmmm_doTileOrMosaicOfC_deps_t),     tileCatalogOfB,       RO);
        ADD_DEPENDENCE(NULL_GUID,             leftMosaic_edt,     NAME(spmmm_doTileOrMosaicOfC_deps_t),     serializationTrigger, RO);
    } else {
        myParams->numTilesOfMosaic = aNumTilesAcross;
        myParams->firstColOfTileOfAAndRowOfTileOfB = 0;
        ocrGuid_t tileOfC_edt;
        ocrEdtCreate (&tileOfC_edt, doTileOfC_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &resultDatablock);
        ADD_DEPENDENCE(gTileCatalogOfA,       tileOfC_edt,        NAME(spmmm_doTileOrMosaicOfC_deps_t),     tileCatalogOfA,       RO);
        ADD_DEPENDENCE(gTileCatalogOfB,       tileOfC_edt,        NAME(spmmm_doTileOrMosaicOfC_deps_t),     tileCatalogOfB,       RO);
        ADD_DEPENDENCE(NULL_GUID,             tileOfC_edt,        NAME(spmmm_doTileOrMosaicOfC_deps_t),     serializationTrigger, RO);
    }

    //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_doMosaicOfC_task))); fflush(stdout);
    return resultDatablock;
} // ?spmmm_doMosaicOfC_task

static ocrGuid_t NAME(spmmm_doTileOfC_task) (   // Drive the full matrix multiply for a single tile of C.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {

    NAME(spmmm_doMatMulCalcs_params_t)   * myParams = (NAME(spmmm_doMatMulCalcs_params_t) *) paramv;
    NAME(spmmm_doTileOrMosaicOfC_deps_t) * myDeps   = (NAME(spmmm_doTileOrMosaicOfC_deps_t)      *) depv;
    SpmmmIdx_t    firstRowOfStripOfC     = myParams->firstRowOfStripOfC;
    SpmmmIdx_t    firstColOfC            = myParams->firstColOfC;
    SpmmmIdx_t    firstColOfTileOfAAndRowOfTileOfB = myParams->firstColOfTileOfAAndRowOfTileOfB;
    SpmmmIdx_t    aNumTilesAcross        = myParams->aNumTilesAcross;
    SpmmmIdx_t    aNumTilesDown          = myParams->aNumTilesDown;
    SpmmmIdx_t    bNumTilesAcross        = myParams->bNumTilesAcross;
    SpmmmIdx_t    bNumTilesDown          = myParams->bNumTilesDown;
    SpmmmIdx_t    numTilesOfMosaic       = myParams->numTilesOfMosaic;
    ocrGuid_t     doTileOfC_template     = myParams->doTileOfC_template;
    ocrGuid_t     collateMosaic_prelude_template = myParams->collateMosaic_prelude_template;
    ocrGuid_t     doTileOfAxB_1_template = myParams->doTileOfAxB_1_template;
    ocrGuid_t     gTileCatalogOfA        = myDeps->tileCatalogOfA.guid;
    ocrGuid_t     gTileCatalogOfB        = myDeps->tileCatalogOfB.guid;
    InputTileDescr_t * pTileCatalogOfA   = myDeps->tileCatalogOfA.ptr;
    InputTileDescr_t * pTileCatalogOfB   = myDeps->tileCatalogOfB.ptr;

    SpmmmIdx_t cTileColNum = firstColOfC;
    SpmmmIdx_t bTileColNum = cTileColNum;
    SpmmmIdx_t cTileRowNum = firstRowOfStripOfC;
    SpmmmIdx_t aTileRowNum = cTileRowNum;
    SpmmmIdx_t aTileColNum = firstColOfTileOfAAndRowOfTileOfB;
    SpmmmIdx_t bTileRowNum = firstColOfTileOfAAndRowOfTileOfB;

    //printf ("Entered %s function. C[%ld][%ld] = A[%ld][%ld:%ld]*B[%ld:%ld][%ld]\n",
        //STRINGIFY(NAME(spmmm_doTileOfC_task)),
        //(u64) cTileRowNum, (u64) cTileColNum,
        //(u64) aTileRowNum, (u64) aTileColNum, (u64) (aTileColNum+numTilesOfMosaic-1),
        //(u64) bTileRowNum, (u64) (bTileRowNum+numTilesOfMosaic-1), (u64) bTileColNum);  fflush(stdout);

    ocrGuid_t combinedResult_datablock;

    if (numTilesOfMosaic > 1) {
        myParams->numTilesOfMosaic >>= 1;
        ocrGuid_t leftPartialAccumulation_edt;
        ocrGuid_t leftPartialAccumulation_datablock;
        ocrEdtCreate (&leftPartialAccumulation_edt,  doTileOfC_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &leftPartialAccumulation_datablock);

        myParams->firstColOfTileOfAAndRowOfTileOfB += myParams->numTilesOfMosaic;
        ocrGuid_t rightPartialAccumulation_edt;
        ocrGuid_t rightPartialAccumulation_datablock;
        ocrEdtCreate (&rightPartialAccumulation_edt, doTileOfC_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &rightPartialAccumulation_datablock);

        ocrGuid_t combinedResult_edt;
        ocrEdtCreate (&combinedResult_edt, collateMosaic_prelude_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &combinedResult_datablock);

// Plumb the dependences, in reverse order.

        ADD_DEPENDENCE(leftPartialAccumulation_datablock,  combinedResult_edt, NAME(spmmm_collateMosaic_prelude_deps_t), mosaicCRSofC_0, RO);
        ADD_DEPENDENCE(rightPartialAccumulation_datablock, combinedResult_edt, NAME(spmmm_collateMosaic_prelude_deps_t), mosaicCRSofC_1, RO);

        ADD_DEPENDENCE(gTileCatalogOfA,       rightPartialAccumulation_edt, NAME(spmmm_doTileOrMosaicOfC_deps_t), tileCatalogOfA,       RO);
        ADD_DEPENDENCE(gTileCatalogOfB,       rightPartialAccumulation_edt, NAME(spmmm_doTileOrMosaicOfC_deps_t), tileCatalogOfB,       RO);
#ifdef SERIALIZEFORDEBUGGING
        ADD_DEPENDENCE(leftPartialAccumulation_datablock,  rightPartialAccumulation_edt, NAME(spmmm_doTileOrMosaicOfC_deps_t), serializationTrigger, RO);
#else
        ADD_DEPENDENCE(NULL_GUID,             rightPartialAccumulation_edt, NAME(spmmm_doTileOrMosaicOfC_deps_t), serializationTrigger, RO);
#endif

        ADD_DEPENDENCE(gTileCatalogOfA,       leftPartialAccumulation_edt,  NAME(spmmm_doTileOrMosaicOfC_deps_t), tileCatalogOfA,       RO);
        ADD_DEPENDENCE(gTileCatalogOfB,       leftPartialAccumulation_edt,  NAME(spmmm_doTileOrMosaicOfC_deps_t), tileCatalogOfB,       RO);
        ADD_DEPENDENCE(NULL_GUID,             leftPartialAccumulation_edt,  NAME(spmmm_doTileOrMosaicOfC_deps_t), serializationTrigger, RO);
    } else {
        ocrGuid_t doTileOfAxB_1_edt;

        ocrGuid_t gTileMetaCRSofA = pTileCatalogOfA[aTileRowNum*aNumTilesAcross+aTileColNum].tileCRS;
        ocrGuid_t gTileMetaCCSofA = pTileCatalogOfA[aTileRowNum*aNumTilesAcross+aTileColNum].tileCCS;
        ocrGuid_t gTileMetaCRSofB = pTileCatalogOfB[bTileRowNum*bNumTilesAcross+bTileColNum].tileCRS;
        ocrGuid_t gTileMetaCCSofB = pTileCatalogOfB[bTileRowNum*bNumTilesAcross+bTileColNum].tileCCS;
        if (gTileMetaCRSofA == NULL_GUID || gTileMetaCCSofB == NULL_GUID) {
            combinedResult_datablock = NULL_GUID;
        } else {
            ocrEdtCreate (&doTileOfAxB_1_edt, doTileOfAxB_1_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &combinedResult_datablock);
            ADD_DEPENDENCE(gTileMetaCRSofA, doTileOfAxB_1_edt, NAME(spmmm_doTileOfAxB_1_deps_t), tileCRSofA, RO);
            ADD_DEPENDENCE(gTileMetaCCSofA, doTileOfAxB_1_edt, NAME(spmmm_doTileOfAxB_1_deps_t), tileCCSofA, RO);
            ADD_DEPENDENCE(gTileMetaCRSofB, doTileOfAxB_1_edt, NAME(spmmm_doTileOfAxB_1_deps_t), tileCRSofB, RO);
            ADD_DEPENDENCE(gTileMetaCCSofB, doTileOfAxB_1_edt, NAME(spmmm_doTileOfAxB_1_deps_t), tileCCSofB, RO);
        }
    }

    //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_doTileOfC_task))); fflush(stdout);
    return combinedResult_datablock;
} // ?spmmm_doTileOfC_task


static ocrGuid_t NAME(spmmm_doTileOfAxB_1_task) (   // Do step1 (storage allocation of C) for a single tile of A times a corresponding tile of B.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {

    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_doTileOfAxB_1_task))); fflush(stdout);
    NAME(spmmm_doMatMulCalcs_params_t) * myParams = (NAME(spmmm_doMatMulCalcs_params_t) *) paramv;
    NAME(spmmm_doTileOfAxB_1_deps_t)   * myDeps   = (NAME(spmmm_doTileOfAxB_1_deps_t)      *) depv;
    SpmmmIdx_t      firstRowOfStripOfC               = myParams->firstRowOfStripOfC;
    SpmmmIdx_t      firstColOfC                      = myParams->firstColOfC;
    SpmmmIdx_t      firstColOfTileOfAAndRowOfTileOfB = myParams->firstColOfTileOfAAndRowOfTileOfB ;
    SpmmmIdx_t      aNumTilesDown                    = myParams->aNumTilesDown;
    SpmmmIdx_t      bNumTilesDown                    = myParams->bNumTilesDown;
    ocrGuid_t       doTileOfAxB_2_template           = myParams->doTileOfAxB_2_template;
    CRSTileMeta_t * pTileMetaCRSofA                  = myDeps->tileCRSofA.ptr;
    CCSTileMeta_t * pTileMetaCCSofA                  = myDeps->tileCCSofA.ptr;
    CRSTileMeta_t * pTileMetaCRSofB                  = myDeps->tileCRSofB.ptr;
    ocrGuid_t       gTileMetaCRSofA                  = myDeps->tileCRSofA.guid;
    ocrGuid_t       gTileMetaCCSofB                  = myDeps->tileCCSofB.guid;

    SpmmmIdx_t aTileColNum = firstColOfTileOfAAndRowOfTileOfB;
    SpmmmIdx_t bTileRowNum = aTileColNum;
    SpmmmIdx_t cTileColNum = firstColOfC;
    SpmmmIdx_t bTileColNum = cTileColNum;
    SpmmmIdx_t cTileRowNum = firstRowOfStripOfC;
    SpmmmIdx_t aTileRowNum = cTileRowNum;

    ocrGuid_t resultAccTileCRSofC;
    colNum_t         * pColNum_CCSofA         = getAddrCCSTileColNumArray(pTileMetaCCSofA);
    numNzRowsInCol_t * pNumNzRowsInCol_CCSofA = getAddrCCSTileNumNzRowsInColArray(pTileMetaCCSofA);
    rowNum_t         * pRowNum_CRSofB         = getAddrCRSTileRowNumArray(pTileMetaCRSofB);
    numNzColsInRow_t * pNumNzColsInRow_CRSofB = getAddrCRSTileNumNzColsInRowArray(pTileMetaCRSofB);
    u64 i = 0;
    u64 j = 0;
    u64 numNzElementsInTileOfC = 0;
    u64 numRowsInTileOfCWithNzElements = pTileMetaCRSofA->numRows;
    while (i < pTileMetaCCSofA->numCols && j < pTileMetaCRSofB->numRows) {
        if (pColNum_CCSofA[i] == pRowNum_CRSofB[j]) {
            numNzElementsInTileOfC += pNumNzRowsInCol_CCSofA[i] * pNumNzColsInRow_CRSofB[j];
            i++;
            j++;
        } else if (pColNum_CCSofA[i] < pRowNum_CRSofB[j]) {
            i++;
        } else {
            j++;
        }
    }

    if (numRowsInTileOfCWithNzElements != 0) {
        u64 storageRequirement = sizeof(CRSTileMeta_t);
        storageRequirement += 7LL;
        storageRequirement &= ~(7LL);
        storageRequirement += numRowsInTileOfCWithNzElements * sizeof(rowNum_t);            // Size of the array of rowNum.  (rowNum contains the row number of each row that has some non-zeros).
        storageRequirement += 7LL;
        storageRequirement &= ~(7LL);
        storageRequirement += numRowsInTileOfCWithNzElements * sizeof(numNzColsInRow_t);    // Size of the array of numNzColsInRow.  (This is the number of non-zeros in the row identified by rowNum).
        storageRequirement += 7LL;
        storageRequirement &= ~(7LL);
        storageRequirement += numNzElementsInTileOfC * sizeof(colIdx_t);                    // Size of the array of colIdx.  (These are the column indices of the nonzero elements.)
        storageRequirement += 7LL;
        storageRequirement &= ~(7LL);
        storageRequirement += numNzElementsInTileOfC * sizeof(BLAS_MATH_t);                 // Size of the array of val.  (These are the values of the nonzero elements.)
        storageRequirement += 7LL;
        storageRequirement &= ~(7LL);
        u64 * addr;            // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.)
        ocrGuid_t thisTileOfAxB;
        ocrDbCreate (&thisTileOfAxB,  (void**) &addr, storageRequirement, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
        ocrGuid_t doTileOfAxB_2_edt;
        myParams->numRowsInTileOfCWithNzElements = numRowsInTileOfCWithNzElements;
        myParams->numNzElementsInTileOfC         = numNzElementsInTileOfC;
        ocrEdtCreate (&doTileOfAxB_2_edt, doTileOfAxB_2_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &resultAccTileCRSofC);
        ADD_DEPENDENCE(gTileMetaCRSofA, doTileOfAxB_2_edt, NAME(spmmm_doTileOfAxB_2_deps_t), tileCRSofA,   RO);
        ADD_DEPENDENCE(gTileMetaCCSofB, doTileOfAxB_2_edt, NAME(spmmm_doTileOfAxB_2_deps_t), tileCCSofB,   RO);
        ADD_DEPENDENCE(thisTileOfAxB,   doTileOfAxB_2_edt, NAME(spmmm_doTileOfAxB_2_deps_t), tileCRSofAxB, RW);
    } else {
        resultAccTileCRSofC = NULL_GUID;
    }

    //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_doTileOfAxB_1_task))); fflush(stdout);
    return resultAccTileCRSofC;
} // ?spmmm_doTileOfAxB_1_task


static ocrGuid_t NAME(spmmm_doTileOfAxB_2_task) (   // Do step2 (actual matrix multiply calculation) for a single tile of A times a corresponding tile of B.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {

    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_doTileOfAxB_2_task))); fflush(stdout);
    NAME(spmmm_doMatMulCalcs_params_t) * myParams = (NAME(spmmm_doMatMulCalcs_params_t) *) paramv;
    NAME(spmmm_doTileOfAxB_2_deps_t)   * myDeps   = (NAME(spmmm_doTileOfAxB_2_deps_t)      *) depv;
    SpmmmIdx_t    firstRowOfStripOfC     = myParams->firstRowOfStripOfC;
    SpmmmIdx_t    firstColOfC            = myParams->firstColOfC;
    SpmmmIdx_t    numRowsInTileOfCWithNzElements = myParams->numRowsInTileOfCWithNzElements;
    SpmmmIdx_t    numNzElementsInTileOfC         = myParams->numNzElementsInTileOfC;
    ocrGuid_t     collateMosaic_template = myParams->collateMosaic_template;

    ocrGuid_t       g_cTile      = myDeps->tileCRSofAxB.guid;
    CRSTileMeta_t * p_cTile      = (CRSTileMeta_t *) myDeps->tileCRSofAxB.ptr;
    CRSTileMeta_t * p_aTile      = (CRSTileMeta_t *) myDeps->tileCRSofA.ptr;
    CCSTileMeta_t * p_bTile      = (CCSTileMeta_t *) myDeps->tileCCSofB.ptr;

    p_cTile->numRows = 0;
    p_cTile->numNzElements = 0;
    p_cTile->offsetToRowNum         = ((((u64) sizeof(CRSTileMeta_t))+7LL)&(~7LL));
    p_cTile->offsetToNumNzColsInRow = p_cTile->offsetToRowNum         + (((((u64) sizeof(rowNum_t))         * numRowsInTileOfCWithNzElements)+7LL)&(~7LL));
    p_cTile->offsetToColIdx         = p_cTile->offsetToNumNzColsInRow + (((((u64) sizeof(numNzColsInRow_t)) * numRowsInTileOfCWithNzElements)+7LL)&(~7LL));
    p_cTile->offsetToVal            = p_cTile->offsetToColIdx         + (((((u64) sizeof(colIdx_t))         * numNzElementsInTileOfC)        +7LL)&(~7LL));

    rowNum_t         * p_cRowNum         = getAddrCRSTileRowNumArray(p_cTile);
    numNzColsInRow_t * p_cNumNzColsInRow = getAddrCRSTileNumNzColsInRowArray(p_cTile);
    colIdx_t         * p_cColIdx         = getAddrCRSTileColIdxArray(p_cTile);
    BLAS_MATH_t      * p_cVal            = getAddrCRSTileValArray(p_cTile);

    rowNum_t         * p_aRowNum         = getAddrCRSTileRowNumArray(p_aTile);
    numNzColsInRow_t * p_aNumNzColsInRow = getAddrCRSTileNumNzColsInRowArray(p_aTile);
    colIdx_t         * p_aColIdx         = getAddrCRSTileColIdxArray(p_aTile);
    BLAS_MATH_t      * p_aVal            = getAddrCRSTileValArray(p_aTile);

    colNum_t         * p_bColNum         = getAddrCCSTileColNumArray(p_bTile);
    numNzRowsInCol_t * p_bNumNzRowsInCol = getAddrCCSTileNumNzRowsInColArray(p_bTile);
    rowIdx_t         * p_bRowIdx         = getAddrCCSTileRowIdxArray(p_bTile);
    BLAS_MATH_t      * p_bVal            = getAddrCCSTileValArray(p_bTile);

// Compute tile C results for this tile of A times this tile of B.  Compute C in row-by-row order, column-by-column in each row.
// The only rows of C that might possibly have any nonzero elements are those for which the same row in A have nonzero elements.

    u64 aRowIdxIdx;
    u64 bColIdxIdx;

    for (aRowIdxIdx = 0; aRowIdxIdx < p_aTile->numRows; aRowIdxIdx++) {  // For each of the rows of A that have one or more non-zero elements.
        u64 aRowNum = p_aRowNum[aRowIdxIdx];
        u64 cRowNum = aRowNum;
        p_bColNum         = getAddrCCSTileColNumArray(p_bTile);
        p_bNumNzRowsInCol = getAddrCCSTileNumNzRowsInColArray(p_bTile);
        p_bRowIdx         = getAddrCCSTileRowIdxArray(p_bTile);
        p_bVal            = getAddrCCSTileValArray(p_bTile);
        bool thisRowOfCEmptySoFar = true;
        for (bColIdxIdx = 0; bColIdxIdx < p_bTile->numCols; bColIdxIdx++) {  // For each of the columns of B that have one or more non-zero elements.
            u64 bColNum = p_bColNum[bColIdxIdx];
            u64 cColNum = bColNum;
            BLAS_MATH_t acc;
            xSETc (acc, 0.0, 0.0);
            bool isNonzeroC = false;
            SpmmmIdx_t idx_aColIdx = 0;
            SpmmmIdx_t idx_bRowIdx = 0;
            while (idx_aColIdx < p_aNumNzColsInRow[aRowIdxIdx] && idx_bRowIdx < p_bNumNzRowsInCol[bColIdxIdx]) {
                if (p_aColIdx[idx_aColIdx] == p_bRowIdx[idx_bRowIdx]) {  // Found a contributor to this element of C:
                    xFMA(acc, p_aVal[idx_aColIdx], p_bVal[idx_bRowIdx]);
                    isNonzeroC = true;
                    idx_aColIdx++;
                    idx_bRowIdx++;
                } else if (p_aColIdx[idx_aColIdx] < p_bRowIdx[idx_bRowIdx]) {
                    idx_aColIdx++;
                } else {
                    idx_bRowIdx++;
                }
            }
            if (isNonzeroC) {   // Found one or more contributors to this element of C.
                if (thisRowOfCEmptySoFar) {
                    p_cTile->numRows++;
                    *p_cRowNum++ = cRowNum;
                    *p_cNumNzColsInRow++ = 0;
                    thisRowOfCEmptySoFar = false;
                }
                p_cNumNzColsInRow[-1]++;
                p_cTile->numNzElements++;
                *p_cColIdx++ = cColNum;
                *p_cVal++ = acc;
            }
            p_bRowIdx += p_bNumNzRowsInCol[bColIdxIdx];
            p_bVal    += p_bNumNzRowsInCol[bColIdxIdx];
        }
        p_aColIdx += p_aNumNzColsInRow[aRowIdxIdx];
        p_aVal    += p_aNumNzColsInRow[aRowIdxIdx];
    }

    //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_doTileOfAxB_2_task))); fflush(stdout);
    return g_cTile;
} // ?spmmm_doTileOfAxB_2_task


static ocrGuid_t NAME(spmmm_collateMosaic_prelude_task) ( // Preamble for collateMosaic, needed for collation of final tiles of C into mosaic, or combining mosaics into bigger ones. Allocates storage for newAcc.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {
    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_collateMosaic_prelude_task))); fflush(stdout);
    NAME(spmmm_doMatMulCalcs_params_t)       * myParams = (NAME(spmmm_doMatMulCalcs_params_t) *)        paramv;
    NAME(spmmm_collateMosaic_prelude_deps_t) * myDeps   = (NAME(spmmm_collateMosaic_prelude_deps_t) *) depv;

    SpmmmIdx_t    firstRowOfStripOfC     = myParams->firstRowOfStripOfC;

    ocrGuid_t gCRS0 = myDeps->mosaicCRSofC_0.guid;
    ocrGuid_t gCRS1 = myDeps->mosaicCRSofC_1.guid;
    if (gCRS0 == NULL_GUID && gCRS1 == NULL_GUID) {
        //printf ("        %s function exiting, just returning NULL_GUID. firstRowOfStripOrC = %ld\n", STRINGIFY(NAME(spmmm_collateMosaic_prelude_task)), (u64) firstRowOfStripOfC); fflush(stdout);
        return NULL_GUID;
    }
    if (gCRS1 == NULL_GUID) {
        CRSTileMeta_t * pCRS0Meta = (CRSTileMeta_t *) myDeps->mosaicCRSofC_0.ptr;
        //printf ("        %s function exiting, just returning gCRS0 = 0x%lx/0x%lx. firstRowOfStripOrC = %ld\n", STRINGIFY(NAME(spmmm_collateMosaic_prelude_task)), (u64) gCRS0, (u64) pCRS0Meta, (u64) firstRowOfStripOfC); fflush(stdout);
        return gCRS0;
    }
    if (gCRS0 == NULL_GUID) {
        CRSTileMeta_t * pCRS1Meta = (CRSTileMeta_t *) myDeps->mosaicCRSofC_1.ptr;
        //printf ("        %s function exiting, just returning gCRS1 = 0x%lx/0x%lx. firstRowOfStripOrC = %ld\n", STRINGIFY(NAME(spmmm_collateMosaic_prelude_task)), (u64) gCRS1, (u64) pCRS1Meta, (u64) firstRowOfStripOfC); fflush(stdout);
        return gCRS1;
    }

    ocrGuid_t       collateMosaic_template = myParams->collateMosaic_template;
    CRSTileMeta_t * pCRS0Meta = (CRSTileMeta_t *) myDeps->mosaicCRSofC_0.ptr;
    CRSTileMeta_t * pCRS1Meta = (CRSTileMeta_t *) myDeps->mosaicCRSofC_1.ptr;

    u64 storageRequirement = sizeof(CRSTileMeta_t);
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    storageRequirement += (pCRS0Meta->numRows + pCRS1Meta->numRows) * sizeof(rowNum_t);            // Size of the array of rowNum.  (rowNum contains the row number of each row that has some non-zeros).
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    storageRequirement += (pCRS0Meta->numRows + pCRS1Meta->numRows) * sizeof(numNzColsInRow_t);    // Size of the array of numNzColsInRow.  (This is the number of non-zeros in the row identified by rowNum).
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    storageRequirement += (pCRS0Meta->numNzElements + pCRS1Meta->numNzElements) * sizeof(colIdx_t); // Size of the array of colIdx.  (These are the column indices of the nonzero elements.)
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    storageRequirement += (pCRS0Meta->numNzElements + pCRS1Meta->numNzElements) * sizeof(BLAS_MATH_t); // Size of the array of val.  (These are the values of the nonzero elements.)
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    u64 * addr;                       // Catch the datablock address here.  (But we do not dereference this pointer; it is junk to this function.)
    ocrGuid_t dbNewAcc;
    ocrGuid_t collateMosaic_edt;
    ocrGuid_t gNewAcc;
    ocrDbCreate (&dbNewAcc, (void**) &addr, storageRequirement, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    ocrEdtCreate (&collateMosaic_edt, collateMosaic_template, EDT_PARAM_DEF, ((u64 *) myParams), EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &gNewAcc);
    ADD_DEPENDENCE(gCRS0,    collateMosaic_edt, NAME(spmmm_collateMosaic_deps_t), mosaicCRSofC_0, RO);
    ADD_DEPENDENCE(gCRS1,    collateMosaic_edt, NAME(spmmm_collateMosaic_deps_t), mosaicCRSofC_1, RO);
    ADD_DEPENDENCE(dbNewAcc, collateMosaic_edt, NAME(spmmm_collateMosaic_deps_t), newAcc,         RW);
    //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_collateMosaic_prelude_task))); fflush(stdout);
    return gNewAcc;
} // ?spmmm_collateMosaic_prelude_task


static ocrGuid_t NAME(spmmm_collateMosaic_task) (   // Accumulate and collate with previous contributors to single tile (or mosaic) of C).
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {

    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_collateMosaic_task))); fflush(stdout);
    NAME(spmmm_doMatMulCalcs_params_t) * myParams = (NAME(spmmm_doMatMulCalcs_params_t) *) paramv;
    NAME(spmmm_collateMosaic_deps_t)   * myDeps   = (NAME(spmmm_collateMosaic_deps_t)      *) depv;
    SpmmmIdx_t    firstRowOfStripOfC     = myParams->firstRowOfStripOfC;

    CRSTileMeta_t * pCRS0Meta   = (CRSTileMeta_t *) myDeps->mosaicCRSofC_0.ptr;
    CRSTileMeta_t * pCRS1Meta   = (CRSTileMeta_t *) myDeps->mosaicCRSofC_1.ptr;
    CRSTileMeta_t * pNewAccMeta = (CRSTileMeta_t *) myDeps->newAcc.ptr;
    ocrGuid_t       gCRS0       =                   myDeps->mosaicCRSofC_0.guid;
    ocrGuid_t       gCRS1       =                   myDeps->mosaicCRSofC_1.guid;
    ocrGuid_t       gNewAcc     =                   myDeps->newAcc.guid;

    pNewAccMeta->numRows        = (pCRS0Meta->numRows + pCRS1Meta->numRows);             // Subsequently adjusted downwards by detection of common rows.
    pNewAccMeta->numNzElements  = (pCRS0Meta->numNzElements + pCRS1Meta->numNzElements); // Subsequently adjusted downwards by detection of common nz elements.
    pNewAccMeta->offsetToRowNum         = ((((u64) sizeof(CRSTileMeta_t))+7LL)&(~7LL));
    pNewAccMeta->offsetToNumNzColsInRow = pNewAccMeta->offsetToRowNum         + (((((u64) sizeof(rowNum_t))         * (pCRS0Meta->numRows       + pCRS1Meta->numRows))      +7LL)&(~7LL));
    pNewAccMeta->offsetToColIdx         = pNewAccMeta->offsetToNumNzColsInRow + (((((u64) sizeof(numNzColsInRow_t)) * (pCRS0Meta->numRows       + pCRS1Meta->numRows))      +7LL)&(~7LL));
    pNewAccMeta->offsetToVal            = pNewAccMeta->offsetToColIdx         + (((((u64) sizeof(colIdx_t))         * (pCRS0Meta->numNzElements + pCRS1Meta->numNzElements))+7LL)&(~7LL));

    rowNum_t         * pCRS0RowNum           = getAddrCRSTileRowNumArray(pCRS0Meta);
    numNzColsInRow_t * pCRS0NumNzColsInRow   = getAddrCRSTileNumNzColsInRowArray(pCRS0Meta);
    colIdx_t         * pCRS0ColIdx           = getAddrCRSTileColIdxArray(pCRS0Meta);
    BLAS_MATH_t      * pCRS0Val              = getAddrCRSTileValArray(pCRS0Meta);

    rowNum_t         * pCRS1RowNum           = getAddrCRSTileRowNumArray(pCRS1Meta);
    numNzColsInRow_t * pCRS1NumNzColsInRow   = getAddrCRSTileNumNzColsInRowArray(pCRS1Meta);
    colIdx_t         * pCRS1ColIdx           = getAddrCRSTileColIdxArray(pCRS1Meta);
    BLAS_MATH_t      * pCRS1Val              = getAddrCRSTileValArray(pCRS1Meta);

    rowNum_t         * pNewAccRowNum         = getAddrCRSTileRowNumArray(pNewAccMeta);
    numNzColsInRow_t * pNewAccNumNzColsInRow = getAddrCRSTileNumNzColsInRowArray(pNewAccMeta);
    colIdx_t         * pNewAccColIdx         = getAddrCRSTileColIdxArray(pNewAccMeta);
    BLAS_MATH_t      * pNewAccVal            = getAddrCRSTileValArray(pNewAccMeta);

    SpmmmIdx_t accTileRowDescrDownCtr  = pCRS0Meta->numRows;
    SpmmmIdx_t thisTileRowDescrDownCtr = pCRS1Meta->numRows;
#if 0
    {
    u64 i, j;
    SpmmmIdx_t    firstColOfC      = myParams->firstColOfC;
    printf ("\n**** ACCUMULATION of C[%ld][%ld]:\n", (u64) firstRowOfStripOfC, (u64) firstColOfC);
    printf ("CRS0 ACC tile contents:    numRows = %ld    numNzElements = %ld   offsetToColIdx = %ld    offsetToVal = %ld, addr of CRS0 is 0x%lx\n",
        (u64) pCRS0Meta->numRows, (u64) pCRS0Meta->numNzElements, (u64) pCRS0Meta->offsetToColIdx, (u64) pCRS0Meta->offsetToVal, (u64) pCRS0Meta);
    for (i = 0; i < pCRS0Meta->numRows; i++) {
        printf ("Row %ld:\n", (u64) (*pCRS0RowNum));
        for (j = 0; j < *pCRS0NumNzColsInRow; j++) {
            printf ("        Column %ld:   Value = (%f, %f)\n", (u64) (*pCRS0ColIdx), xREAL(*pCRS0Val), xIMAG(*pCRS0Val));
            pCRS0ColIdx++;
            pCRS0Val++;
        }
        pCRS0RowNum++;
        pCRS0NumNzColsInRow++;
    }
    pCRS0RowNum           = getAddrCRSTileRowNumArray(pCRS0Meta);
    pCRS0NumNzColsInRow   = getAddrCRSTileNumNzColsInRowArray(pCRS0Meta);
    pCRS0ColIdx           = getAddrCRSTileColIdxArray(pCRS0Meta);
    pCRS0Val              = getAddrCRSTileValArray(pCRS0Meta);
    printf ("CRS1 ACC tile contents:    numRows = %ld    numNzElements = %ld   offsetToColIdx = %ld    offsetToVal = %ld, addr of CRS1 is 0x%lx\n",
        (u64) pCRS1Meta->numRows, (u64) pCRS1Meta->numNzElements, (u64) pCRS1Meta->offsetToColIdx, (u64) pCRS1Meta->offsetToVal, (u64) pCRS1Meta);
    for (i = 0; i < pCRS1Meta->numRows; i++) {
        printf ("Row %ld:\n", (u64) (*pCRS1RowNum));
        for (j = 0; j < *pCRS1NumNzColsInRow; j++) {
            printf ("        Column %ld:   Value = (%f, %f)\n", (u64) (*pCRS1ColIdx), xREAL(*pCRS1Val), xIMAG(*pCRS1Val));
            pCRS1ColIdx++;
            pCRS1Val++;
        }
        pCRS1RowNum++;
        pCRS1NumNzColsInRow++;
    }
    pCRS1RowNum           = getAddrCRSTileRowNumArray(pCRS1Meta);
    pCRS1NumNzColsInRow   = getAddrCRSTileNumNzColsInRowArray(pCRS1Meta);
    pCRS1ColIdx           = getAddrCRSTileColIdxArray(pCRS1Meta);
    pCRS1Val              = getAddrCRSTileValArray(pCRS1Meta);
    }
    printf ("New Acc:  numRows = %ld, numNzElements = %ld, offsetToRowNum = %ld, offsetToNumNzColsInRow = %ld, offsetToColIdx = %ld, offsetToVal = %ld\n", (u64) pNewAccMeta->numRows, (u64) pNewAccMeta->numNzElements, (u64) pNewAccMeta->offsetToRowNum, (u64) pNewAccMeta->offsetToNumNzColsInRow, (u64) pNewAccMeta->offsetToColIdx, (u64) pNewAccMeta->offsetToVal);
#endif

    while (accTileRowDescrDownCtr > 0 && thisTileRowDescrDownCtr > 0) {
        if (*pCRS0RowNum == *pCRS1RowNum) {                   // Nonzero elements in the same row of both the previous accumulation and this tile of C.  Need to collate/accumulate.
            pNewAccMeta->numRows--;                           // Adjust this count downwards, because this is a row common to both the previous accumulation and the current tile.
            numNzColsInRow_t accNzColsDownCtr  = *pCRS0NumNzColsInRow;
            numNzColsInRow_t thisNzColsDownCtr = *pCRS1NumNzColsInRow;
            *pNewAccRowNum = *pCRS0RowNum;
            *pNewAccNumNzColsInRow = *pCRS0NumNzColsInRow + *pCRS1NumNzColsInRow; // Will adjust downwards for elements common to old acc and curr tile.
            while (accNzColsDownCtr > 0 && thisNzColsDownCtr > 0) {
                if (*pCRS0ColIdx == *pCRS1ColIdx) {           // Element in this tile already appears in accumulation.  Add it thereto, and the sum to the new accumulation tile.
                    pNewAccMeta->numNzElements--;             // Adjust this count downwards, because this element is common to both the previous accumulation and the current tile.
                    *pNewAccNumNzColsInRow = *pNewAccNumNzColsInRow-1; // Adjust this count downwards, because this element is common to both the previous accumulation and the current tile.
                    *pNewAccColIdx++ = *pCRS0ColIdx++;
                    pCRS1ColIdx++;
                    xADD(*pNewAccVal, *pCRS0Val, *pCRS1Val);
                    pNewAccVal++;
                    pCRS0Val++;
                    pCRS1Val++;
                    accNzColsDownCtr--;
                    thisNzColsDownCtr--;
                } else if (*pCRS0ColIdx < *pCRS1ColIdx) {     // Element in row of accumulator does not appear in this tile of C.  Move it to new accmulation.
                    *pNewAccColIdx++ = *pCRS0ColIdx++;
                    *pNewAccVal++    = *pCRS0Val++;
                    accNzColsDownCtr--;
                } else {                                      // Element in row of this tile of C isn't yet in accumlator.  Move it to new accumulation.
                    *pNewAccColIdx++ = *pCRS1ColIdx++;
                    *pNewAccVal++    = *pCRS1Val++;
                    thisNzColsDownCtr--;
                }
            }
            while (accNzColsDownCtr > 0) {                    // Move the rest of this row of the old accumulator, if any
                *pNewAccColIdx++ = *pCRS0ColIdx++;
                *pNewAccVal++    = *pCRS0Val++;
                accNzColsDownCtr--;
            }
            while (thisNzColsDownCtr > 0) {                   // Move the rest of this row of the current tile of C, if any
                *pNewAccColIdx++ = *pCRS1ColIdx++;
                *pNewAccVal++    = *pCRS1Val++;
                thisNzColsDownCtr--;
            }
            pNewAccRowNum++;
            pNewAccNumNzColsInRow++;
            pCRS0RowNum++;
            pCRS0NumNzColsInRow++;
            pCRS1RowNum++;
            pCRS1NumNzColsInRow++;
            accTileRowDescrDownCtr--;
            thisTileRowDescrDownCtr--;
        } else if (*pCRS0RowNum < *pCRS1RowNum) {             // This row of the previous accumulation has no additions from the current tile of C.  Move to new accumulation.
            numNzColsInRow_t accNzColsDownCtr;
            for (accNzColsDownCtr = *pCRS0NumNzColsInRow; accNzColsDownCtr > 0; accNzColsDownCtr--) {
                *pNewAccColIdx++ = *pCRS0ColIdx++;
                *pNewAccVal++    = *pCRS0Val++;
            }
            *pNewAccRowNum++         = *pCRS0RowNum++;
            *pNewAccNumNzColsInRow++ = *pCRS0NumNzColsInRow++;
            accTileRowDescrDownCtr--;
        } else {                                              // This row from the current tile of C does not appear in the previous accumulation.  Move to new accumulation.
            numNzColsInRow_t thisNzColsDownCtr;
            for (thisNzColsDownCtr = *pCRS1NumNzColsInRow; thisNzColsDownCtr > 0; thisNzColsDownCtr--) {
                *pNewAccColIdx++ = *pCRS1ColIdx++;
                *pNewAccVal++    = *pCRS1Val++;
            }
            *pNewAccRowNum++         = *pCRS1RowNum++;
            *pNewAccNumNzColsInRow++ = *pCRS1NumNzColsInRow++;
            thisTileRowDescrDownCtr--;
        }
    }

    while (accTileRowDescrDownCtr > 0) {                      // Move any remaining rows from the previous accumulation to the new accumulation.
        numNzColsInRow_t accNzColsDownCtr;
        for (accNzColsDownCtr = *pCRS0NumNzColsInRow; accNzColsDownCtr > 0; accNzColsDownCtr--) {
            *pNewAccColIdx++ = *pCRS0ColIdx++;
            *pNewAccVal++    = *pCRS0Val++;
        }
        *pNewAccRowNum++         = *pCRS0RowNum++;
        *pNewAccNumNzColsInRow++ = *pCRS0NumNzColsInRow++;
        accTileRowDescrDownCtr--;
    }

    while (thisTileRowDescrDownCtr > 0) {                     // Move any remaining rows from the current tile of C to the new accumulation.
        numNzColsInRow_t thisNzColsDownCtr;
        for (thisNzColsDownCtr = *pCRS1NumNzColsInRow; thisNzColsDownCtr > 0; thisNzColsDownCtr--) {
            *pNewAccColIdx++ = *pCRS1ColIdx++;
            *pNewAccVal++    = *pCRS1Val++;
        }
        *pNewAccRowNum++         = *pCRS1RowNum++;
        *pNewAccNumNzColsInRow++ = *pCRS1NumNzColsInRow++;
        thisTileRowDescrDownCtr--;
    }

    u64 storageRequirement = sizeof(CRSTileMeta_t);
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    storageRequirement += (pCRS0Meta->numRows + pCRS1Meta->numRows) * sizeof(rowNum_t);            // Size of the array of rowNum.  (rowNum contains the row number of each row that has some non-zeros).
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    storageRequirement += (pCRS0Meta->numRows + pCRS1Meta->numRows) * sizeof(numNzColsInRow_t);    // Size of the array of numNzColsInRow.  (This is the number of non-zeros in the row identified by rowNum).
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    storageRequirement += (pCRS0Meta->numNzElements + pCRS1Meta->numNzElements) * sizeof(colIdx_t); // Size of the array of colIdx.  (These are the column indices of the nonzero elements.)
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    storageRequirement += (pCRS0Meta->numNzElements + pCRS1Meta->numNzElements) * sizeof(BLAS_MATH_t); // Size of the array of val.  (These are the values of the nonzero elements.)
    storageRequirement += 7LL;
    storageRequirement &= ~(7LL);
    if (((u64) pNewAccVal) > (((u64) pNewAccMeta) + storageRequirement)) {
        printf ("OVERFLOW IN COLLATE!!! 0x%lx 0x%lx 0x%lx\n", (u64) pNewAccVal, (u64) pNewAccMeta, (u64) storageRequirement);
        ocrShutdown();
    }
#if 0
    u64 i, j;
    SpmmmIdx_t    firstColOfC      = myParams->firstColOfC;
    printf ("\n**** ACCUMULATION of C[%ld][%ld]:\n", (u64) firstRowOfStripOfC, (u64) firstColOfC);
    printf ("CRS0 ACC tile contents:    numRows = %ld    numNzElements = %ld   offsetToColIdx = %ld    offsetToVal = %ld\n",
        (u64) pCRS0Meta->numRows, (u64) pCRS0Meta->numNzElements, (u64) pCRS0Meta->offsetToColIdx, (u64) pCRS0Meta->offsetToVal);
    pCRS0RowNum           = getAddrCRSTileRowNumArray(pCRS0Meta);
    pCRS0NumNzColsInRow   = getAddrCRSTileNumNzColsInRowArray(pCRS0Meta);
    pCRS0ColIdx           = getAddrCRSTileColIdxArray(pCRS0Meta);
    pCRS0Val              = getAddrCRSTileValArray(pCRS0Meta);
    for (i = 0; i < pCRS0Meta->numRows; i++) {
        printf ("Row %ld:\n", (u64) (*pCRS0RowNum));
        for (j = 0; j < *pCRS0NumNzColsInRow; j++) {
            printf ("        Column %ld:   Value = (%f, %f)\n", (u64) (*pCRS0ColIdx), xREAL(*pCRS0Val), xIMAG(*pCRS0Val));
            pCRS0ColIdx++;
            pCRS0Val++;
        }
        pCRS0RowNum++;
        pCRS0NumNzColsInRow++;
    }
    printf ("CRS1 ACC tile contents:    numRows = %ld    numNzElements = %ld   offsetToColIdx = %ld    offsetToVal = %ld\n",
        (u64) pCRS1Meta->numRows, (u64) pCRS1Meta->numNzElements, (u64) pCRS1Meta->offsetToColIdx, (u64) pCRS1Meta->offsetToVal);
    pCRS1RowNum           = getAddrCRSTileRowNumArray(pCRS1Meta);
    pCRS1NumNzColsInRow   = getAddrCRSTileNumNzColsInRowArray(pCRS1Meta);
    pCRS1ColIdx           = getAddrCRSTileColIdxArray(pCRS1Meta);
    pCRS1Val              = getAddrCRSTileValArray(pCRS1Meta);
    for (i = 0; i < pCRS1Meta->numRows; i++) {
        printf ("Row %ld:\n", (u64) (*pCRS1RowNum));
        for (j = 0; j < *pCRS1NumNzColsInRow; j++) {
            printf ("        Column %ld:   Value = (%f, %f)\n", (u64) (*pCRS1ColIdx), xREAL(*pCRS1Val), xIMAG(*pCRS1Val));
            pCRS1ColIdx++;
            pCRS1Val++;
        }
        pCRS1RowDescr++;
        pCRS1NumNzColsInRow++;
    }
    printf ("Results:                  numRows = %ld    numNzElements = %ld   offsetToColIdx = %ld   offsetToVal = %ld\n",
        (u64) pNewAccMeta->numRows, (u64) pNewAccMeta->numNzElements, (u64) pNewAccMeta->offsetToColIdx, (u64) pNewAccMeta->offsetToVal);
    pNewAccRowNum         = getAddrCRSTileRowNumArray(pNewAccMeta);
    pNewAccNumNzColsInRow = getAddrCRSTileNumNzColsInRowArray(pNewAccMeta);
    pNewAccColIdx         = getAddrCRSTileColIdxArray(pNewAccMeta);
    pNewAccVal            = getAddrCRSTileValArray(pNewAccMeta);
    for (i = 0; i < pNewAccMeta->numRows; i++) {
        printf ("Row %ld:\n", (u64) (*pNewAccRowNum));
        for (j = 0; j < *pNewAccRowNumNzColsInRow; j++) {
            printf ("        Column %ld:   Value = (%f, %f)\n", (u64) (*pNewAccColIdx), xREAL(*pNewAccVal), xIMAG(*pNewAccVal));
            pNewAccColIdx++;
            pNewAccVal++;
        }
        pNewAccRowDescr++;
        pNewAccNumNzColsInRow++;
    }
#endif
    ocrDbDestroy(gCRS0);
    ocrDbDestroy(gCRS1);


    //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_collateMosaic_task))); fflush(stdout);
    return gNewAcc;
} // ?spmmm_collateMosaic_task

#if defined(BLAS_TYPE__single) || defined(BLAS_TYPE__double)
#define IS_BELOW_THRESHOLD(x) ((x)<epsilon && (x)>negEpsilon)
#elif defined(BLAS_TYPE__complex) || defined(BLAS_TYPE__complex_double)
#define IS_BELOW_THRESHOLD(x) ((x.real*x.real+x.imag*x.imag)<epsilon && (x.real*x.real+x.imag*x.imag)>negEpsilon)
#else
*** ERROR: BLAS_TYPE__xxx not defined.
#endif


static ocrGuid_t NAME(spmmm_collateIntoOutput_task) (   // Collate two CRS half-strip-width mosaics of C into its storage space in the ELL output construct.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {

    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_collateIntoOutput_task))); fflush(stdout);
    NAME(spmmm_doMatMulCalcs_params_t)   * myParams = (NAME(spmmm_doMatMulCalcs_params_t) *) paramv;
    NAME(spmmm_collateIntoOutput_deps_t) * myDeps   = (NAME(spmmm_collateIntoOutput_deps_t) *) depv;

    SpmmmIdx_t      m                  = myParams->m;
    SpmmmIdx_t      firstRowOfStripOfC = myParams->firstRowOfStripOfC;
    BLAS_MATH_t   * pVal               = myParams->cVal;
    SpmmmIdx_t    * pColIdx            = myParams->cColIdx;
    SpmmmIdx_t    * pNumNzColsInRow    = myParams->cNumNzColsInRow;
    SpmmmIdx_t      aNumTilesDown      = myParams->aNumTilesDown;           // Number of tiles down full-width strip of A.
    SpmmmIdx_t      ldc                = myParams->ldc;
    double          epsilon            = myParams->epsilon;
    double          negEpsilon         = myParams->epsilon;
    if (epsilon > 0.0) {
        negEpsilon = -negEpsilon;
    } else {
        epsilon = -epsilon;
    }

    ocrGuid_t       gMosaic0           = myDeps->mosaicCRSofC_0.guid;
    ocrGuid_t       gMosaic1           = myDeps->mosaicCRSofC_1.guid;
    ocrGuid_t       gStats             = myDeps->statisticsForStripOfC.guid;
    statisticsForStripOfC_t * pStats   = myDeps->statisticsForStripOfC.ptr;

    SpmmmIdx_t      cNumTilesDown      = aNumTilesDown;

    CRSTileMeta_t    * pMosaic0Meta     = NULL;
    rowNum_t         * pRowNum0         = NULL;
    numNzColsInRow_t * pNumNzColsInRow0 = NULL;
    colIdx_t         * pColIdx0         = NULL;
    BLAS_MATH_t      * pVal0            = NULL;
    colIdx_t           rowDownCtr0      = 0;
    CRSTileMeta_t    * pMosaic1Meta     = NULL;
    rowNum_t         * pRowNum1         = NULL;
    numNzColsInRow_t * pNumNzColsInRow1 = NULL;
    colIdx_t         * pColIdx1         = NULL;
    BLAS_MATH_t      * pVal1            = NULL;
    colIdx_t           rowDownCtr1      = 0;

    if (gMosaic0 != NULL_GUID) {
        pMosaic0Meta       = (CRSTileMeta_t *) myDeps->mosaicCRSofC_0.ptr;
        pRowNum0           = getAddrCRSTileRowNumArray(pMosaic0Meta);
        pNumNzColsInRow0   = getAddrCRSTileNumNzColsInRowArray(pMosaic0Meta);
        pColIdx0           = getAddrCRSTileColIdxArray(pMosaic0Meta);
        pVal0              = getAddrCRSTileValArray(pMosaic0Meta);
        rowDownCtr0        = pMosaic0Meta->numRows;
    }
    if (gMosaic1 != NULL_GUID) {
        pMosaic1Meta       = (CRSTileMeta_t *) myDeps->mosaicCRSofC_1.ptr;
        pRowNum1           = getAddrCRSTileRowNumArray(pMosaic1Meta);
        pNumNzColsInRow1   = getAddrCRSTileNumNzColsInRowArray(pMosaic1Meta);
        pColIdx1           = getAddrCRSTileColIdxArray(pMosaic1Meta);
        pVal1              = getAddrCRSTileValArray(pMosaic1Meta);
        rowDownCtr1        = pMosaic1Meta->numRows;
    }
#if 0
    u64 i, j;
    printf ("\n**** COLLATE INTO OUTPUT of C[%ld][...]:\n", (u64) firstRowOfStripOfC);
    if (gMosaic0 == NULL_GUID) {
        printf ("CRS0 = NULL_GUID\n");
    } else {
        rowNum_t           pRowNum0           = getAddrCRSTileRowNumArray(pMosaic0Meta);
        numNzColsInRow_t   pNumNzColsInRow0   = getAddrCRSTileNumNzColsInRowArray(pMosaic0Meta);
        colIdx_t           pColIdx0           = getAddrCRSTileColIdxArray(pMosaic0Meta);
        BLAS_MATH_t        pVal0              = getAddrCRSTileValArray(pMosaic0Meta);
        printf ("CRS0 ACC tile contents:    numRows = %ld    numNzElements = %ld   offsetToColIdx = %ld    offsetToVal = %ld\n",
            (u64) pMosaic0Meta->numRows, (u64) pMosaic0Meta->numNzElements, (u64) pMosaic0Meta->offsetToColIdx, (u64) pMosaic0Meta->offsetToVal);
        for (i = 0; i < pMosaic0Meta->numRows; i++) {
            printf ("Row %ld:\n", (u64) (*pCRS0RowNum));
            for (j = 0; j < *pCRS0NumNzColsInRow; j++) {
                printf ("        Column %ld:   Value = (%f, %f)\n", (u64) (*pCRS0ColIdx), xREAL(*pCRS0Val), xIMAG(*pCRS0Val));
                pCRS0ColIdx++;
                pCRS0Val++;
            }
            pCRS0RowNum++;
            pCRS0NumNzColsInRow++;
        }
    }
    if (gMosaic1 == NULL_GUID) {
        printf ("CRS1 = NULL_GUID\n");
    } else {
        rowNum_t           pRowNum1           = getAddrCRSTileRowNumArray(pMosaic1Meta);
        numNzColsInRow_t   pNumNzColsInRow1   = getAddrCRSTileNumNzColsInRowArray(pMosaic1Meta);
        colIdx_t           pColIdx1           = getAddrCRSTileColIdxArray(pMosaic1Meta);
        BLAS_MATH_t        pVal1              = getAddrCRSTileValArray(pMosaic1Meta);
        printf ("CRS1 ACC tile contents:    numRows = %ld    numNzElements = %ld   offsetToColIdx = %ld    offsetToVal = %ld\n",
            (u64) pMosaic1Meta->numRows, (u64) pMosaic1Meta->numNzElements, (u64) pMosaic1Meta->offsetToColIdx, (u64) pMosaic1Meta->offsetToVal);
        for (i = 0; i < pMosaic1Meta->numRows; i++) {
            printf ("Row %ld:\n", (u64) (*pCRS1RowNum));
            for (j = 0; j < *pCRS1NumNzColsInRow; j++) {
                printf ("        Column %ld:   Value = (%f, %f)\n", (u64) (*pCRS1ColIdx), xREAL(*pCRS1Val), xIMAG(*pCRS1Val));
                pCRS1ColIdx++;
                pCRS1Val++;
            }
            pCRS1RowNum++;
            pCRS1NumNzColsInRow++;
        }
    }
#endif

    u64 rowNum;
    for (rowNum = firstRowOfStripOfC; rowNum < m; rowNum+=cNumTilesDown) {
        pNumNzColsInRow[rowNum] = 0;
    }
    pStats->longestRowOfC       = 0; // Largest number of nonzero elements seen.
    pStats->cTotalNumNzElements = 0; // Total number of nozero elements in matrix C.  This INCLUDES the number of excess nonzero elements that might have to be dropped to fit in a row of the ELL structure.
    pStats->resultCode          = 0; // Error code, indicative of a row having more non-zero elements than can fit in the ELL structure provided by the caller.

    bool scanRowOf0 = true;
    bool scanRowOf1 = true;

    while (rowDownCtr0 > 0 && rowDownCtr1 > 0) {
        u64 numNzElemsInRow0 = *pNumNzColsInRow0;

        if (scanRowOf0) {
            u64 idxIn, idxOut;
            for (idxIn = idxOut = 0; idxIn < numNzElemsInRow0; idxIn++) {
                if (IS_BELOW_THRESHOLD(pVal0[idxIn])) {  // If value is withing +/- epsilon, throw it away (i.e. set it to zero).
                    break;
                } else {
                    idxOut++;
                }
            }
            for (idxIn++; idxIn < numNzElemsInRow0; idxIn++) {  // Once the first small value is eliminated, we have to move others up in the list.
                if (IS_BELOW_THRESHOLD(pVal0[idxIn])) {  // If value is withing +/- epsilon, throw it away (i.e. set it to zero).
                    continue;
                } else {
                    pVal0[idxOut]    = pVal0[idxIn];
                    pColIdx0[idxOut] = pColIdx0[idxIn];
                    idxOut++;
                }
            }
            if (idxOut == 0) {  // If the row had nothing but values that were within epsilon, eliminate the row altogether and move on to the next.
                pVal0    += *pNumNzColsInRow0;
                pColIdx0 += *pNumNzColsInRow0;
                pNumNzColsInRow0++;
                rowDownCtr0--;
                pRowNum0++;
                continue;       // Restart the while-loop body.
            } else {
                numNzElemsInRow0 = idxOut;
                scanRowOf0 = false;
            }
        }

        u64 numNzElemsInRow1 = *pNumNzColsInRow1;

        if (scanRowOf1) {
            u64 idxIn, idxOut;
            for (idxIn = idxOut = 0; idxIn < numNzElemsInRow1; idxIn++) {
                if (IS_BELOW_THRESHOLD(pVal1[idxIn])) {  // If value is withing +/- epsilon, throw it away (i.e. set it to zero).
                    break;
                } else {
                    idxOut++;
                }
            }
            for (idxIn++; idxIn < numNzElemsInRow1; idxIn++) {  // Once the first small value is eliminated, we have to move others up in the list.
                if (IS_BELOW_THRESHOLD(pVal1[idxIn])) {  // If value is withing +/- epsilon, throw it away (i.e. set it to zero).
                    continue;
                } else {
                    pVal1[idxOut]    = pVal1[idxIn];
                    pColIdx1[idxOut] = pColIdx1[idxIn];
                    idxOut++;
                }
            }
            if (idxOut == 0) {  // If the row had nothing but values that were within epsilon, eliminate the row altogether and move on to the next.
                pVal1    += *pNumNzColsInRow1;
                pColIdx1 += *pNumNzColsInRow1;
                pNumNzColsInRow1++;
                rowDownCtr1--;
                pRowNum1++;
                continue;       // Restart the while-loop body.
            } else {
                numNzElemsInRow1 = idxOut;
                scanRowOf1 = false;
            }
        }

        if (*pRowNum0 == *pRowNum1) {
            pStats->longestRowOfC = MAX(pStats->longestRowOfC, numNzElemsInRow0 + numNzElemsInRow1);
            pStats->cTotalNumNzElements += numNzElemsInRow0 + numNzElemsInRow1;
            if (numNzElemsInRow0 + numNzElemsInRow1 > ldc) {
                SPMMM_RETURN_CODE_t * pRet = (SPMMM_RETURN_CODE_t *) (&(pStats->resultCode));
                if (pRet->errorDetail < numNzElemsInRow0 + numNzElemsInRow1) {
                    pRet->offendingRowNum = *pRowNum0;
                    pRet->errorDetail     = numNzElemsInRow0 + numNzElemsInRow1;
                    pRet->errorType       = SPMMM_ERROR__ROW_TOO_LONG;
                }
            }

            u64 idx0 = 0;
            u64 idx1 = 0;
            while (idx0 < numNzElemsInRow0 && idx1 < numNzElemsInRow1 && pNumNzColsInRow[*pRowNum0] != ldc) {
                if (*pColIdx0 == *pColIdx1) {
                    printf ("In %s, should NOT find any columns occuring in BOTH input mosaics.\n", STRINGIFY(NAME(spmmm_collateIntoOutput_task)));
                    ocrShutdown();
                } else if (pColIdx0[idx0] < pColIdx1[idx1]) {
                    pColIdx[*pRowNum0*ldc+pNumNzColsInRow[*pRowNum0]] = pColIdx0[idx0];
                    pVal   [*pRowNum0*ldc+pNumNzColsInRow[*pRowNum0]] = pVal0[idx0];
                    pNumNzColsInRow[*pRowNum0]++;
                    idx0++;
                } else {
                    pColIdx[*pRowNum1*ldc+pNumNzColsInRow[*pRowNum1]] = pColIdx1[idx1];
                    pVal   [*pRowNum1*ldc+pNumNzColsInRow[*pRowNum1]] = pVal1[idx1];
                    pNumNzColsInRow[*pRowNum1]++;
                    idx1++;
                }
            }
            while (idx0 < numNzElemsInRow0 && pNumNzColsInRow[*pRowNum0] != ldc) {
                pColIdx[*pRowNum0*ldc+pNumNzColsInRow[*pRowNum0]] = pColIdx0[idx0];
                pVal   [*pRowNum0*ldc+pNumNzColsInRow[*pRowNum0]] = pVal0[idx0];
                pNumNzColsInRow[*pRowNum0]++;
                idx0++;
            }
            while (idx1 < numNzElemsInRow1 && pNumNzColsInRow[*pRowNum0] != ldc) {
                pColIdx[*pRowNum1*ldc+pNumNzColsInRow[*pRowNum1]] = pColIdx1[idx1];
                pVal   [*pRowNum1*ldc+pNumNzColsInRow[*pRowNum1]] = pVal1[idx1];
                pNumNzColsInRow[*pRowNum1]++;
                idx1++;
            }
            pVal0    += *pNumNzColsInRow0;
            pVal1    += *pNumNzColsInRow1;
            pColIdx0 += *pNumNzColsInRow0;
            pColIdx1 += *pNumNzColsInRow1;
            pNumNzColsInRow0++;
            pNumNzColsInRow1++;
            rowDownCtr0--;
            rowDownCtr1--;
            pRowNum0++;
            pRowNum1++;
            scanRowOf0 = true;
            scanRowOf1 = true;
        } else if (*pRowNum0 < *pRowNum1) {
            pStats->longestRowOfC = MAX(pStats->longestRowOfC, numNzElemsInRow0);
            pStats->cTotalNumNzElements += numNzElemsInRow0;
            if (numNzElemsInRow0 > ldc) {
                SPMMM_RETURN_CODE_t * pRet = (SPMMM_RETURN_CODE_t *) (&(pStats->resultCode));
                if (pRet->errorDetail < numNzElemsInRow0) {
                    pRet->offendingRowNum = *pRowNum0;
                    pRet->errorDetail     = numNzElemsInRow0;
                    pRet->errorType       = SPMMM_ERROR__ROW_TOO_LONG;
                }
                numNzElemsInRow0 = ldc;
            }
            u64 idx0;
            for (idx0 = 0; idx0 < numNzElemsInRow0; idx0++) {
                pColIdx[*pRowNum0*ldc+pNumNzColsInRow[*pRowNum0]] = pColIdx0[idx0];
                pVal   [*pRowNum0*ldc+pNumNzColsInRow[*pRowNum0]] = pVal0[idx0];
                pNumNzColsInRow[*pRowNum0]++;
            }
            pVal0    += *pNumNzColsInRow0;
            pColIdx0 += *pNumNzColsInRow0;
            pNumNzColsInRow0++;
            rowDownCtr0--;
            pRowNum0++;
            scanRowOf0 = true;
        } else {
            pStats->longestRowOfC = MAX(pStats->longestRowOfC, numNzElemsInRow1);
            pStats->cTotalNumNzElements += numNzElemsInRow1;
            if (numNzElemsInRow1 > ldc) {
                SPMMM_RETURN_CODE_t * pRet = (SPMMM_RETURN_CODE_t *) (&(pStats->resultCode));
                if (pRet->errorDetail < numNzElemsInRow1) {
                    pRet->offendingRowNum = *pRowNum1;
                    pRet->errorDetail     = numNzElemsInRow1;
                    pRet->errorType       = SPMMM_ERROR__ROW_TOO_LONG;
                }
                numNzElemsInRow1 = ldc;
            }
            u64 idx1;
            for (idx1 = 0; idx1 < numNzElemsInRow1; idx1++) {
                pColIdx[*pRowNum1*ldc+pNumNzColsInRow[*pRowNum1]] = pColIdx1[idx1];
                pVal   [*pRowNum1*ldc+pNumNzColsInRow[*pRowNum1]] = pVal1[idx1];
                pNumNzColsInRow[*pRowNum1]++;
            }
            pVal1    += *pNumNzColsInRow1;
            pColIdx1 += *pNumNzColsInRow1;
            pNumNzColsInRow1++;
            pRowNum1++;
            scanRowOf1 = true;
        }
    }

    while (rowDownCtr0-- > 0) {
        u64 numNzElemsInRow0 = *pNumNzColsInRow0;
        u64 idxIn, idxOut;
        for (idxIn = idxOut = 0; idxIn < numNzElemsInRow0; idxIn++) {
            if (IS_BELOW_THRESHOLD(pVal0[idxIn])) {         // If value is withing +/- epsilon, throw it away (i.e. set it to zero).
                break;
            } else {
                idxOut++;
            }
        }
        for (idxIn++; idxIn < numNzElemsInRow0; idxIn++) {  // Once the first small value is eliminated, we have to move others up in the list.
            if (IS_BELOW_THRESHOLD(pVal0[idxIn])) {         // If value is withing +/- epsilon, throw it away (i.e. set it to zero).
                continue;
            } else {
                pVal0[idxOut]    = pVal0[idxIn];
                pColIdx0[idxOut] = pColIdx0[idxIn];
                idxOut++;
            }
        }
        if (idxOut == 0) {  // If the row had nothing but values that were within epsilon, eliminate the row altogether and move on to the next.
            pVal0    += *pNumNzColsInRow0;
            pColIdx0 += *pNumNzColsInRow0;
            pNumNzColsInRow0++;
            pRowNum0++;
            continue;       // Restart the while-loop body.
        } else {
            numNzElemsInRow0 = idxOut;
        }

        pStats->longestRowOfC = MAX(pStats->longestRowOfC, numNzElemsInRow0);
        pStats->cTotalNumNzElements += numNzElemsInRow0;
        if (numNzElemsInRow0 > ldc) {
            SPMMM_RETURN_CODE_t * pRet = (SPMMM_RETURN_CODE_t *) (&(pStats->resultCode));
            if (pRet->errorDetail < numNzElemsInRow0) {
                pRet->offendingRowNum = *pRowNum0;
                pRet->errorDetail     = numNzElemsInRow0;
                pRet->errorType       = SPMMM_ERROR__ROW_TOO_LONG;
            }
            numNzElemsInRow0 = ldc;
        }
        u64 idx0;
        for (idx0 = 0; idx0 < numNzElemsInRow0; idx0++) {
            pColIdx[*pRowNum0*ldc+pNumNzColsInRow[*pRowNum0]] = pColIdx0[idx0];
            pVal   [*pRowNum0*ldc+pNumNzColsInRow[*pRowNum0]] = pVal0[idx0];
            pNumNzColsInRow[*pRowNum0]++;
        }
        pVal0    += *pNumNzColsInRow0;
        pColIdx0 += *pNumNzColsInRow0;
        pNumNzColsInRow0++;
        pRowNum0++;
    }

    while (rowDownCtr1-- > 0) {
        u64 numNzElemsInRow1 = *pNumNzColsInRow1;
        u64 idxIn, idxOut;
        for (idxIn = idxOut = 0; idxIn < numNzElemsInRow1; idxIn++) {
            if (IS_BELOW_THRESHOLD(pVal1[idxIn])) {         // If value is withing +/- epsilon, throw it away (i.e. set it to zero).
                break;
            } else {
                idxOut++;
            }
        }
        for (idxIn++; idxIn < numNzElemsInRow1; idxIn++) {  // Once the first small value is eliminated, we have to move others up in the list.
            if (IS_BELOW_THRESHOLD(pVal1[idxIn])) {         // If value is withing +/- epsilon, throw it away (i.e. set it to zero).
                continue;
            } else {
                pVal1[idxOut]    = pVal1[idxIn];
                pColIdx1[idxOut] = pColIdx1[idxIn];
                idxOut++;
            }
        }
        if (idxOut == 0) {  // If the row had nothing but values that were within epsilon, eliminate the row altogether and move on to the next.
            pVal1    += *pNumNzColsInRow1;
            pColIdx1 += *pNumNzColsInRow1;
            pNumNzColsInRow1++;
            pRowNum1++;
            continue;       // Restart the while-loop body.
        } else {
            numNzElemsInRow1 = idxOut;
        }

        pStats->longestRowOfC = MAX(pStats->longestRowOfC, numNzElemsInRow1);
        pStats->cTotalNumNzElements += numNzElemsInRow1;
        if (numNzElemsInRow1 > ldc) {
            SPMMM_RETURN_CODE_t * pRet = (SPMMM_RETURN_CODE_t *) (&(pStats->resultCode));
            if (pRet->errorDetail < numNzElemsInRow1) {
                pRet->offendingRowNum = *pRowNum1;
                pRet->errorDetail     = numNzElemsInRow1;
                pRet->errorType       = SPMMM_ERROR__ROW_TOO_LONG;
            }
            numNzElemsInRow1 = ldc;
        }
        u64 idx1;
        for (idx1 = 0; idx1 < numNzElemsInRow1; idx1++) {
            pColIdx[*pRowNum1*ldc+pNumNzColsInRow[*pRowNum1]] = pColIdx1[idx1];
            pVal   [*pRowNum1*ldc+pNumNzColsInRow[*pRowNum1]] = pVal1[idx1];
            pNumNzColsInRow[*pRowNum1]++;
        }
        pVal1    += *pNumNzColsInRow1;
        pColIdx1 += *pNumNzColsInRow1;
        pNumNzColsInRow1++;
        pRowNum1++;
    }
    if (gMosaic0 != NULL_GUID) ocrDbDestroy(gMosaic0);
    if (gMosaic1 != NULL_GUID) ocrDbDestroy(gMosaic1);
    ocrDbRelease(gStats);

    //printf ("        %s function exiting.\n", STRINGIFY(NAME(spmmm_collateIntoOutput_task))); fflush(stdout);
    return gStats;
} // ?spmmm_collateIntoOutput_task


static ocrGuid_t NAME(spmmm_collateStripsOfC_task) (   // Collate statistics about the strips of C, and report them out to caller (via a final summary to wrapup).
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {
    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_collateStripsOfC_task))); fflush(stdout);
    NAME(spmmm_collateStripsOfC_deps_t) * myDeps = (NAME(spmmm_collateStripsOfC_deps_t) *) depv;
    ocrGuid_t gStripOfCStats_0 = myDeps->stripOfCStats_0.guid;
    ocrGuid_t gStripOfCStats_1 = myDeps->stripOfCStats_1.guid;
    statisticsForStripOfC_t * pStripOfCStats_0 = myDeps->stripOfCStats_0.ptr;
    statisticsForStripOfC_t * pStripOfCStats_1 = myDeps->stripOfCStats_1.ptr;

    pStripOfCStats_0->longestRowOfC       = MAX(pStripOfCStats_0->longestRowOfC,        pStripOfCStats_1->longestRowOfC);
    pStripOfCStats_0->cTotalNumNzElements =     pStripOfCStats_0->cTotalNumNzElements + pStripOfCStats_1->cTotalNumNzElements;
    pStripOfCStats_0->resultCode          = MAX(pStripOfCStats_0->resultCode,           pStripOfCStats_1->resultCode);

    ocrDbRelease(gStripOfCStats_0);
    ocrDbRelease(gStripOfCStats_1);
    ocrDbDestroy(gStripOfCStats_1);
    return gStripOfCStats_0;
} // ?spmmm_collateStripsOfC_task


static ocrGuid_t NAME(spmmm_wrapup_task) (   // Return control to caller -- TODO!  FIXME!
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[]) {
    //printf ("Entered %s function.\n", STRINGIFY(NAME(spmmm_wrapup_task))); fflush(stdout);
    NAME(spmmm_doFullMatMul_params_t) * myParams = (NAME(spmmm_doFullMatMul_params_t) *) paramv;
    NAME(spmmm_wrapup_deps_t)         * myDeps   = (NAME(spmmm_wrapup_deps_t)         *) depv;
    SpmmmIdx_t    m               = myParams->m;
    SpmmmIdx_t    ldc             = myParams->ldc;
    SpmmmIdx_t  * cNumNzColsInRow = myParams->cNumNzColsInRow;
    SpmmmIdx_t  * cColIdx         = myParams->cColIdx;
    BLAS_MATH_t * cVal            = myParams->cVal;
    SpmmmIdx_t    aNumTilesAcross = myParams->aNumTilesAcross;
    SpmmmIdx_t    aNumTilesDown   = myParams->aNumTilesDown;
    SpmmmIdx_t    bNumTilesAcross = myParams->bNumTilesAcross;
    SpmmmIdx_t    bNumTilesDown   = myParams->bNumTilesDown;
    s64         * cTotalNumNzElements = myParams->cTotalNumNzElements; // Output: NZs in matrix C.  If C is used as a source (A of B) for a subsequent ?spmmm call, providing this value will improve performance.
    SpmmmIdx_t  * cMaxNumNzColsInRow  = myParams->cMaxNumNzColsInRow;  // Address at which to return the number of non-zero elements in the most populous row of matrix C.
    u64         * resultCode          = myParams->resultCode;          // Returns zero if totally successful, else cast resultCode to SPMMM_RETURN_CODE_t to decipher problem.

    ocrGuid_t                 gTileCatalogOfA = myDeps->tileCatalogOfA.guid;
    ocrGuid_t                 gTileCatalogOfB = myDeps->tileCatalogOfB.guid;
    ocrGuid_t                 gMatrixCStats   = myDeps->matrixCStats.guid;
    InputTileDescr_t        * pTileCatalogOfA = myDeps->tileCatalogOfA.ptr;
    InputTileDescr_t        * pTileCatalogOfB = myDeps->tileCatalogOfB.ptr;
    statisticsForStripOfC_t * pMatrixCStats   = myDeps->matrixCStats.ptr;

    u64 i;
    for (i = 0; i < aNumTilesAcross * aNumTilesDown; i++) {
        if (pTileCatalogOfA->tileCRS != NULL_GUID) {
//printf ("CRSofA[%ld], numRows = %ld, numNzElements = %ld\n", (u64) i, (u64) ((CRSTileMeta_t *) pTileCatalogOfA->CRSptr)->numRows,  (u64) ((CRSTileMeta_t *) pTileCatalogOfA->CRSptr)->numNzElements);
            ocrDbDestroy (pTileCatalogOfA->tileCRS);
            ocrDbDestroy (pTileCatalogOfA->tileCCS);
        }
        pTileCatalogOfA++;
    }
//printf ("\n");
    for (i = 0; i < bNumTilesAcross * bNumTilesDown; i++) {
        if (pTileCatalogOfB->tileCRS != NULL_GUID) {
//printf ("CRSofB[%ld], numRows = %ld, numNzElements = %ld\n", (u64) i, (u64) ((CRSTileMeta_t *) pTileCatalogOfB->CRSptr)->numRows,  (u64) ((CRSTileMeta_t *) pTileCatalogOfB->CRSptr)->numNzElements);
            ocrDbDestroy (pTileCatalogOfB->tileCRS);
            ocrDbDestroy (pTileCatalogOfB->tileCCS);
        }
        pTileCatalogOfB++;
    }
//printf ("\n");
    *cTotalNumNzElements = pMatrixCStats->cTotalNumNzElements; // Number of nonzeroes in matrix C.  If C is used as a source (A of B) for a subsequent ?spmmm call, providing this value will improve performance.
    *resultCode          = pMatrixCStats->resultCode;          // Error code, indicative of a row having more non-zero elements than can fit in the ELL structure provided by the caller.
    *cMaxNumNzColsInRow  = pMatrixCStats->longestRowOfC;       // Largest number of nonzero elements seen.

    ocrDbDestroy (gTileCatalogOfA);
    ocrDbDestroy (gTileCatalogOfB);
    ocrDbDestroy (gMatrixCStats);

#if 0
    u64 rowNum;
    for (rowNum = 0; rowNum < m; rowNum++) {
        if (cNumNzColsInRow[rowNum] == 0) continue;
        printf ("Row %ld\n", (u64) rowNum);
        u64 colIdxIdx;
        for (colIdxIdx = 0; colIdxIdx < cNumNzColsInRow[rowNum]; colIdxIdx++) {
            printf ("   Col:  %ld = (%f, %f)\n", cColIdx[rowNum*ldc+colIdxIdx], xREAL(cVal[rowNum*ldc+colIdxIdx]), xIMAG(cVal[rowNum*ldc+colIdxIdx]));
        }
    }
#endif

    printf ("        %s function exiting\n", STRINGIFY(NAME(spmmm_wrapup_task)));
    return NULL_GUID;
} // ?spmmm_wrapup_task
