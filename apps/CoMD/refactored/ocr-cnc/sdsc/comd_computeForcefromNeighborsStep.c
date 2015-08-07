#include "comd.h"

#include "force.h"
#include "strings.h"
//#include <assert.h>

void comd_computeForcefromNeighborsStep (cncTag_t i, cncTag_t j1, cncTag_t j2, cncTag_t j3,cncTag_t j4,cncTag_t j5,cncTag_t j6,cncTag_t j7,cncTag_t j8,cncTag_t j9,cncTag_t j10,cncTag_t j11,cncTag_t j12,cncTag_t j13,cncTag_t j14,cncTag_t j15,cncTag_t j16,cncTag_t j17,cncTag_t j18,cncTag_t j19,cncTag_t j20,cncTag_t j21,cncTag_t j22,cncTag_t j23,cncTag_t j24,cncTag_t j25,cncTag_t j26, cncTag_t iter,
        BItem b1, BItem b2, BItem b3,BItem b4,BItem b5,BItem b6,BItem b7,BItem b8,BItem b9,BItem b10,BItem b11,BItem b12,BItem b13,BItem b14,BItem b15,BItem b16,BItem b17,BItem b18,BItem b19,BItem b20,BItem b21,BItem b22,BItem b23,BItem b24,BItem b25,BItem b26,BItem b27, EAMPOTItem eampot, comdCtx *ctx) {

  if (i == 0)
    printf("CnC: computeForcefromNeighborsStep %lu, %lu\n",i,iter);

    struct box *b, *bn[27];
    b = b1;
    bn[0] = b2; bn[1] = b3; bn[2] = b4; bn[3] = b5; bn[4] = b6; bn[5] = b7; bn[6] = b8; bn[7] = b9; bn[8] = b10;
    bn[9] = b11; bn[10] = b12; bn[11] = b13; bn[12] = b1; bn[13] = b14; bn[14] = b15; bn[15] = b16; bn[16] = b17; bn[17] = b18;
    bn[18] = b19; bn[19] = b20; bn[20] = b21; bn[21] = b22; bn[22] = b23; bn[23] = b24; bn[24] = b25; bn[25] = b26; bn[26] = b27;

  if (ctx->doeam) {
    struct eamPot *p = eampot;
    force_eam(i, iter,0, b, bn, p);
  } else {
    force(i, iter,0, b, bn);  // need to remove 0
  }

  KinEnergy(i, iter, b);

  if (ctx->doeam)
    cncPut_B(b1, i, 3, 0, iter, ctx);
  else
    cncPut_B(b1, i, 4, 0, iter, ctx);
}

int force(int i, int iter, int k, struct box *b, struct box *bnAll[27]) {

   real_t sigma = b->potSigma;
   real_t epsilon = b->potEpsilon;
   real_t rCut = b->potCutoff;
   real_t rCut2 = rCut*rCut;
   struct box *bn;

   // zero forces and energy
   real_t ePot = 0.0;

   int iBox = i;

   int nIBox = b->nAtoms;

   real_t s6 = sigma*sigma*sigma*sigma*sigma*sigma;

   real_t rCut6 = s6 / (rCut2*rCut2*rCut2);
   real_t eShift = POT_SHIFT * rCut6 * (rCut6 - 1.0);


   if ( nIBox == 0) {
       b->ePot = ePot;
       return 0;
   }

   int iii;
   for (iii = 0; iii < 27; iii++) {
       bn = bnAll[iii];

       ////////////////////////////////////////////////
       int ix,iy,iz,jx,jy,jz;
       getTuple1(b, b->i, &ix, &iy, &iz);
       getTuple1(bn, bn->i, &jx, &jy, &jz);
       /*   if (i == 0) {
       printf("%d: (%d, %d, %d)\n", b->i, ix,iy,iz);
       printf("%d: (%d, %d, %d)\n", bn->i, jx,jy,jz);
   }
        */
       real_t pbc[3];
       pbc[0] = pbc[1] = pbc[2] = 0.0;
       if ((ix-jx) == (b->gridSize[0]-1))
           pbc[0] = 1.0;
       else if ((ix-jx) == -(b->gridSize[0]-1))
           pbc[0] = -1.0;

       if ((iy-jy) == (b->gridSize[1]-1))
           pbc[1] = 1.0;
       else if ((iy-jy) == -(b->gridSize[1]-1))
           pbc[1] = -1.0;

       if ((iz-jz) == (b->gridSize[2]-1))
           pbc[2] = 1.0;
       else if ((iz-jz) == -(b->gridSize[2]-1))
           pbc[2] = -1.0;

       real3 shift;

       shift[0] = pbc[0] * b->globalExtent[0];
       shift[1] = pbc[1] * b->globalExtent[1];
       shift[2] = pbc[2] * b->globalExtent[2];


       ////////////////////////////////////////////////




       int jBox = bn->i;

       assert(jBox >= 0);

       int nJBox = bn->nAtoms;
       if (nJBox == 0) {
           continue;
       }

       real_t ri=0.0, rj = 0.0;
       // loop over atoms in iBox
       for (int iOff = 0; iOff < nIBox; iOff++) {
           int iId = b->atoms.gid[iOff];

           ri += b->atoms.r[iOff][0] + b->atoms.r[iOff][1] + b->atoms.r[iOff][2];

           rj = 0.0;
           // loop over atoms in jBox
           for (int jOff = 0; jOff < nJBox; jOff++) {
               real_t dr[3];
               int jId = bn->atoms.gid[jOff];

               rj += bn->atoms.r[jOff][0] + bn->atoms.r[jOff][1] + bn->atoms.r[jOff][2] +shift[0]+shift[1]+shift[2];

               if (jBox < b->nLocalBoxes && jId == iId)
                   continue; // don't double count local-local pairs.
               real_t r2 = 0.0;
               for (int m = 0; m < 3; m++) {
                   dr[m] = b->atoms.r[iOff][m] - bn->atoms.r[jOff][m]-(shift[m]); ////////////////ToDo  need to check shift!!!!
                   r2 += dr[m] * dr[m];
               }

               if (r2 > rCut2) {
                   continue;
               }

               // Important note:
               // from this point on r actually refers to 1.0/r
               r2 = 1.0 / r2;
               real_t r6 = s6 * (r2 * r2 * r2);
               real_t eLocal = r6 * (r6 - 1.0) - eShift;
               b->atoms.U[iOff] += eLocal; //0.5*eLocal;
               ePot += 0.5 * eLocal;

               // different formulation to avoid sqrt computation
               real_t fr = -4.0 * epsilon * r6 * r2 * (12.0 * r6 - 6.0);
               for (int m = 0; m < 3; m++) {
                   b->atoms.f[iOff][m] -= dr[m] * fr;
               }
           } // loop over atoms in jBox
       } // loop over atoms in iBox

   }

/*
   if (i==0)
       printf("j = %d, nj = %d, sumri = %lf, sumrj = %lf\n", jBox, nJBox, ri, rj);
*/
   ePot = ePot * 4.0 * epsilon;
   b->ePot = ePot;
   return 0;
}


int force_eam(int i, int iter, int k, struct box *b, struct box *bnAll[27], struct eamPot *pot) {

    struct box *bn;
    real_t rCut2  = pot->cutoff*pot->cutoff;

    // zero forces / energy / rho /rhoprime
    real_t etot = 0.0;
/*
    memset(b->atoms.f,  0, MAXATOMS*sizeof(real3));
    memset(b->atoms.U,  0, MAXATOMS*sizeof(real_t));
    memset(b->potDfEmbed, 0, MAXATOMS*sizeof(real_t));
    memset(b->potRhobar,  0, MAXATOMS*sizeof(real_t));
*/
    bzero(b->atoms.f,   MAXATOMS*sizeof(real3));
    bzero(b->atoms.U,   MAXATOMS*sizeof(real_t));
    bzero(b->potDfEmbed, MAXATOMS*sizeof(real_t));
    bzero(b->potRhobar,  MAXATOMS*sizeof(real_t));

    int nbrBoxes[27];
    int nIBox = b->nAtoms;
    int iBox = i;


    // loop over neighbor boxes of iBox
    int iii;
    for (iii = 0; iii < 27; iii++) {
       bn = bnAll[iii];

       ////////////////////////////////////////////////
       int ix,iy,iz,jx,jy,jz;
       getTuple1(b, b->i, &ix, &iy, &iz);
       getTuple1(bn, bn->i, &jx, &jy, &jz);
       real_t pbc[3];
       pbc[0] = pbc[1] = pbc[2] = 0.0;
       if ((ix-jx) == (b->gridSize[0]-1))
           pbc[0] = 1.0;
       else if ((ix-jx) == -(b->gridSize[0]-1))
           pbc[0] = -1.0;

       if ((iy-jy) == (b->gridSize[1]-1))
           pbc[1] = 1.0;
       else if ((iy-jy) == -(b->gridSize[1]-1))
           pbc[1] = -1.0;

       if ((iz-jz) == (b->gridSize[2]-1))
           pbc[2] = 1.0;
       else if ((iz-jz) == -(b->gridSize[2]-1))
           pbc[2] = -1.0;

       real3 shift;

       shift[0] = pbc[0] * b->globalExtent[0];
       shift[1] = pbc[1] * b->globalExtent[1];
       shift[2] = pbc[2] * b->globalExtent[2];
       ////////////////////////////////////////////////

       int jBox = bn->i;
       int nJBox = bn->nAtoms;
       // loop over atoms in iBox
       for (int iOff=0; iOff<nIBox; iOff++)
       {
          // loop over atoms in jBox
          for (int jOff=0; jOff<nJBox; jOff++)
          {
             double r2 = 0.0;
             real3 dr;
             for (int k=0; k<3; k++)
             {
                dr[k]=b->atoms.r[iOff][k]-bn->atoms.r[jOff][k] - (shift[k]);  // ToDo: shift added, verify!
                r2+=dr[k]*dr[k];
             }


             if(r2>rCut2) continue;
             if (r2 <= 0.0) continue;

             double r = sqrt(r2);

             real_t phiTmp, dPhi, rhoTmp, dRho;

             interpolateNew(&(pot->phi), r, &phiTmp, &dPhi);
             interpolateNew(&(pot->rho), r, &rhoTmp, &dRho);


             for (int k=0; k<3; k++)
             {
                b->atoms.f[iOff][k] -= dPhi*dr[k]/r;
             }

             // update energy terms
             // calculate energy contribution based on whether
             // the neighbor box is local or remote
             etot += 0.5 * phiTmp;

             b->atoms.U[iOff] += 0.5*phiTmp;
             b->potRhobar[iOff] += rhoTmp;
          } // loop over atoms in jBox
       } // loop over atoms in iBox
    } // loop over neighbor boxes


    // Compute Embedding Energy
    // loop over atoms in iBox
    real_t psum = 0.0, rsum = 0.0; // Manu:: testing
    for (int iOff=0; iOff<nIBox; iOff++)
    {
        rsum += b->atoms.f[iOff][0] + b->atoms.f[iOff][1] +b->atoms.f[iOff][2];
        psum += b->atoms.p[iOff][0] + b->atoms.p[iOff][1] +b->atoms.p[iOff][2];

       real_t fEmbed, dfEmbed;
       interpolateNew(&(pot->f), b->potRhobar[iOff], &fEmbed, &dfEmbed);
       b->potDfEmbed[iOff] = dfEmbed;
       etot += fEmbed;
       b->atoms.U[iOff] += fEmbed;
    }

    b->ePot = etot;
    return 0;
}


int KinEnergy(int i, int iter, struct box *b) {

    real_t kinE = 0.0;
    int iBox = i;
    for (int iOff=0; iOff<b->nAtoms; iOff++)
    {
       int iSpecies = b->atoms.iSpecies[iOff];
       real_t invMass = 0.5/b->species[iSpecies].mass;
       kinE += ( b->atoms.p[iOff][0] * b->atoms.p[iOff][0] +
       b->atoms.p[iOff][1] * b->atoms.p[iOff][1] +
       b->atoms.p[iOff][2] * b->atoms.p[iOff][2] )*invMass;
    }


    b->eKin = kinE;
    return 0;
}
