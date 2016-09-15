!-----------------------------------------------------------------------
! The code for the procedure "jacobf" was taken from
!   nekbone-2.3.4/src/speclib.f
!-----------------------------------------------------------------------
      program test_jacobf

      implicit none

      REAL*8    POLY,PDER, POLYM1,PDERM1, POLYM2,PDERM2

      REAL*8 alpha, beta
      REAL*8 x,sz

      integer podegree
      integer firstdegree, lastdegree
      integer sampleCount,s

      alpha = 0. ! This is how it is use throughout NEKbone
      beta  = 0. ! This is how it is use throughout NEKbone

      ! If any of these values changes, make also the change in nekbone_jacobf.c
      firstdegree = 2
      lastdegree  = 16
      sampleCount = 10

      open(unit=10,file='fortran.out')

      sz = ((sampleCount))
      do podegree=firstdegree,lastdegree,1
        do s = 1,sampleCount,1
            x = ((1))
            x = x /(sz + 1.)
            call JACOBF(POLY,PDER, POLYM1,PDERM1,POLYM2,PDERM2, podegree, alpha,beta,x)

            write(10,'(2I10,6ES23.14)') podegree, s, POLY,PDER, POLYM1,PDERM1, POLYM2,PDERM2
        enddo
      enddo

      close(10)

      call exit(0)
      end
!--------------------------------------------------------------------
!
!     Computes the Jacobi polynomial (POLY) and its derivative (PDER)
!     of degree N at X.
!
!       ALP must be > -1, is 0 in ZWGL
!       BET must be > -1, is 0 in ZWGL
!
!     -1 < X < 1
!--------------------------------------------------------------------
      SUBROUTINE JACOBF (oPOLY,oPDER,oPOLYM1,oPDERM1,oPOLYM2,oPDERM2,N,ALP,BET,X)

      IMPLICIT REAL*8  (A-H,O-Z)
      APB  = ALP+BET
      oPOLY = 1.
      oPDER = 0.
      IF (N .EQ. 0) RETURN
      POLYL = oPOLY
      PDERL = oPDER
      oPOLY  = (ALP-BET+(APB+2.)*X)/2.
      oPDER  = (APB+2.)/2.
      IF (N .EQ. 1) RETURN
      DO 20 K=2,N
         DK = ((K))
         A1 = 2.*DK*(DK+APB)*(2.*DK+APB-2.)
         A2 = (2.*DK+APB-1.)*(ALP**2-BET**2)
         B3 = (2.*DK+APB-2.)
         A3 = B3*(B3+1.)*(B3+2.)
         A4 = 2.*(DK+ALP-1.)*(DK+BET-1.)*(2.*DK+APB)
         POLYN  = ((A2+A3*X)*oPOLY-A4*POLYL)/A1
         PDERN  = ((A2+A3*X)*oPDER-A4*PDERL+A3*oPOLY)/A1
         PSAVE  = POLYL
         PDSAVE = PDERL
         POLYL  = oPOLY
         oPOLY   = POLYN
         PDERL  = oPDER
         oPDER   = PDERN
 20   CONTINUE
      oPOLYM1 = POLYL
      oPDERM1 = PDERL
      oPOLYM2 = PSAVE
      oPDERM2 = PDSAVE
      RETURN
      END
