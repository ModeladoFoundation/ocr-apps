#include <ocr.h>

typedef struct
{
    char c;
    u64 na;
    u32 nonzer;
    u32 niter;
    u8 on;
    u32 blk;
    double shift;
    double zvv;
} classdb_t;

typedef struct
{
    u8 tp;
    double t[0];
} timerdb_t;

typedef struct
{
    double tran;
    u64 amult;
} randdb_t;

enum {class_T='T', class_S='S', class_W='W', class_A='A', class_B='B',
      class_C='C', class_D='D', class_E='E', class_U='U'};

void class_init(classdb_t** class, ocrGuid_t* guid, char c, u32 blocking);

void timer_init(timerdb_t** timerdb, ocrGuid_t* guid, classdb_t* class, u8 on);

void timer_start(timerdb_t* timerdb);

void timer_stop(timerdb_t* timerdb, u8 tr);

double timer_read(timerdb_t* timerdb, u8 tr);

void rand_init(randdb_t** randdb, ocrGuid_t* guid);

double randd(randdb_t* randdb);

int randi(randdb_t* randdb, int i);

void print_results(classdb_t* class, double t, double mops);

void conj_grad(u64 n, u64 blk, ocrGuid_t a, ocrGuid_t x, ocrGuid_t cont, u32 dep);

int makea(classdb_t* class, ocrGuid_t* a);

ocrGuid_t square(u64 n, ocrGuid_t* o);

void alphas(u64 n, ocrGuid_t r, ocrGuid_t p, ocrGuid_t q, ocrGuid_t* al, ocrGuid_t* nal);

void alpha(u64 n, ocrGuid_t r, ocrGuid_t p, ocrGuid_t q, ocrGuid_t* al);

ocrGuid_t fspMv(u64 n, u64 blk, ocrGuid_t p, ocrGuid_t* q);

void spMv(u64 n, u64 blk, ocrGuid_t a, ocrGuid_t p, ocrGuid_t* q);

void scale(u64 n, ocrGuid_t a, ocrGuid_t x, ocrGuid_t* z);

void daxpy(u64 n, ocrGuid_t z, ocrGuid_t a, ocrGuid_t p, ocrGuid_t* zp);

void daxpyl(u64 n, ocrGuid_t z, ocrGuid_t a, ocrGuid_t p, ocrGuid_t* zp);

void distance(u64 n, ocrGuid_t r, ocrGuid_t x, ocrGuid_t rr, ocrGuid_t pp, ocrGuid_t* d);

void update(u64 n, ocrGuid_t nal, ocrGuid_t p, ocrGuid_t q, ocrGuid_t r, ocrGuid_t rr, ocrGuid_t pp, ocrGuid_t rho, ocrGuid_t* po, ocrGuid_t* ppo);

// depv: n x
ocrGuid_t square_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

// depv: n rho p q -> nalpha
ocrGuid_t alphas_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

// depv: n rho p q
ocrGuid_t alpha_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

// depv: n a x
ocrGuid_t scale_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

// depv: n y a x
ocrGuid_t daxpy_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

// depv: n na p q r rho
ocrGuid_t update_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

// depv: x x1 x2 ... xn out
ocrGuid_t assign_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

// depv: k rowstr colidx A x
ocrGuid_t rowvec_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

// depv: n y rowstr colidx A x out
ocrGuid_t spmv_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

// depv: n r x
ocrGuid_t dist_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
