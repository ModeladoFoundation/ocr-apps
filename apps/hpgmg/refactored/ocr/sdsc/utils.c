#include <ocr.h>
#include <math.h>
#ifndef TG_ARCH
#include <sys/time.h>
#endif
#include "utils.h"

#include "hpgmg.h"
#include "operators.h"

#ifdef TG_ARCH
float tanh_approx(float val){
  float valsq, num, den;
  valsq = val * val;
  num = val * (135135.0 + valsq * (17325.0 + valsq * (378.0 + valsq)));
  den = 135135.0 + valsq * (62370.0 + valsq * (3150.0 + valsq * 28.0));
  return num / den;
}
void ABORT(int a) {
}
#endif


double time() {
#ifndef TG_ARCH
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec+1.0e-6*tv.tv_usec;
#else
  return 0;
#endif
}

double mean(level_type * level, int id_a){

  int box;
  double sum_level =  0.0;
  for(box=0;box<level->num_boxes;box++){
    int i,j,k;
    int jStride = level->jStride;
    int kStride = level->kStride;
    int  ghosts = NUM_GHOSTS;
    int     dim = level->box_dim;
    double * __restrict__ grid_a = (double*)(((char*)level->temp[box])+ id_a) + ghosts*(1+jStride+kStride); // i.e. [0] = first non ghost zone point
    double sum_box = 0.0;
    for(k=0;k<dim;k++){
    for(j=0;j<dim;j++){
    for(i=0;i<dim;i++){
      int ijk = i + j*jStride + k*kStride;
      sum_box += grid_a[ijk];
    }}}
    sum_level+=sum_box;
  }

  double ncells_level = (double)level->dim.i*(double)level->dim.j*(double)level->dim.k;
  double mean_level = sum_level / ncells_level;
  return(mean_level);
}


void shift_vector(level_type * level, int id_c, int id_a, double shift_a){

  int box;
  for(box=0;box<level->num_boxes;box++){
    int i,j,k;
    int jStride = level->jStride;
    int kStride = level->kStride;
    int  ghosts = NUM_GHOSTS;
    int     dim = level->box_dim;
    double * __restrict__ grid_c = (double*)(((char*)level->temp[box])+ id_c) + ghosts*(1+jStride+kStride); // i.e. [0] = first non ghost zone point
    double * __restrict__ grid_a = (double*)(((char*)level->temp[box])+ id_a) + ghosts*(1+jStride+kStride); // i.e. [0] = first non ghost zone point

    for(k=0;k<dim;k++){
    for(j=0;j<dim;j++){
    for(i=0;i<dim;i++){
      int ijk = i + j*jStride + k*kStride;
      grid_c[ijk] = grid_a[ijk] + shift_a;
    }}}
  }
}

double dot(level_type * level, int id_a, int id_b){

  int box;
  double a_dot_b_level =  0.0;
  for(box=0;box<level->num_boxes;box++){
    int i,j,k;
    int jStride = level->jStride;
    int kStride = level->kStride;
    int  ghosts = NUM_GHOSTS;
    int     dim = level->box_dim;
    double * __restrict__ grid_a = (double*)(((char*)level->temp[box])+ id_a) + ghosts*(1+jStride+kStride); // i.e. [0] = first non ghost zone point
    double * __restrict__ grid_b = (double*)(((char*)level->temp[box])+ id_b) + ghosts*(1+jStride+kStride);
    double a_dot_b_box = 0.0;
    for(k=0;k<dim;k++){
    for(j=0;j<dim;j++){
    for(i=0;i<dim;i++){
      int ijk = i + j*jStride + k*kStride;
      a_dot_b_box += grid_a[ijk]*grid_b[ijk];
    }}}
    a_dot_b_level+=a_dot_b_box;
  }

  return(a_dot_b_level);
}



void get_tuple(int b, int dim_i, int dim_j, int dim_k, int *i, int *j, int* k) {
  *i = b % dim_i;
  b = b / dim_i;
  *j = b % dim_j;
  *k = b / dim_j;
}

// returns a list of 6 box ids representing neighbors of "b"
// dim_? is the number of boxes in each dimension
void get_neighbors(int b, int dim_i, int dim_j, int dim_k, int* nbrs) {

  int i,j,k;

  get_tuple(b,dim_i, dim_j, dim_k, &i, &j, &k);

  if (i == 0)
    nbrs[0] = -1;
  else
    nbrs[0] = i-1 + j*dim_i + k*dim_i*dim_j; //(i-1,j,k)
  if (i >= dim_i-1)
    nbrs[1] = -1;
  else
    nbrs[1] = i+1 + j*dim_i + k*dim_i*dim_j; //(i+1,j,k)
  if (j == 0)
    nbrs[2] = -1;
  else
    nbrs[2] = i + (j-1)*dim_i + k*dim_i*dim_j; //(i,j-1,k)
  if (j >= dim_j-1)
    nbrs[3] = -1;
  else
    nbrs[3] = i + (j+1)*dim_i + k*dim_i*dim_j; //(i,j+1,k)
  if (k == 0)
    nbrs[4] = -1;
  else
    nbrs[4] = i + j*dim_i + (k-1)*dim_i*dim_j; //(i,j,k-1)
  if (k >= dim_k-1)
    nbrs[5] = -1;
  else
    nbrs[5] = i + j*dim_i + (k+1)*dim_i*dim_j; //(i,j,k+1)
}

void get_neighbors_all(int b, int dim_i, int dim_j, int dim_k, int *nbrs) {

  int i,j,k,x,y,z, c = 0;
  get_tuple(b, dim_i, dim_j, dim_k, &i, &j, &k);
  int ii, jj, kk;
  for (x=-1;x<2;x++)
   for (y=-1;y<2;y++)
     for (z=-1;z<2;z++) {
        if ( (x == 0) && (y == 0) && (z == 0))
	  continue;
        ii=i+x; jj=j+y; kk=k+z;
	if ((ii < 0) || (jj < 0) || (kk < 0))
	  nbrs[c++] = -1;
	else if ((ii >= dim_i) || (jj >= dim_j) || (kk >= dim_k))
	   nbrs[c++] = -1;
	else
	   nbrs[c++] = ii + jj*dim_i + kk*dim_i*dim_j;
     }
}

void get_neighbor_guids_all(int b, level_type *level, ocrGuid_t* nbr_guids) {
  int i,nbrs[26];
  int dim_i,dim_j,dim_k;
  dim_i = level->boxes_in.i;
  dim_j = level->boxes_in.j;
  dim_k = level->boxes_in.k;
  get_neighbors_all(b, dim_i, dim_j, dim_k, nbrs);

  for (i=0;i<26;i++) {
    if (nbrs[i] < 0 || nbrs[i] >= level->num_boxes)
      nbr_guids[i] = level->constant_box_guid;
    else {
      nbr_guids[i] = get_box_guid(level,nbrs[i]);
    }
  }
}

void get_neighbor_guids(int b, level_type *level, ocrGuid_t* nbr_guids) {

  int i,nbrs[6];
  int dim_i,dim_j,dim_k;
  dim_i = level->boxes_in.i;
  dim_j = level->boxes_in.j;
  dim_k = level->boxes_in.k;
  get_neighbors(b, dim_i, dim_j, dim_k, nbrs);

  for (i=0;i<6;i++) {
    if (nbrs[i] < 0 || nbrs[i] >= level->num_boxes)
      nbr_guids[i] = level->constant_box_guid;
    else
      nbr_guids[i] = get_box_guid(level,nbrs[i]);
  }
}



ocrGuid_t get_box_guid(level_type *level, int box_id) {

  ocrGuid_t *box_guid = (ocrGuid_t *)(((char*)level)+ level->boxes)+box_id;
  return *box_guid;
}

ocrGuid_t get_coarse_box(level_type *fine, level_type *coarse, int fine_box_id) {

  int fi,fj,fk,fdim_i,fdim_j,fdim_k;
  int ci,cj,ck,coarse_box_id;

  fdim_i = fine->boxes_in.i;
  fdim_j = fine->boxes_in.j;
  fdim_k = fine->boxes_in.k;

  get_tuple(fine_box_id, fdim_i, fdim_j,fdim_k, &fi, &fj, &fk);

  ci = fi * coarse->boxes_in.i / fine->boxes_in.i;
  cj = fj * coarse->boxes_in.j / fine->boxes_in.j;
  ck = fk * coarse->boxes_in.k / fine->boxes_in.k;

  coarse_box_id = ci + cj * coarse->boxes_in.i + ck * coarse->boxes_in.i * coarse->boxes_in.j;

  return (get_box_guid(coarse, coarse_box_id));
}


int get_coarse_box_id(level_type *fine, level_type *coarse, int fine_box_id) {

  int fi,fj,fk,fdim_i,fdim_j,fdim_k;
  int ci,cj,ck,coarse_box_id;

  fdim_i = fine->boxes_in.i;
  fdim_j = fine->boxes_in.j;
  fdim_k = fine->boxes_in.k;

  get_tuple(fine_box_id, fdim_i, fdim_j,fdim_k, &fi, &fj, &fk);

  ci = fi * coarse->boxes_in.i / fine->boxes_in.i;
  cj = fj * coarse->boxes_in.j / fine->boxes_in.j;
  ck = fk * coarse->boxes_in.k / fine->boxes_in.k;

  coarse_box_id = ci + cj * coarse->boxes_in.i + ck * coarse->boxes_in.i * coarse->boxes_in.j;

  return coarse_box_id;
}


// returns a list of box_guids for interpolation and also the size of the list (num_boxes)
void get_fine_boxes(level_type *fine, level_type *coarse, int coarse_box_id, ocrGuid_t *fine_boxes_guids) {

  int fi,fj,fk,fdim_i,fdim_j,fdim_k, fine_box_id;
  int cdim_i,cdim_j,cdim_k,ci,cj,ck;

  cdim_i = coarse->boxes_in.i; cdim_j = coarse->boxes_in.j; cdim_k = coarse->boxes_in.k;
  fdim_i = fine->boxes_in.i; fdim_j = fine->boxes_in.j; fdim_k = fine->boxes_in.k;

  int num_boxes =   (fdim_i/cdim_i) * (fdim_j/cdim_j) * (fdim_k/cdim_k);

  get_tuple(coarse_box_id, cdim_i, cdim_j, cdim_k, &ci, &cj, &ck);

  int i,j,k, count=0;
  for (k=0;k<fdim_k/cdim_k;k++) {
    for (j=0;j<fdim_j/cdim_j;j++) {
      for (i=0;i<fdim_i/cdim_i;i++) {
        fi = (fdim_i/cdim_i) * ci + i;
	fj = (fdim_j/cdim_j) * cj + j;
	fk = (fdim_k/cdim_k) * ck + k;

	fine_box_id = fi + fj*fdim_i + fk*fdim_i*fdim_j;
	fine_boxes_guids[count++] = get_box_guid(fine, fine_box_id);
      }
    }
  }
  ASSERT(count == num_boxes);
}

void get_fine_box_ids(level_type *fine, level_type *coarse, int coarse_box_id, int *fine_box_ids) {

  int fi,fj,fk,fdim_i,fdim_j,fdim_k, fine_box_id;
  int cdim_i,cdim_j,cdim_k,ci,cj,ck;

  cdim_i = coarse->boxes_in.i; cdim_j = coarse->boxes_in.j; cdim_k = coarse->boxes_in.k;
  fdim_i = fine->boxes_in.i; fdim_j = fine->boxes_in.j; fdim_k = fine->boxes_in.k;

  int num_boxes =   (fdim_i/cdim_i) * (fdim_j/cdim_j) * (fdim_k/cdim_k);

  get_tuple(coarse_box_id, cdim_i, cdim_j, cdim_k, &ci, &cj, &ck);

  int i,j,k, count=0;
  for (k=0;k<fdim_k/cdim_k;k++) {
    for (j=0;j<fdim_j/cdim_j;j++) {
      for (i=0;i<fdim_i/cdim_i;i++) {
        fi = (fdim_i/cdim_i) * ci + i;
        fj = (fdim_j/cdim_j) * cj + j;
        fk = (fdim_k/cdim_k) * ck + k;

        fine_box_id = fi + fj*fdim_i + fk*fdim_i*fdim_j;
        fine_box_ids[count++] = fine_box_id;
      }
    }
  }
  ASSERT(count == num_boxes);
}



void residual_coarse(level_type * level, box_type *box, double *res_id, double *x_id, double *rhs_id, double a, double b) {

  int i,j,k;
  int jStride = level->jStride;
  int kStride = level->kStride;
  int  ghosts = NUM_GHOSTS;
  int     dim = level->box_dim;

#ifndef STENCIL_FUSE_BC
  apply_bcs(level,box,x_id+ghosts*(1+jStride+kStride));
#endif

  double h2inv = 1.0/(level->h*level->h);
  const double * __restrict__ x      = x_id + ghosts*(1+jStride+kStride); // i.e. [0] = first non ghost zone point
  const double * __restrict__ rhs    = rhs_id + ghosts*(1+jStride+kStride);
  const double * __restrict__ alpha  = (double*)(((char*)box)+ level->alpha) + ghosts*(1+jStride+kStride);
  const double * __restrict__ beta_i = (double*)(((char*)box)+ level->beta_i) + ghosts*(1+jStride+kStride);
  const double * __restrict__ beta_j = (double*)(((char*)box)+ level->beta_j) + ghosts*(1+jStride+kStride);
  const double * __restrict__ beta_k = (double*)(((char*)box)+ level->beta_k) + ghosts*(1+jStride+kStride);
  const double * __restrict__ valid  = (double*)(((char*)box)+ level->valid) + ghosts*(1+jStride+kStride); // cell is inside the domain
        double * __restrict__ res    =  res_id + ghosts*(1+jStride+kStride);

  for(k=0;k<dim;k++) {
    for(j=0;j<dim;j++) {
      for(i=0;i<dim;i++) {
        int ijk = i + j*jStride + k*kStride;
        double Ax = apply_op_ijk(x);
        res[ijk] = rhs[ijk]-Ax;
      }
    }
  }
}

double dot_coarse(level_type * level, double *id_a, double *id_b){
  int box;
  double a_dot_b_level =  0.0;
  for(box=0;box<level->num_boxes;box++){
    int i,j,k;
    int jStride = level->jStride;
    int kStride = level->kStride;
    int  ghosts = NUM_GHOSTS;
    int     dim = level->box_dim;
    double * __restrict__ grid_a = id_a + ghosts*(1+jStride+kStride); // i.e. [0] = first non ghost zone point
    double * __restrict__ grid_b = id_b + ghosts*(1+jStride+kStride);
    double a_dot_b_box = 0.0;
    for(k=0;k<dim;k++){
    for(j=0;j<dim;j++){
    for(i=0;i<dim;i++){
      int ijk = i + j*jStride + k*kStride;
      a_dot_b_box += grid_a[ijk]*grid_b[ijk];
    }}}
    a_dot_b_level+=a_dot_b_box;
  }
  return(a_dot_b_level);
}

void scale_vector(level_type * level, double *id_c, double scale_a, double *id_a){ // c[]=scale_a*a[]

  int box;

  for(box=0;box<level->num_boxes;box++){
    int i,j,k;
    int jStride = level->jStride;
    int kStride = level->kStride;
    int  ghosts = NUM_GHOSTS;
    int     dim = level->box_dim;
    double * __restrict__ grid_c = id_c + ghosts*(1+jStride+kStride);
    double * __restrict__ grid_a = id_a + ghosts*(1+jStride+kStride);
    for(k=0;k<dim;k++){
    for(j=0;j<dim;j++){
    for(i=0;i<dim;i++){
        int ijk = i + j*jStride + k*kStride;
        grid_c[ijk] = scale_a*grid_a[ijk];
    }}}
  }
}

double norm_coarse(level_type * level, double *component_id){ // implements the max norm

  double max_norm =  0.0;
    int i,j,k;
    int jStride = level->jStride;
    int kStride = level->kStride;
    int  ghosts = NUM_GHOSTS;
    int     dim = level->box_dim;
    double * __restrict__ grid   = component_id + ghosts*(1+jStride+kStride); // i.e. [0] = first non ghost zone point
    double box_norm = 0.0;
    for(k=0;k<dim;k++){
    for(j=0;j<dim;j++){
    for(i=0;i<dim;i++){
      int ijk = i + j*jStride + k*kStride;
      double fabs_grid_ijk = fabs(grid[ijk]);
      if(fabs_grid_ijk>box_norm){box_norm=fabs_grid_ijk;} // max norm
    }}}
    if(box_norm>max_norm){max_norm = box_norm;}

  return(max_norm);
}


void mul_vectors(level_type * level, double *id_c, double scale, double *id_a, double *id_b){ // id_c=scale*id_a*id_b

double sum =0.0;
    int i,j,k;
    int jStride = level->jStride;
    int kStride = level->kStride;
    int  ghosts = NUM_GHOSTS;
    int     dim = level->box_dim;
    double * __restrict__ grid_c = id_c + ghosts*(1+jStride+kStride);
    double * __restrict__ grid_a = id_a + ghosts*(1+jStride+kStride);
    double * __restrict__ grid_b = id_b + ghosts*(1+jStride+kStride);
    for(k=0;k<dim;k++){
    for(j=0;j<dim;j++){
    for(i=0;i<dim;i++){
        int ijk = i + j*jStride + k*kStride;
        grid_c[ijk] = scale*grid_a[ijk]*grid_b[ijk];

    }}}
}


void apply_op(level_type * level, box_type *box, double *Ax_id, double *x_id, double a, double b){  // y=Ax

#ifndef STENCIL_FUSE_BC
    apply_bcs(level,box,x_id);
#endif

    int i,j,k;
    int jStride = level->jStride;
    int kStride = level->kStride;
    int  ghosts = NUM_GHOSTS;
    int     dim = level->box_dim;
    double h2inv = 1.0/(level->h*level->h);


    const double * __restrict__ x      = x_id + ghosts*(1+jStride+kStride); // i.e. [0] = first non ghost zone point
          double * __restrict__ Ax     = Ax_id + ghosts*(1+jStride+kStride);
    const double * __restrict__ alpha  = (double*)(((char*)box)+ level->alpha) + ghosts*(1+jStride+kStride);
    const double * __restrict__ beta_i = (double*)(((char*)box)+ level->beta_i) + ghosts*(1+jStride+kStride);
    const double * __restrict__ beta_j = (double*)(((char*)box)+ level->beta_j) + ghosts*(1+jStride+kStride);
    const double * __restrict__ beta_k = (double*)(((char*)box)+ level->beta_k) + ghosts*(1+jStride+kStride);
    const double * __restrict__  valid = (double*)(((char*)box)+ level->valid) + ghosts*(1+jStride+kStride);

    for(k=0;k<dim;k++){
    for(j=0;j<dim;j++){
    for(i=0;i<dim;i++){
      int ijk = i + j*jStride + k*kStride;
      Ax[ijk] = apply_op_ijk(x);
    }}}
}

void add_vectors(level_type * level, double *id_c, double scale_a, double *id_a, double scale_b, double *id_b){ // c=scale_a*id_a + scale_b*id_b

    int i,j,k;
    int jStride = level->jStride;
    int kStride = level->kStride;
    int  ghosts = NUM_GHOSTS;
    int     dim = level->box_dim;
    double * __restrict__ grid_c = id_c + ghosts*(1+jStride+kStride);
    double * __restrict__ grid_a = id_a + ghosts*(1+jStride+kStride);
    double * __restrict__ grid_b = id_b + ghosts*(1+jStride+kStride);
    for(k=0;k<dim;k++){
    for(j=0;j<dim;j++){
    for(i=0;i<dim;i++){
        int ijk = i + j*jStride + k*kStride;
        grid_c[ijk] = scale_a*grid_a[ijk] + scale_b*grid_b[ijk];
    }}}
}


void update_boundary_all (level_type *l, box_type *b, int b_off, box_type *nb, int nb_off, int face_only) {
  int i,j,k,ni,nj,nk;
  int jStride,kStride,dim,dim_i,dim_j,dim_k;
  jStride = l->jStride; kStride = l->kStride;
  dim = l->box_dim; dim_i = l->dim.i; dim_j = l->dim.j; dim_k = l->dim.k;
  double *nx = (double*)((char*)nb+ nb_off) + NUM_GHOSTS * (1 + jStride + kStride);
  double *x = (double*)((char*)b+ b_off) + NUM_GHOSTS * (1 + jStride + kStride);
  get_tuple(b->global_box_id,l->boxes_in.i, l->boxes_in.j, l->boxes_in.k, &i, &j, &k);
  get_tuple(nb->global_box_id,l->boxes_in.i, l->boxes_in.j, l->boxes_in.k, &ni, &nj, &nk);

  int fi, fj, fk, ii, jj, kk, nii, njj, nkk;
  fi = ni - i; fj = nj - j; fk = nk - k;
  // update faces
  if ((fi && (!fj) && (!fk)) || ((!fi) && fj && (!fk)) || ((!fi) && (!fj) && fk)) {
    update_boundary(l,x,nb,nb_off,fi,fj,fk,1);
  }
  if (face_only)
    return;

  // update edges
  if (fi && fj && (!fk)) {  // (i-1, j-1, k), (i-1, j+1, k), (i+1, j-1, k), (i+1, j+1, k)
    ii = fi; jj = fj;
    nii = dim-1; njj = dim-1;
    if (fi == 1) {
      ii = dim;
      nii = 0;
    }
    if (fj == 1) {
      jj = dim;
      njj = 0;
    }
    for (kk = 0; kk < dim; kk++) {
      int ijk = ii + jj*jStride + kk*kStride;
      int nijk = nii + njj*jStride + kk*kStride;
      x[ijk] = nx[nijk];
    }
  }

  if (fi && (!fj) && fk) {  // (i-1, j, k-1), (i-1, j, k+1), (i+1, j, k-1), (i+1, j, k+1)
    ii = fi; kk = fk;
    nii = dim-1; nkk = dim-1;
    if (fi == 1) {
      ii = dim;
      nii = 0;
    }
    if (fk == 1) {
      kk = dim;
      nkk = 0;
    }
    for (jj = 0; jj < dim; jj++) {
      int ijk = ii + jj*jStride + kk*kStride;
      int nijk = nii + jj*jStride + nkk*kStride;
      x[ijk] = nx[nijk];
    }
  }

  if ((!fi) && fj && fk) {  // (i, j-1, k-1), (i, j-1, k+1), (i, j+1, k-1), (i, j+1, k+1)
    kk = fk; jj = fj;
    nkk = dim-1; njj = dim-1;
    if (fk == 1) {
      kk = dim;
      nkk = 0;
    }
    if (fj == 1) {
      jj = dim;
      njj = 0;
    }
    for (ii = 0; ii < dim; ii++) {
      int ijk = ii + jj*jStride + kk*kStride;
      int nijk = ii + njj*jStride + nkk*kStride;
      x[ijk] = nx[nijk];
    }
  }


  // update vertices

  if (fi && fj && fk) {  // (i+-1, j+-1, k+-1))
    kk = fk; jj = fj; ii = fi;
    nkk = dim-1; njj = dim-1; nii = dim-1;
    if (fk == 1) {
      kk = dim;
      nkk = 0;
    }
    if (fj == 1) {
      jj = dim;
      njj = 0;
    }
    if (fi == 1) {
      ii = dim;
      nii = 0;
    }

    int ijk = ii + jj*jStride + kk*kStride;
    int nijk = nii + njj*jStride + nkk*kStride;
    x[ijk] = nx[nijk];
  }

}


void update_boundary(level_type *l, double *x, box_type *nb, int nb_off, int i, int j, int k, int c) {

  int ii,jj,kk;
  int jStride,kStride,dim_i,dim_j,dim_k;
  jStride = l->jStride;
  kStride = l->kStride;
  dim_i = dim_j = dim_k = l->box_dim;

  double *nx = (double*)((char*)nb+ nb_off) + NUM_GHOSTS * (1 + jStride + kStride);

  if (c == 1) { // update boundary from neighbor
    if (i == -1) {
      ii = -1;
      for (jj=0;jj<dim_j;jj++)
        for (kk=0;kk<dim_k;kk++) {
          int ijk = ii + jj*jStride + kk*kStride;
          int nijk = dim_i-1 + jj*jStride + kk*kStride;
          x[ijk] = nx[nijk];
        }
    } else if (i == 1) {
      ii = dim_i;
      for (jj=0;jj<dim_j;jj++)
        for (kk=0;kk<dim_k;kk++) {
          int ijk = ii + jj*jStride + kk*kStride;
          int nijk = 0 + jj*jStride + kk*kStride;
          x[ijk] = nx[nijk];
        }
    } else if (j == -1) {
      jj = -1;
      for (ii=0;ii<dim_i;ii++)
        for (kk=0;kk<dim_k;kk++) {
          int ijk = ii + jj*jStride + kk*kStride;
          int nijk = ii + (dim_j-1)*jStride + kk*kStride;
          x[ijk] = nx[nijk];
        }
    } else if (j == 1) {
      jj = dim_j;
      for (ii=0;ii<dim_i;ii++)
        for (kk=0;kk<dim_k;kk++) {
          int ijk = ii + jj*jStride + kk*kStride;
          int nijk = ii + (0)*jStride + kk*kStride;
          x[ijk] = nx[nijk];
        }
    } else if (k == -1) {
      kk = -1;
      for (ii=0;ii<dim_i;ii++)
        for (jj=0;jj<dim_j;jj++) {
          int ijk = ii + jj*jStride + kk*kStride;
          int nijk = ii + jj*jStride + (dim_k-1)*kStride;
          x[ijk] = nx[nijk];
        }
    } else if (k == 1) {
      kk = dim_k;
      for (ii=0;ii<dim_i;ii++)
        for (jj=0;jj<dim_j;jj++) {
          int ijk = ii + jj*jStride + kk*kStride;
          int nijk = ii + jj*jStride + (0)*kStride;
          x[ijk]= nx[nijk];
        }
    }
  }

}


// ToDo: Manu: Need to use the below function (apply_bcs) instead of apply_bc in the future.
void apply_bcs(level_type *l, box_type *box, double *x) {

  int i,j,k,dim,dim_i, dim_j,dim_k;
  int jStride, kStride;
  int ii,jj,kk;

  jStride = l->jStride; kStride = l->kStride;
  i = box->low.i; j =box->low.j; k = box->low.k;
  dim = l->box_dim;
  dim_i = l->dim.i; dim_j = l->dim.j; dim_k = l->dim.k;


  int box_on_low_i  = (box->low.i     ==            0);
  int box_on_low_j  = (box->low.j     ==            0);
  int box_on_low_k  = (box->low.k     ==            0);
  int box_on_high_i = (box->low.i+dim == l->dim.i);
  int box_on_high_j = (box->low.j+dim == l->dim.j);
  int box_on_high_k = (box->low.k+dim == l->dim.k);


  int s,normal;

// if face is on a domain boundary, impose the boundary condition using the calculated normal
  s=1;
  if(box_on_low_i ){
    normal= 1+      0+      0;
    s*=-1;
  }
  if(box_on_low_i ){
    i= -1;j  =0;k  =0;
    for(j=0;j<dim;j++)
      for(k=0;k<dim;k++){
        int ijk=i+j*jStride+k*kStride;
        x[ijk]=s*x[ijk+normal];
      }
  }

  s=1;
  if(box_on_low_j ){
    normal= 0+jStride+      0;
    s*=-1;
  }
  if(box_on_low_j ){
    i=  0;j= -1;k  =0;
    for(k=0;k<dim;k++)
      for(i=0;i<dim;i++){
        int ijk=i+j*jStride+k*kStride;
        x[ijk]=s*x[ijk+normal];
      }
  }


  s=1;
  if(box_on_low_k ){
    normal= 0+      0+kStride;
    s*=-1;
  }
  if(box_on_low_k ){
   i = 0;j = 0;k= -1;
   for(j=0;j<dim;j++)
     for(i=0;i<dim;i++){
       int ijk=i+j*jStride+k*kStride;
       x[ijk]=s*x[ijk+normal];
     }
  }

  s=1;
  if(box_on_high_i){
    normal=-1+      0+      0;
    s*=-1;
  }
  if(box_on_high_i){
    i=dim;j  =0;k  =0;
    for(j=0;j<dim;j++)
      for(k=0;k<dim;k++){
        int ijk=i+j*jStride+k*kStride;
        x[ijk]=s*x[ijk+normal];
      }
  }

  s=1;
  if(box_on_high_j){
    normal= 0-jStride+      0;
    s*=-1;
  }
  if(box_on_high_j){
    i=  0;j=dim;k  =0;
    for(k=0;k<dim;k++)
      for(i=0;i<dim;i++){
        int ijk=i+j*jStride+k*kStride;
        x[ijk]=s*x[ijk+normal];
      }
  }

  s=1;
  if(box_on_high_k){
     normal= 0+      0-kStride;
     s*=-1;
  }
  if(box_on_high_k){
    i=  0;j  =0;k=dim;
    for(j=0;j<dim;j++)
      for(i=0;i<dim;i++){
        int ijk=i+j*jStride+k*kStride;
        x[ijk]=s*x[ijk+normal];
      }
  }

      // calculate a normal vector for this edge                                                                                              // if edge is on a domain boundary, impose the boundary condition using the calculated normal
      s=1;normal=0;if(box_on_low_j ){normal+=jStride;s*=-1;}if(box_on_low_k ){normal+=kStride;s*=-1;}                                         if(box_on_low_j ||box_on_low_k ){i=  0;j= -1;k= -1;for(i=0;i<dim;i++){int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_high_j){normal-=jStride;s*=-1;}if(box_on_low_k ){normal+=kStride;s*=-1;}                                         if(box_on_high_j||box_on_low_k ){i=  0;j=dim;k= -1;for(i=0;i<dim;i++){int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_low_j ){normal+=jStride;s*=-1;}if(box_on_high_k){normal-=kStride;s*=-1;}                                         if(box_on_low_j ||box_on_high_k){i=  0;j= -1;k=dim;for(i=0;i<dim;i++){int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_high_j){normal-=jStride;s*=-1;}if(box_on_high_k){normal-=kStride;s*=-1;}                                         if(box_on_high_j||box_on_high_k){i=  0;j=dim;k=dim;for(i=0;i<dim;i++){int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_low_i ){normal+=      1;s*=-1;}if(box_on_low_k ){normal+=kStride;s*=-1;}                                         if(box_on_low_i ||box_on_low_k ){i= -1;j=  0;k= -1;for(j=0;j<dim;j++){int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_high_i){normal-=      1;s*=-1;}if(box_on_low_k ){normal+=kStride;s*=-1;}                                         if(box_on_high_i||box_on_low_k ){i=dim;j=  0;k= -1;for(j=0;j<dim;j++){int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_low_i ){normal+=      1;s*=-1;}if(box_on_high_k){normal-=kStride;s*=-1;}                                         if(box_on_low_i ||box_on_high_k){i= -1;j=  0;k=dim;for(j=0;j<dim;j++){int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_high_i){normal-=      1;s*=-1;}if(box_on_high_k){normal-=kStride;s*=-1;}                                         if(box_on_high_i||box_on_high_k){i=dim;j=  0;k=dim;for(j=0;j<dim;j++){int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_low_i ){normal+=      1;s*=-1;}if(box_on_low_j ){normal+=jStride;s*=-1;}                                         if(box_on_low_i ||box_on_low_j ){i= -1;j= -1;k=  0;for(k=0;k<dim;k++){int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_high_i){normal-=      1;s*=-1;}if(box_on_low_j ){normal+=jStride;s*=-1;}                                         if(box_on_high_i||box_on_low_j ){i=dim;j= -1;k=  0;for(k=0;k<dim;k++){int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_low_i ){normal+=      1;s*=-1;}if(box_on_high_j){normal-=jStride;s*=-1;}                                         if(box_on_low_i ||box_on_high_j){i= -1;j=dim;k=  0;for(k=0;k<dim;k++){int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_high_i){normal-=      1;s*=-1;}if(box_on_high_j){normal-=jStride;s*=-1;}                                         if(box_on_high_i||box_on_high_j){i=dim;j=dim;k=  0;for(k=0;k<dim;k++){int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}

      // calculate a normal vector for this corner                                                                                            // if corner is on a domain boundary, impose the boundary condition using the calculated normal
      s=1;normal=0;if(box_on_low_i ){normal+=      1;s*=-1;}if(box_on_low_j ){normal+=jStride;s*=-1;}if(box_on_low_k ){normal+=kStride;s*=-1;}if(box_on_low_i || box_on_low_j || box_on_low_k ){i= -1;j= -1;k= -1;{int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_high_i){normal-=      1;s*=-1;}if(box_on_low_j ){normal+=jStride;s*=-1;}if(box_on_low_k ){normal+=kStride;s*=-1;}if(box_on_high_i|| box_on_low_j || box_on_low_k ){i=dim;j= -1;k= -1;{int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_low_i ){normal+=      1;s*=-1;}if(box_on_high_j){normal-=jStride;s*=-1;}if(box_on_low_k ){normal+=kStride;s*=-1;}if(box_on_low_i || box_on_high_j|| box_on_low_k ){i= -1;j=dim;k= -1;{int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_high_i){normal-=      1;s*=-1;}if(box_on_high_j){normal-=jStride;s*=-1;}if(box_on_low_k ){normal+=kStride;s*=-1;}if(box_on_high_i|| box_on_high_j|| box_on_low_k ){i=dim;j=dim;k= -1;{int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_low_i ){normal+=      1;s*=-1;}if(box_on_low_j ){normal+=jStride;s*=-1;}if(box_on_high_k){normal-=kStride;s*=-1;}if(box_on_low_i || box_on_low_j || box_on_high_k){i= -1;j= -1;k=dim;{int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_high_i){normal-=      1;s*=-1;}if(box_on_low_j ){normal+=jStride;s*=-1;}if(box_on_high_k){normal-=kStride;s*=-1;}if(box_on_high_i|| box_on_low_j || box_on_high_k){i=dim;j= -1;k=dim;{int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_low_i ){normal+=      1;s*=-1;}if(box_on_high_j){normal-=jStride;s*=-1;}if(box_on_high_k){normal-=kStride;s*=-1;}if(box_on_low_i || box_on_high_j|| box_on_high_k){i= -1;j=dim;k=dim;{int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}
      s=1;normal=0;if(box_on_high_i){normal-=      1;s*=-1;}if(box_on_high_j){normal-=jStride;s*=-1;}if(box_on_high_k){normal-=kStride;s*=-1;}if(box_on_high_i|| box_on_high_j|| box_on_high_k){i=dim;j=dim;k=dim;{int ijk=i+j*jStride+k*kStride;x[ijk]=s*x[ijk+normal];}}

}

void populate_boundary(level_type *l, double *b, box_type *nb, int nb_off, int c) {


  if (c == 0) { // left
    update_boundary(l,b,nb,nb_off,-1,0,0,1);
  } else if (c == 1) { // right
    update_boundary(l,b,nb,nb_off,1,0,0,1);
  } else if (c == 2) { // back
    update_boundary(l,b,nb,nb_off,0,-1,0,1);
  } else if (c == 3) { // front
    update_boundary(l,b,nb,nb_off,0,1,0,1);
  } else if (c == 4) { // bottom
    update_boundary(l,b,nb,nb_off,0,0,-1,1);
  } else if (c == 5) { // top
    update_boundary(l,b,nb,nb_off,0,0,1,1);
  } else {
    PRINTF("populate_boundary: error processing a neighbor - %d\n", c);
  }

}


void print_vectors_sum(level_type *level, box_type *box) {

  int ghosts = NUM_GHOSTS;
  int jStride = level->jStride;
  int kStride = level->kStride;


  const double * __restrict__ alpha  = (double*)(((char*)box)+ level->alpha) + ghosts*(1+jStride+kStride);
  const double * __restrict__ beta_i = (double*)(((char*)box)+ level->beta_i) + ghosts*(1+jStride+kStride);
  const double * __restrict__ beta_j = (double*)(((char*)box)+ level->beta_j) + ghosts*(1+jStride+kStride);
  const double * __restrict__ beta_k = (double*)(((char*)box)+ level->beta_k) + ghosts*(1+jStride+kStride);
  const double * __restrict__  valid = (double*)(((char*)box)+ level->valid) + ghosts*(1+jStride+kStride);

  const double * __restrict__ f  = (double*)(((char*)box)+ level->f) + ghosts*(1+jStride+kStride);
  const double * __restrict__ f_Av = (double*)(((char*)box)+ level->f_Av) + ghosts*(1+jStride+kStride);
  const double * __restrict__ u_true = (double*)(((char*)box)+ level->u_true) + ghosts*(1+jStride+kStride);
  const double * __restrict__ u = (double*)(((char*)box)+ level->u) + ghosts*(1+jStride+kStride);
  const double * __restrict__  Dinv = (double*)(((char*)box)+ level->Dinv) + ghosts*(1+jStride+kStride);
  const double * __restrict__  L1inv = (double*)(((char*)box)+ level->L1inv) + ghosts*(1+jStride+kStride);
  const double * __restrict__ temp = (double*)(((char*)box)+ level->vec_temp) + ghosts*(1+jStride+kStride);

  int i,j,k;
  double sum[12] = {0.0};
  int dim = level->box_dim;
  for (k = -1; k <= dim; k++) {
    for (j = -1; j <= dim; j++) {
      for (i = -1; i <= dim; i++) {
        int ijk = i + j*jStride + k*kStride;
	sum[0] += alpha[ijk]; sum[1] += beta_i[ijk]; sum[2] += beta_j[ijk]; sum[3] += beta_k[ijk];
	sum[4] += valid[ijk]; sum[5] += f[ijk]; sum[6] += f_Av[ijk]; sum[7] += u_true[ijk];
	sum[8] += u[ijk]; sum[9] += Dinv[ijk]; sum[10] += L1inv[ijk]; sum[11] += temp[ijk];
      }
    }
  }
if (box->global_box_id == 0)
PRINTF("L %f, B %d :: %15f, %15f, %15f, %15f, %15f, %15f, %15f\n",level->h,box->global_box_id, sum[5],sum[6], sum[7], sum[8], sum[9], sum[10], sum[11]);
//PRINTF("L %f, B %d :: %15f, %15f, %15f, %15f, %15f, %15f, %15f, %15f, %15f, %15f, %15f, %15f\n",level->h,box->global_box_id, sum[0], sum[1], sum[2], sum[3], sum[4], sum[5],sum[6], sum[7], sum[8], sum[9], sum[10], sum[11]);
}

void print_vectors(level_type *level) {

  int ghosts = NUM_GHOSTS;
  int jStride = level->jStride;
  int kStride = level->kStride;

  int b;
  box_type *box;
  for (b = 0; b < level->num_boxes; b++) {

    box = level->temp[b];

    const double * __restrict__ alpha  = (double*)(((char*)box)+ level->alpha) + ghosts*(1+jStride+kStride);
    const double * __restrict__ beta_i = (double*)(((char*)box)+ level->beta_i) + ghosts*(1+jStride+kStride);
    const double * __restrict__ beta_j = (double*)(((char*)box)+ level->beta_j) + ghosts*(1+jStride+kStride);
    const double * __restrict__ beta_k = (double*)(((char*)box)+ level->beta_k) + ghosts*(1+jStride+kStride);
    const double * __restrict__  valid = (double*)(((char*)box)+ level->valid) + ghosts*(1+jStride+kStride);

    const double * __restrict__ f  = (double*)(((char*)box)+ level->f) + ghosts*(1+jStride+kStride);
    const double * __restrict__ f_Av = (double*)(((char*)box)+ level->f_Av) + ghosts*(1+jStride+kStride);
    const double * __restrict__ u_true = (double*)(((char*)box)+ level->u_true) + ghosts*(1+jStride+kStride);
    const double * __restrict__ u = (double*)(((char*)box)+ level->u) + ghosts*(1+jStride+kStride);
    const double * __restrict__  Dinv = (double*)(((char*)box)+ level->Dinv) + ghosts*(1+jStride+kStride);
    const double * __restrict__  L1inv = (double*)(((char*)box)+ level->L1inv) + ghosts*(1+jStride+kStride);
    const double * __restrict__ temp = (double*)(((char*)box)+ level->vec_temp) + ghosts*(1+jStride+kStride);

    int i,j,k;
    double sum[12] = {0.0};
    int dim = level->box_dim;
    for (k = 0; k < dim; k++) {
      for (j = 0; j < dim; j++) {
        for (i = 0; i < dim; i++) {
          int ijk = i + j*jStride + k*kStride;
          sum[0] += alpha[ijk]; sum[1] += beta_i[ijk]; sum[2] += beta_j[ijk]; sum[3] += beta_k[ijk];
          sum[4] += valid[ijk]; sum[5] += f[ijk]; sum[6] += f_Av[ijk]; sum[7] += u_true[ijk];
          sum[8] += u[ijk]; sum[9] += Dinv[ijk]; sum[10] += L1inv[ijk]; sum[11] += temp[ijk];
        }
      }
    }
    if (b==0)
      PRINTF("sum0-10:: %15f, %15f, %15f, %15f, %15f, %15f, %15f, %15f, %15f, %15f, %15f, %15f\n", sum[0], sum[1], sum[2], sum[3], sum[4], sum[5],sum[6], sum[7], sum[8], sum[9], sum[10], sum[11]);
      //PRINTF("sum0-10:: %10f, %10f, %10f, %10f, %10f, %10f, %10f, %10f, %10f, %10f, %10f, %10f\n", sum[0], sum[1], sum[2], sum[3], sum[4], sum[5],sum[6], sum[7], sum[8], sum[9], sum[10], sum[11]);

  }
}

