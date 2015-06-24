#ifndef UTILS_H
#define UTILS_H

#include "hpgmg.h"

double time();
void shift_vector(level_type * level, int id_c, int id_a, double shift_a);
double dot(level_type * level, int id_a, int id_b);
void get_neighbors(int b, int dim_i, int dim_j, int dim_k, int* nbrs);
void get_neighbor_guids(int b, level_type *level, ocrGuid_t* nbrs);
ocrGuid_t get_coarse_box(level_type *fine, level_type *coarse, int fine_box_id);
void get_fine_boxes(level_type *fine, level_type *coarse, int coarse_box_id, ocrGuid_t *fine_boxes_guids);
ocrGuid_t get_box_guid(level_type *level, int box_id);
void get_tuple(int b, int dim_i, int dim_j, int dim_k, int *i, int *j, int* k);
//void apply_bc(level_type *l, double *a, int i);
void apply_bcs(level_type *l, box_type *box, double *x);
void populate_boundary(level_type *l, double *b, box_type *nb, int nb_off, int i);
void update_boundary(level_type *l, double *b, box_type *nb, int nb_off, int i, int j, int k, int c);
double dot_coarse(level_type * level, double *id_a, double *id_b);
void residual_coarse(level_type * level, box_type *box, double *res_id, double *x_id, double *rhs_id, double a, double b);
double norm_coarse(level_type * level, double *component_id);
void update_boundary_all (level_type *l, box_type *b, int b_off, box_type *nb, int nb_off, int face_only);

void get_fine_box_ids(level_type *fine, level_type *coarse, int coarse_box_id, int *fine_box_ids);
void get_neighbors_all(int b, int dim_i, int dim_j, int dim_k, int *nbrs);
double mean(level_type * level, int id_a);
void mul_vectors(level_type * level, double *id_c, double scale, double *id_a, double *id_b);
void get_neighbor_guids_all(int b, level_type *level, ocrGuid_t* nbr_guids);
void scale_vector(level_type * level, double *id_c, double scale_a, double *id_a);
void apply_op(level_type * level, box_type *box, double *Ax_id, double *x_id, double a, double b);
void add_vectors(level_type * level, double *id_c, double scale_a, double *id_a, double scale_b, double *id_b);

void print_vectors_sum(level_type *level, box_type *box);

#ifdef TG_ARCH
void ABORT(int a);
float tanh_approx(float val);
#endif


#endif
