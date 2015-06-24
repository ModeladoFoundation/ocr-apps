#include <ocr.h>

#include "hpgmg.h"
#include "utils.h"
#include <math.h>
#include <string.h>

#ifdef TG_ARCH
#include "strings.h"
#endif


#define KRYLOV_DIAGONAL_PRECONDITION

// deps: leveln - coarsest box
ocrGuid_t solve_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  VERBOSEP("solve_edt\n");


  level_type* level = (level_type*) depv[0].ptr;
  box_type* box = (box_type*) depv[1].ptr;

  // Manu:: these are constants, at least looks like that
  double a=0.0, b=1.0, desired_reduction_in_norm=1e-03;

  ocrGuid_t temp_guid;
  double *temp;
  int volume = level->volume;
  int temp_size = volume * 7*sizeof(double);

  ocrDbCreate(&temp_guid, (void**)&temp, temp_size, 0,NULL_GUID,NO_ALLOC);

  bzero(temp,temp_size);

  double *r0_id, *r_id, *p_id, *s_id, *Ap_id, *As_id,*vec_temp;
  r0_id = temp; r_id = temp + volume; p_id = temp + 2*volume; s_id = temp + 3*volume;
  Ap_id = temp + 4*volume; As_id = temp + 5*volume; vec_temp = temp + 6*volume;

  double *R_id = (double*)((char*)box+ level->f_Av);
  double *x_id = (double*)((char*)box+ level->u);
  double *vec_Dinv = (double*)((char*)box+ level->Dinv);

  int jMax=200;
  int j=0;
  int BiCGStabFailed    = 0;
  int BiCGStabConverged = 0;

  residual_coarse(level,box, r0_id,x_id,R_id,a,b); // r0[] = R_id[] - A(x_id)
  scale_vector(level,r_id,1.0,r0_id);              // r[] = r0[]
  scale_vector(level,p_id,1.0,r0_id);              // p[] = r0[]

  double r_dot_r0 = dot_coarse(level,r_id,r0_id);  // r_dot_r0 = dot(r,r0)
  double norm_of_r0 = norm_coarse(level,r_id);     // the norm of the initial residual...


  if(r_dot_r0   == 0.0){
    BiCGStabConverged = 1; // entered BiCGStab with exact solution
  }
  if(norm_of_r0 == 0.0){
    BiCGStabConverged =1 ;
  }

  while( (j<jMax) && (!BiCGStabFailed) && (!BiCGStabConverged) ){
    j++;
#ifdef KRYLOV_DIAGONAL_PRECONDITION
    mul_vectors(level,vec_temp,1.0,vec_Dinv,p_id); //   temp[] = Dinv[]*p[]
    apply_op(level,box,Ap_id,vec_temp,a,b);            //   Ap = AD^{-1}(p)
#else
    apply_op(level,box,Ap_id,p_id,a,b);                //   Ap = A(p)
#endif
    double Ap_dot_r0 = dot_coarse(level,Ap_id,r0_id);     //   Ap_dot_r0 = dot(Ap,r0)
    if(Ap_dot_r0 == 0.0){
      BiCGStabFailed=1; //   pivot breakdown ???
      break;
    }
    double alpha = r_dot_r0 / Ap_dot_r0;           //   alpha = r_dot_r0 / Ap_dot_r0
//    if(isinf(alpha)){
#ifndef TG_ARCH
    if(isinf(alpha)){
#else
    if(isInf(alpha)) {
#endif
      BiCGStabFailed=2;
      break;
    }

    add_vectors(level,x_id,1.0,x_id, alpha, p_id); //   x_id[] = x_id[] + alpha*p[]
    add_vectors(level,s_id,1.0,r_id,-alpha,Ap_id); //   s[]    = r[]    - alpha*Ap[]   (intermediate residual?)

    double norm_of_s = norm_coarse(level,s_id);    //   FIX - redundant??  norm of intermediate residual
    if(norm_of_s == 0.0){
      BiCGStabConverged=1;
      break;
    }
    if(norm_of_s < desired_reduction_in_norm*norm_of_r0){
      BiCGStabConverged=1;
      break;
    }
#ifdef KRYLOV_DIAGONAL_PRECONDITION
    mul_vectors(level,vec_temp,1.0,vec_Dinv,s_id);  //   temp[] = Dinv[]*s[]
    apply_op(level,box,As_id,vec_temp,a,b);     //   As = AD^{-1}(s)
#else
    apply_op(level,box,As_id,s_id,a,b);   //   As = A(s)
#endif

    double As_dot_As = dot_coarse(level,As_id,As_id);  //   As_dot_As = dot(As,As)
    double As_dot_s  = dot_coarse(level,As_id, s_id);  //   As_dot_s  = dot(As, s)
    if(As_dot_As == 0.0){
      BiCGStabConverged=1;
      break;
    }
    double omega = As_dot_s / As_dot_As; //   omega = As_dot_s / As_dot_As
    if(omega == 0.0){
      BiCGStabFailed=3;   //   stabilization breakdown ???
      break;
    }

//    if(isinf(omega)){
#ifndef TG_ARCH
    if(isinf(omega)){
#else
    if(isInf(omega)) {
#endif
      BiCGStabFailed=4;   //   stabilization breakdown ???
      break;
    }

    add_vectors(level,x_id,1.0,x_id, omega, s_id);  //   x_id[] = x_id[] + omega*s[]
    add_vectors(level,r_id,1.0,s_id,-omega,As_id);  //   r[]    = s[]    - omega*As[]  (recursively computed / updated residual)

    double norm_of_r = norm_coarse(level,r_id);     //   norm of recursively computed residual (good enough??)
    if(norm_of_r == 0.0){
      BiCGStabConverged=1;
      break;
    }
    if(norm_of_r < desired_reduction_in_norm*norm_of_r0){
      BiCGStabConverged=1;
      break;
    }
#if DEBUG
    residual_coarse(level,box,vec_temp,x_id,R_id,a,b);
    double norm_of_residual = norm_coarse(level,vec_temp);
    PRINTF("j=%8d, norm=%12.6e, norm_inital=%12.6e, reduction=%e\n",j,norm_of_residual,norm_of_r0,norm_of_residual/norm_of_r0);
#endif
    double r_dot_r0_new = dot_coarse(level,r_id,r0_id);     //   r_dot_r0_new = dot(r,r0)
    if(r_dot_r0_new == 0.0){
      BiCGStabFailed=5;   //   Lanczos breakdown ???
      break;
    }

    double beta = (r_dot_r0_new/r_dot_r0) * (alpha/omega);  //   beta = (r_dot_r0_new/r_dot_r0) * (alpha/omega)
//    if(isinf(beta)){
#ifndef TG_ARCH
    if(isinf(beta)){
#else
    if(isInf(beta)) {
#endif
      BiCGStabFailed=6;
      break;
    }

    add_vectors(level,vec_temp,1.0,p_id,-omega, Ap_id);  //   VECTOR_TEMP =         (p[]-omega*Ap[])
    add_vectors(level, p_id,1.0,r_id,  beta, vec_temp);  //   p[] = r[] + beta*(p[]-omega*Ap[])
    r_dot_r0 = r_dot_r0_new;                             //   r_dot_r0 = r_dot_r0_new   (save old r_dot_r0)
  }
#ifdef KRYLOV_DIAGONAL_PRECONDITION
    mul_vectors(level,x_id,1.0,vec_Dinv,x_id); //   x_id[] = Dinv[]*x_id[] // i.e. x = D^{-1}x'
#endif

#if DEBUG
  if(BiCGStabFailed)PRINTF("BiCGStab Failed... error = %d\n",BiCGStabFailed);
#endif

  ocrDbDestroy(temp_guid);
  return NULL_GUID;
}
