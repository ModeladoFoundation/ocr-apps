!-----------------------------------------------------------------------
! The code for the procedure "gammaf" was taken from
!   nekbone-2.3.4/src/speclib.f
!-----------------------------------------------------------------------
      program test_gammaf

      implicit none

      INTERFACE
        FUNCTION GAMMAF (X)
            REAL*8 Area_Circle
            REAL*8 X
        END FUNCTION GAMMAF
      END INTERFACE

      REAL*8 X,Y
      REAL*8 samples(15)

      integer i, N

      open(unit=10,file='fortran.out')

      N = 15
      samples( 1) =  0.0
      samples( 2) = -0.5
      samples( 3) =  0.5
      samples( 4) =  1.0
      samples( 5) =  2.0
      samples( 6) =  1.5
      samples( 7) =  2.5
      samples( 8) =  3.
      samples( 9) =  3.5
      samples(10) =  4.
      samples(11) =  5.
      samples(12) =  6.
      samples(13) =  1.25
      samples(14) =  3.25
      samples(15) =  5.25

      do i=1,N
        X = samples(i)
        Y = GAMMAF(X)
        write(10,'(1I10,2ES23.14)') i, X, Y
      enddo

      close(10)

      call exit(0)
      end

!-----------------------------------------------------------------------
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
