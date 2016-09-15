!-----------------------------------------------------------------------
! The code for the procedure "gammaf" was taken from
!   nekbone-2.3.4/src/speclib.f
!-----------------------------------------------------------------------
      program test_jacg

      implicit none

      REAL*8 alpha, beta
      REAL*8 x(100), v

      integer podegree, k, N
      integer firstdegree, lastdegree

      N = 100

      alpha = 0. ! This is how it is use throughout NEKbone
      beta  = 0. ! This is how it is use throughout NEKbone

      ! If any of these values changes, make also the change in nekbone_jacg.c
      firstdegree = 2
      lastdegree  = 25

      if( lastdegree >= N) then
        write(*,*) "ERROR: test_jacg: lastdegree must be smaller than 100"
        call exit(1)
      endif

      open(unit=10,file='fortran.out')

      do podegree=firstdegree,lastdegree
        x=0
        do k=1,podegree
            v = REAL(k)
            x(k) = v /(v + 1.)
        enddo

        call JACG(x, podegree, alpha,beta)

        write(10,'(1I10)') podegree
        do k=1,podegree
            write(10,'(1ES23.14)') x(k)
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
!--------------------------------------------------------------------
      SUBROUTINE JACOBF (POLY,PDER,POLYM1,PDERM1,POLYM2,PDERM2, N,ALP,BET,X)
      IMPLICIT REAL*8  (A-H,O-Z)
      APB  = ALP+BET
      POLY = 1.
      PDER = 0.
      IF (N .EQ. 0) RETURN
      POLYL = POLY
      PDERL = PDER
      POLY  = (ALP-BET+(APB+2.)*X)/2.
      PDER  = (APB+2.)/2.
      IF (N .EQ. 1) RETURN
      DO 20 K=2,N
         DK = ((K))
         A1 = 2.*DK*(DK+APB)*(2.*DK+APB-2.)
         A2 = (2.*DK+APB-1.)*(ALP**2-BET**2)
         B3 = (2.*DK+APB-2.)
         A3 = B3*(B3+1.)*(B3+2.)
         A4 = 2.*(DK+ALP-1.)*(DK+BET-1.)*(2.*DK+APB)
         POLYN  = ((A2+A3*X)*POLY-A4*POLYL)/A1
         PDERN  = ((A2+A3*X)*PDER-A4*PDERL+A3*POLY)/A1
         PSAVE  = POLYL
         PDSAVE = PDERL
         POLYL  = POLY
         POLY   = POLYN
         PDERL  = PDER
         PDER   = PDERN
 20   CONTINUE
      POLYM1 = POLYL
      PDERM1 = PDERL
      POLYM2 = PSAVE
      PDERM2 = PDSAVE
      RETURN
      END
!--------------------------------------------------------------------
!
!     Compute NP Gauss points XJAC, which are the zeros of the
!     Jacobi polynomial J(NP) with parameters ALPHA and BETA.
!     ALPHA and BETA determines the specific type of Gauss points.
!     Examples:
!     ALPHA = BETA =  0.0  ->  Legendre points
!     ALPHA = BETA = -0.5  ->  Chebyshev points
!
!--------------------------------------------------------------------
      SUBROUTINE JACG (XJAC,NP,ALPHA,BETA)

      IMPLICIT REAL*8  (A-H,O-Z)
      REAL*8  XJAC(1)
      DATA KSTOP /10/
      DATA EPS/1.0e-12/
      N   = NP-1
      one = 1.
      DTH = 4.*ATAN(one)/(2.*((N))+2.)
      DO 40 J=1,NP
         IF (J.EQ.1) THEN
            X = COS((2.*(((J))-1.)+1.)*DTH)
         ELSE
            X1 = COS((2.*(((J))-1.)+1.)*DTH)
            X2 = XLAST
            X  = (X1+X2)/2.
         ENDIF
         DO 30 K=1,KSTOP
            CALL JACOBF (P,PD,PM1,PDM1,PM2,PDM2,NP,ALPHA,BETA,X)
            RECSUM = 0.
            JM = J-1
            DO 29 I=1,JM
               RECSUM = RECSUM+1./(X-XJAC(NP-I+1))
 29         CONTINUE
            DELX = -P/(PD-RECSUM*P)
            X    = X+DELX
            IF (ABS(DELX) .LT. EPS) GOTO 31
 30      CONTINUE
 31      CONTINUE
         XJAC(NP-J+1) = X
         XLAST        = X
 40   CONTINUE
      DO 200 I=1,NP
         XMIN = 2.
         DO 100 J=I,NP
            IF (XJAC(J).LT.XMIN) THEN
               XMIN = XJAC(J)
               JMIN = J
            ENDIF
 100     CONTINUE
         IF (JMIN.NE.I) THEN
            SWAP = XJAC(I)
            XJAC(I) = XJAC(JMIN)
            XJAC(JMIN) = SWAP
         ENDIF
 200  CONTINUE
      RETURN
      END
