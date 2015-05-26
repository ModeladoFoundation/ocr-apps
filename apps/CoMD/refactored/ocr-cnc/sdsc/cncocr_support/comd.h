/**********************************************\
 *  WARNING: AUTO-GENERATED FILE!             *
 *  This file WILL BE OVERWRITTEN on each     *
 *  invocation of the graph translator tool.  *
\**********************************************/

#ifndef _CNCOCR_COMD_H_
#define _CNCOCR_COMD_H_

#include "cncocr.h"
#include "comd_defs.h"

/***************************\
 ******** CNC GRAPH ********
\***************************/

typedef struct comdContext {
    struct {
        ocrGuid_t self;
        ocrGuid_t finalizedEvent;
        ocrGuid_t quiescedEvent;
        ocrGuid_t doneEvent;
        ocrGuid_t awaitTag;
    } _guids;
    struct {
        cncItemCollection_t B;
        cncItemCollection_t SF;
        cncItemCollection_t POT;
        cncItemCollection_t EAMPOT;
        cncItemCollection_t SPECIES;
        cncItemCollection_t DD;
        cncItemCollection_t LC;
        cncItemCollection_t NAtoms;
        cncItemCollection_t redc;
        cncItemCollection_t AtomInfo;
        cncItemCollection_t CMD;
        cncItemCollection_t time;
        cncItemCollection_t ATOMS;
        cncItemCollection_t GID;
        cncItemCollection_t ISP;
        cncItemCollection_t R;
        cncItemCollection_t P;
        cncItemCollection_t F;
        cncItemCollection_t U;
        cncItemCollection_t IT;
        cncItemCollection_t TBoxes;
        cncItemCollection_t Nbs;
    } _items;
    struct {
        ocrGuid_t cncFinalize;
        ocrGuid_t cleanupInitStep;
        ocrGuid_t advanceVelocityStep;
        ocrGuid_t updateBoxStep;
        ocrGuid_t updateNeighborsStep;
        ocrGuid_t generateDataforForceStep;
        ocrGuid_t generateForceTagsStep;
        ocrGuid_t forceStep;
        ocrGuid_t computeForcefromNeighborsStep;
        ocrGuid_t computeForcefromNeighborsStep1;
        ocrGuid_t reduceStep;
    } _steps;
    u32 nx, ny, nz; // #unit cells in each direction
    u32 xproc, yproc, zproc; // processors in each direction
    u32 nSteps, doeam;
    u32 printRate;
    real_t  dt, lat, temperature, initialDelta; // other parameters;
} comdCtx;

comdCtx *comd_create(void);
void comd_destroy(comdCtx *ctx);

void comd_launch(comdArgs *args, comdCtx *ctx);
void comd_await(cncTag_t i, cncTag_t iter, comdCtx *ctx);

/**********************************\
 ******** ITEM KEY STRUCTS ********
\**********************************/

typedef struct { cncTag_t i, j, k, l; } BItemKey;
typedef struct { cncTag_t i; } SFItemKey;
typedef struct { cncTag_t i; } POTItemKey;
typedef struct { cncTag_t i; } EAMPOTItemKey;
typedef struct { cncTag_t i; } SPECIESItemKey;
typedef struct { cncTag_t i; } DDItemKey;
typedef struct { cncTag_t i; } LCItemKey;
typedef struct { cncTag_t i; } NAtomsItemKey;
typedef struct { cncTag_t i, j; } redcItemKey;
typedef struct { cncTag_t i, j, k; } AtomInfoItemKey;
typedef struct { cncTag_t i; } CMDItemKey;
typedef struct { cncTag_t i; } timeItemKey;
typedef struct { cncTag_t i; } ATOMSItemKey;
typedef struct { cncTag_t i; } GIDItemKey;
typedef struct { cncTag_t i; } ISPItemKey;
typedef struct { cncTag_t i; } RItemKey;
typedef struct { cncTag_t i; } PItemKey;
typedef struct { cncTag_t i; } FItemKey;
typedef struct { cncTag_t i; } UItemKey;
typedef struct { cncTag_t i; } ITItemKey;
typedef struct { cncTag_t i; } TBoxesItemKey;
typedef struct { cncTag_t i; } NbsItemKey;

/*****************************\
 ******** ITEM CREATE ********
\*****************************/

struct box *cncItemCreateSized_B(size_t size);
static inline struct box *cncItemCreate_B(void) {
    return cncItemCreateSized_B(sizeof(struct box));
}
static inline struct box *cncItemCreateVector_B(size_t count) {
    return cncItemCreateSized_B(sizeof(struct box) * count);
}

SimFlat *cncItemCreateSized_SF(size_t size);
static inline SimFlat *cncItemCreate_SF(void) {
    return cncItemCreateSized_SF(sizeof(SimFlat));
}
static inline SimFlat *cncItemCreateVector_SF(size_t count) {
    return cncItemCreateSized_SF(sizeof(SimFlat) * count);
}

LjPotential *cncItemCreateSized_POT(size_t size);
static inline LjPotential *cncItemCreate_POT(void) {
    return cncItemCreateSized_POT(sizeof(LjPotential));
}
static inline LjPotential *cncItemCreateVector_POT(size_t count) {
    return cncItemCreateSized_POT(sizeof(LjPotential) * count);
}

struct eamPot *cncItemCreateSized_EAMPOT(size_t size);
static inline struct eamPot *cncItemCreate_EAMPOT(void) {
    return cncItemCreateSized_EAMPOT(sizeof(struct eamPot));
}
static inline struct eamPot *cncItemCreateVector_EAMPOT(size_t count) {
    return cncItemCreateSized_EAMPOT(sizeof(struct eamPot) * count);
}

SpeciesData *cncItemCreateSized_SPECIES(size_t size);
static inline SpeciesData *cncItemCreate_SPECIES(void) {
    return cncItemCreateSized_SPECIES(sizeof(SpeciesData));
}
static inline SpeciesData *cncItemCreateVector_SPECIES(size_t count) {
    return cncItemCreateSized_SPECIES(sizeof(SpeciesData) * count);
}

Domain *cncItemCreateSized_DD(size_t size);
static inline Domain *cncItemCreate_DD(void) {
    return cncItemCreateSized_DD(sizeof(Domain));
}
static inline Domain *cncItemCreateVector_DD(size_t count) {
    return cncItemCreateSized_DD(sizeof(Domain) * count);
}

LinkCell *cncItemCreateSized_LC(size_t size);
static inline LinkCell *cncItemCreate_LC(void) {
    return cncItemCreateSized_LC(sizeof(LinkCell));
}
static inline LinkCell *cncItemCreateVector_LC(size_t count) {
    return cncItemCreateSized_LC(sizeof(LinkCell) * count);
}

int *cncItemCreateSized_NAtoms(size_t size);
static inline int *cncItemCreate_NAtoms(void) {
    return cncItemCreateSized_NAtoms(sizeof(int));
}
static inline int *cncItemCreateVector_NAtoms(size_t count) {
    return cncItemCreateSized_NAtoms(sizeof(int) * count);
}

struct myReduction *cncItemCreateSized_redc(size_t size);
static inline struct myReduction *cncItemCreate_redc(void) {
    return cncItemCreateSized_redc(sizeof(struct myReduction));
}
static inline struct myReduction *cncItemCreateVector_redc(size_t count) {
    return cncItemCreateSized_redc(sizeof(struct myReduction) * count);
}

struct atomInfo *cncItemCreateSized_AtomInfo(size_t size);
static inline struct atomInfo *cncItemCreate_AtomInfo(void) {
    return cncItemCreateSized_AtomInfo(sizeof(struct atomInfo));
}
static inline struct atomInfo *cncItemCreateVector_AtomInfo(size_t count) {
    return cncItemCreateSized_AtomInfo(sizeof(struct atomInfo) * count);
}

struct cmdInfo *cncItemCreateSized_CMD(size_t size);
static inline struct cmdInfo *cncItemCreate_CMD(void) {
    return cncItemCreateSized_CMD(sizeof(struct cmdInfo));
}
static inline struct cmdInfo *cncItemCreateVector_CMD(size_t count) {
    return cncItemCreateSized_CMD(sizeof(struct cmdInfo) * count);
}

struct timeval *cncItemCreateSized_time(size_t size);
static inline struct timeval *cncItemCreate_time(void) {
    return cncItemCreateSized_time(sizeof(struct timeval));
}
static inline struct timeval *cncItemCreateVector_time(size_t count) {
    return cncItemCreateSized_time(sizeof(struct timeval) * count);
}

Atoms *cncItemCreateSized_ATOMS(size_t size);
static inline Atoms *cncItemCreate_ATOMS(void) {
    return cncItemCreateSized_ATOMS(sizeof(Atoms));
}
static inline Atoms *cncItemCreateVector_ATOMS(size_t count) {
    return cncItemCreateSized_ATOMS(sizeof(Atoms) * count);
}

int *cncItemCreateSized_GID(size_t size);
static inline int *cncItemCreate_GID(void) {
    return cncItemCreateSized_GID(sizeof(int));
}
static inline int *cncItemCreateVector_GID(size_t count) {
    return cncItemCreateSized_GID(sizeof(int) * count);
}

int *cncItemCreateSized_ISP(size_t size);
static inline int *cncItemCreate_ISP(void) {
    return cncItemCreateSized_ISP(sizeof(int));
}
static inline int *cncItemCreateVector_ISP(size_t count) {
    return cncItemCreateSized_ISP(sizeof(int) * count);
}

real3 *cncItemCreateSized_R(size_t size);
static inline real3 *cncItemCreate_R(void) {
    return cncItemCreateSized_R(sizeof(real3));
}
static inline real3 *cncItemCreateVector_R(size_t count) {
    return cncItemCreateSized_R(sizeof(real3) * count);
}

real3 *cncItemCreateSized_P(size_t size);
static inline real3 *cncItemCreate_P(void) {
    return cncItemCreateSized_P(sizeof(real3));
}
static inline real3 *cncItemCreateVector_P(size_t count) {
    return cncItemCreateSized_P(sizeof(real3) * count);
}

real3 *cncItemCreateSized_F(size_t size);
static inline real3 *cncItemCreate_F(void) {
    return cncItemCreateSized_F(sizeof(real3));
}
static inline real3 *cncItemCreateVector_F(size_t count) {
    return cncItemCreateSized_F(sizeof(real3) * count);
}

real_t *cncItemCreateSized_U(size_t size);
static inline real_t *cncItemCreate_U(void) {
    return cncItemCreateSized_U(sizeof(real_t));
}
static inline real_t *cncItemCreateVector_U(size_t count) {
    return cncItemCreateSized_U(sizeof(real_t) * count);
}

int *cncItemCreateSized_IT(size_t size);
static inline int *cncItemCreate_IT(void) {
    return cncItemCreateSized_IT(sizeof(int));
}
static inline int *cncItemCreateVector_IT(size_t count) {
    return cncItemCreateSized_IT(sizeof(int) * count);
}

int *cncItemCreateSized_TBoxes(size_t size);
static inline int *cncItemCreate_TBoxes(void) {
    return cncItemCreateSized_TBoxes(sizeof(int));
}
static inline int *cncItemCreateVector_TBoxes(size_t count) {
    return cncItemCreateSized_TBoxes(sizeof(int) * count);
}

int *cncItemCreateSized_Nbs(size_t size);
static inline int *cncItemCreate_Nbs(void) {
    return cncItemCreateSized_Nbs(sizeof(int));
}
static inline int *cncItemCreateVector_Nbs(size_t count) {
    return cncItemCreateSized_Nbs(sizeof(int) * count);
}

static inline void cncItemDestroy(void *item) {
    cncFree(item);
}

/**************************\
 ******** ITEM PUT ********
\**************************/

// B

void cncPutChecked_B(struct box *_item, cncTag_t i, cncTag_t j, cncTag_t k, cncTag_t l, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_B(struct box *_item, cncTag_t i, cncTag_t j, cncTag_t k, cncTag_t l, comdCtx *ctx) {
    cncPutChecked_B(_item, i, j, k, l, true, ctx);
}

// SF

void cncPutChecked_SF(SimFlat *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_SF(SimFlat *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_SF(_item, i, true, ctx);
}

// POT

void cncPutChecked_POT(LjPotential *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_POT(LjPotential *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_POT(_item, i, true, ctx);
}

// EAMPOT

void cncPutChecked_EAMPOT(struct eamPot *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_EAMPOT(struct eamPot *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_EAMPOT(_item, i, true, ctx);
}

// SPECIES

void cncPutChecked_SPECIES(SpeciesData *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_SPECIES(SpeciesData *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_SPECIES(_item, i, true, ctx);
}

// DD

void cncPutChecked_DD(Domain *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_DD(Domain *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_DD(_item, i, true, ctx);
}

// LC

void cncPutChecked_LC(LinkCell *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_LC(LinkCell *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_LC(_item, i, true, ctx);
}

// NAtoms

void cncPutChecked_NAtoms(int *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_NAtoms(int *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_NAtoms(_item, i, true, ctx);
}

// redc

void cncPutChecked_redc(struct myReduction *_item, cncTag_t i, cncTag_t j, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_redc(struct myReduction *_item, cncTag_t i, cncTag_t j, comdCtx *ctx) {
    cncPutChecked_redc(_item, i, j, true, ctx);
}

// AtomInfo

void cncPutChecked_AtomInfo(struct atomInfo *_item, cncTag_t i, cncTag_t j, cncTag_t k, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_AtomInfo(struct atomInfo *_item, cncTag_t i, cncTag_t j, cncTag_t k, comdCtx *ctx) {
    cncPutChecked_AtomInfo(_item, i, j, k, true, ctx);
}

// CMD

void cncPutChecked_CMD(struct cmdInfo *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_CMD(struct cmdInfo *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_CMD(_item, i, true, ctx);
}

// time

void cncPutChecked_time(struct timeval *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_time(struct timeval *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_time(_item, i, true, ctx);
}

// ATOMS

void cncPutChecked_ATOMS(Atoms *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_ATOMS(Atoms *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_ATOMS(_item, i, true, ctx);
}

// GID

void cncPutChecked_GID(int *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_GID(int *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_GID(_item, i, true, ctx);
}

// ISP

void cncPutChecked_ISP(int *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_ISP(int *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_ISP(_item, i, true, ctx);
}

// R

void cncPutChecked_R(real3 *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_R(real3 *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_R(_item, i, true, ctx);
}

// P

void cncPutChecked_P(real3 *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_P(real3 *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_P(_item, i, true, ctx);
}

// F

void cncPutChecked_F(real3 *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_F(real3 *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_F(_item, i, true, ctx);
}

// U

void cncPutChecked_U(real_t *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_U(real_t *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_U(_item, i, true, ctx);
}

// IT

void cncPutChecked_IT(int *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_IT(int *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_IT(_item, i, true, ctx);
}

// TBoxes

void cncPutChecked_TBoxes(int *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_TBoxes(int *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_TBoxes(_item, i, true, ctx);
}

// Nbs

void cncPutChecked_Nbs(int *_item, cncTag_t i, bool checkSingleAssignment, comdCtx *ctx);

static inline void cncPut_Nbs(int *_item, cncTag_t i, comdCtx *ctx) {
    cncPutChecked_Nbs(_item, i, true, ctx);
}

/************************************\
 ******** STEP PRESCRIPTIONS ********
\************************************/

void cncPrescribe_cncFinalize(cncTag_t i, cncTag_t iter, comdCtx *ctx);
void cncPrescribe_cleanupInitStep(cncTag_t i, comdCtx *ctx);
void cncPrescribe_advanceVelocityStep(cncTag_t i, cncTag_t iter, comdCtx *ctx);
void cncPrescribe_updateBoxStep(cncTag_t i, cncTag_t k, cncTag_t iter, comdCtx *ctx);
void cncPrescribe_updateNeighborsStep(cncTag_t i, cncTag_t j, cncTag_t iter, comdCtx *ctx);
void cncPrescribe_generateDataforForceStep(cncTag_t i, cncTag_t iter, comdCtx *ctx);
void cncPrescribe_generateForceTagsStep(cncTag_t iter, comdCtx *ctx);
void cncPrescribe_forceStep(cncTag_t i, cncTag_t iter, comdCtx *ctx);
void cncPrescribe_computeForcefromNeighborsStep(cncTag_t i, cncTag_t j1, cncTag_t j2, cncTag_t j3, cncTag_t j4, cncTag_t j5, cncTag_t j6, cncTag_t j7, cncTag_t j8, cncTag_t j9, cncTag_t j10, cncTag_t j11, cncTag_t j12, cncTag_t j13, cncTag_t j14, cncTag_t j15, cncTag_t j16, cncTag_t j17, cncTag_t j18, cncTag_t j19, cncTag_t j20, cncTag_t j21, cncTag_t j22, cncTag_t j23, cncTag_t j24, cncTag_t j25, cncTag_t j26, cncTag_t iter, comdCtx *ctx);
void cncPrescribe_computeForcefromNeighborsStep1(cncTag_t i, cncTag_t j1, cncTag_t j2, cncTag_t j3, cncTag_t j4, cncTag_t j5, cncTag_t j6, cncTag_t j7, cncTag_t j8, cncTag_t j9, cncTag_t j10, cncTag_t j11, cncTag_t j12, cncTag_t j13, cncTag_t j14, cncTag_t j15, cncTag_t j16, cncTag_t j17, cncTag_t j18, cncTag_t j19, cncTag_t j20, cncTag_t j21, cncTag_t j22, cncTag_t j23, cncTag_t j24, cncTag_t j25, cncTag_t j26, cncTag_t iter, comdCtx *ctx);
void cncPrescribe_reduceStep(cncTag_t i, cncTag_t iter, comdCtx *ctx);

#endif /*_CNCOCR_COMD_H_*/
