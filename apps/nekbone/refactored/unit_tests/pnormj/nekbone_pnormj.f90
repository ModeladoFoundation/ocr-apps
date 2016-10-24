!-----------------------------------------------------------------------
! The code for the procedure "PNORMJ" was taken from
!   nekbone-2.3.4/src/speclib.f
!-----------------------------------------------------------------------
      program test_pnormj

      implicit none

      INTERFACE
        FUNCTION PNORMJ(N,ALPHA,BETA)
            INTEGER N
            REAL*8 ALPHA, BETA
        END FUNCTION PNORMJ
      END INTERFACE

      REAL*8 alpha, beta

      integer pDOF, N
      integer firstdegree, lastdegree

      REAL*8 nj

      alpha = 0. ! This is how it is use throughout NEKbone
      beta  = 0. ! This is how it is use throughout NEKbone

      ! If any of these values changes, make also the change in nekbone_zwgjd.c
      firstdegree = 2
      lastdegree  = 25

      N = 84 ! Largest value allowed in Nekbone's speclib.f::ZWGJ
      if( lastdegree > N) then
        write(*,*) "ERROR: test_zwgjd: lastdegree must be smaller than 84"
        call exit(1)
      endif

      open(unit=10,file='z_fortran.out')

      do pDOF=firstdegree,lastdegree
        nj = PNORMJ(pDOF,alpha,beta)
        write(10,'(1I10,1ES23.14)') pDOF, nj
      enddo

      close(10)

      call exit(0)
      end

!--------------------------------------------------------------------
     REAL*8  FUNCTION GAMMAF (X)
      IMPLICIT REAL*8  (A-H,O-Z)
      REAL*8  X
      ZERO = 0.0
      HALF = 0.5
      ONE  = 1.0
      TWO  = 2.0
      FOUR = 4.0
      PI   = FOUR*ATAN(ONE)
      GAMMAF = ONE
      IF (X.EQ.-HALF) GAMMAF = -TWO*SQRT(PI)
      IF (X.EQ. HALF) GAMMAF =  SQRT(PI)
      IF (X.EQ. ONE ) GAMMAF =  ONE
      IF (X.EQ. TWO ) GAMMAF =  ONE
      IF (X.EQ. 1.5  ) GAMMAF =  SQRT(PI)/2.
      IF (X.EQ. 2.5) GAMMAF =  1.5*SQRT(PI)/2.
      IF (X.EQ. 3.5) GAMMAF =  0.5*(2.5*(1.5*SQRT(PI)))
      IF (X.EQ. 3. ) GAMMAF =  2.
      IF (X.EQ. 4. ) GAMMAF = 6.
      IF (X.EQ. 5. ) GAMMAF = 24.
      IF (X.EQ. 6. ) GAMMAF = 120.
      RETURN
      END

!--------------------------------------------------------------------
      REAL*8  FUNCTION PNORMJ (N,ALPHA,BETA)
      IMPLICIT REAL*8  (A-H,O-Z)
      REAL*8  ALPHA,BETA
      ONE   = 1.
      TWO   = 2.
      DN    = ((N))
      CONST = ALPHA+BETA+ONE
      IF (N.LE.1) THEN
         PROD   = GAMMAF(DN+ALPHA)*GAMMAF(DN+BETA)
         PROD   = PROD/(GAMMAF(DN)*GAMMAF(DN+ALPHA+BETA))
         PNORMJ = PROD * TWO**CONST/(TWO*DN+CONST)
         RETURN
      ENDIF
      PROD  = GAMMAF(ALPHA+ONE)*GAMMAF(BETA+ONE)
      PROD  = PROD/(TWO*(ONE+CONST)*GAMMAF(CONST+ONE))
      PROD  = PROD*(ONE+ALPHA)*(TWO+ALPHA)
      PROD  = PROD*(ONE+BETA)*(TWO+BETA)
      DO 100 I=3,N
         DINDX = ((I))
         FRAC  = (DINDX+ALPHA)*(DINDX+BETA)/(DINDX*(DINDX+ALPHA+BETA))
         PROD  = PROD*FRAC
 100  CONTINUE
      PNORMJ = PROD * TWO**CONST/(TWO*DN+CONST)
      RETURN
      END
