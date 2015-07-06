#ifndef HPGMG_H
#define HPGMG_H

#include <ocr.h>
#define WARMUP 0
#define TIMED 1

#define MG_MAXLEVELS 100
#define NUM_VECTORS 12   // Manu:: added temporary array
#define NUM_GHOSTS 1
#define BC_PERIODIC  0
#define BC_DIRICHLET 1

#define NUM_SMOOTHS      1
#define CHEBYSHEV_DEGREE 4
#define STENCIL_FUSE_BC

#define DEBUG 0

#if DEBUG
#define VERBOSEP(s) PRINTF(s)
#define VERBOSEPA(s,v) PRINTF(s,v)
#define VERBOSEPAA(s,v,w) PRINTF(s,v,w)
#else
#define VERBOSEP(s)
#define VERBOSEPA(s,v)
#define VERBOSEPAA(s,v,w)
#endif

#ifndef MG_AGGLOMERATION_START
#define MG_AGGLOMERATION_START  8 // i.e. start the distributed v-cycle when boxes are smaller than 8^3
#endif

#define FMG_INTERPOLATE 0
#define VC_INTERPOLATE 1

#define mu 0.00000001

typedef struct {
  struct {int i, j, k;} low;
  int global_box_id;
} box_type;

typedef struct {
  double h;
  int num_boxes;
  int box_dim;                          // dimension of each cubical box (not counting ghost zones)
  struct {int i, j, k;} boxes_in;       // total number of boxes in i,j,k across this level
  struct {int i, j, k;} dim;            // global dimensions at this level (NOTE: dim.i == boxes_in.i * box_dim)
  int boundary_condition;
  int alpha_is_zero;
  double dominant_eigenvalue_of_DinvA;
  int jStride, kStride, volume;
  int level;

  // offset for box related vectors
  u32 f;
  u32 f_Av;
  u32 u_true;
  u32 u;
  u32 alpha;
  u32 beta_i;
  u32 beta_j;
  u32 beta_k;
  u32 Dinv;
  u32 L1inv;
  u32 valid;
  u32 vec_temp;

  // constant box per level
  ocrGuid_t constant_box_guid;

  // offset for list of box guids at even level
  u32 boxes;

  // offset for box norms: size == num_boxes * sizeof(double)
  u32 b_norms;

  box_type **temp; // used for temporary storage
  ocrGuid_t tempGuid;

  // timing related variables
  double time_operators[5]; //smooth (0), residual (1), restriction(2), interpolation (3), all (4)
  double time_temp[5];

} level_type;

typedef struct {
  int max_levels;                  // depth of the largest v-cycle
  int current_level;               // current level
  int vcycle_level;                // depth of the current v-cycle
  int num_levels;
  ocrGuid_t levels[MG_MAXLEVELS];  // array of pointers to levels
} mg_type;

/*
 mg_type: levels[]--> level_type
 level_type: ((char*)&level_type)+level_type.boxes[]-->box_type
 box_type: ((char*)&box_type)+level_type._var_[]-->_var_

 examples:

 ocrGud_t level0 = mg.levels[0];

 ocrGuid_t box0 = ((ocrGuid_t*)(((char*)depv[0].ptr)+((level_type*)depv[0].ptr)->boxes))[0];

 double*alpha = (double*)(((char*)depv[0].ptr)+((level_type*)depv[1].ptr)->alpha);
*/

// Initialization functions
void init_all(mg_type* mg, int box_dim, int boxes_in_i, int bc, int level);
level_type* create_level(mg_type* mg, int box_dim, int boxes_in_i, int bc, int level);
box_type* create_box(level_type* l, int num_vecs, int box_dim, int num_ghosts, int box_num);
void initialize_problem(level_type* f, double a, double b);
void rebuild_operator(level_type* l, level_type *fl, double a, double b);
void mg_build(mg_type* mg, level_type* f, double a, double b, int minCoarseDim);
void initialize_valid_region(level_type * level);

void residual_coarse(level_type * level, box_type *box, double *res_id, double *x_id, double *rhs_id, double a, double b);

// MG functions and edts
ocrGuid_t do_solves(ocrGuid_t start, mg_type* mg_ptr, int num, int warmup);

ocrGuid_t init_ur(mg_type* mg_ptr, ocrGuid_t start, int flag);
ocrGuid_t time_all(mg_type* mg_ptr, ocrGuid_t start);
ocrGuid_t restrict_all(mg_type* mg_ptr, ocrGuid_t start);
ocrGuid_t solve(mg_type* mg_ptr, ocrGuid_t start1, ocrGuid_t start2);
ocrGuid_t interpolate(int ln, mg_type* mg_ptr, ocrGuid_t start, u64 type);
ocrGuid_t smooth(int ln, mg_type* mg_ptr, ocrGuid_t start);
ocrGuid_t residual(int ln, mg_type* mg_ptr, ocrGuid_t start);
ocrGuid_t vcycle(int ln, mg_type* mg_ptr, ocrGuid_t start);
ocrGuid_t scaled_residual_norm(mg_type* mg_ptr, ocrGuid_t start);
ocrGuid_t norm(mg_type* mg_ptr, ocrGuid_t start);

ocrGuid_t init_ur_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t init_ur_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t restrict_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t restrict_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
void restriction_f(level_type* l, box_type* lb, level_type* f, box_type* fb, int octant, int dim, int flag);
ocrGuid_t interpolate_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t interpolate_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
void interpolation_pc(level_type* l, box_type* lb, level_type* f, box_type* fb, int octant);
void interpolation_pl(level_type* l, box_type* lb, level_type* f, box_type* fb, int octant);
ocrGuid_t smooth_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t smooth_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t residual_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t residual_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t mulv_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t mulv_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t norm_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t norm_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t norm_final_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t exchange_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t exchange_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t solve_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t zero_vector_level_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t zero_vector_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t time_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t print_timing_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t zero_vector(int l, mg_type* mg_ptr, u64 type, ocrGuid_t start);
void restriction_f_all(level_type* l, box_type* lb, level_type* f, box_type* fb, int octant, int dim, int flag);
void restrict_generic_all(level_type* l, box_type* lb, int l_off, level_type* f, box_type* fb, int f_off, int side);
void restrict_generic(level_type* l, box_type* lb, int l_off, level_type* f, box_type* fb, int f_off, int octant, int dim, int side);
void interpolation_pc_all(level_type* l, box_type* lb, level_type* f, box_type* fb, int octant);
void interpolation_pl_all(level_type* l, box_type* lb, level_type* f, box_type* fb, int octant);


// Operator functions

#endif
