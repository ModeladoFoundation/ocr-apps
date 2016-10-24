!-----------------------------------------------------------------------
! The code for the procedure "proxy_setup" was taken from
!   nekbone-2.3.4/src/proxy_setup.f
!-----------------------------------------------------------------------
      program test_proxy_setup

      implicit none

      !  Here 100 was chosen to be too big.
      !  The array length is controlled by pDOF.
      real ah(100*100),bh(100),ch(100*100),dh(100*100)
      real zpts(100), wght(100*2)

      ! See memoization note in setup_g()> real g(6,nx1*nx1*nx1*lelt)
      real dxm1(100*100), dxtm1(100*100), g(6,100*100*100)

      integer pDOF
      integer firstdegree, lastdegree

      ! If any of these values changes, make also the change in nekbone_proxy_setup.c
      ! Try to keep lastdegree to <= 84.  See Nekbone's speclib.f::ZWGJ for details.

      !2016Oct20: After experimentations, these ranges [firstdegree, lastdegree] had
      ! the following correlation with the C code:
      !     [ 2,  5] -> tol = 1e-14
      !     [ 5, 15] -> tol = 1e-13
      !     [15, 25] -> tol = 5e-13
      firstdegree = 2
      lastdegree  = 25

      do pDOF=firstdegree,lastdegree

        call proxy_setup(ah,bh,ch,dh,zpts,wght, pDOF, g, dxm1, dxtm1)

        call printout_matrices(pDOF, pDOF-1, g, dxm1, dxtm1)

      enddo

      call exit(0)
      end

!--------------------------------------------------------------------
      SUBROUTINE printout_matrices(nx1, n, g, dxm1, dxtm1)
        integer nx1,n
        real g(6,nx1,nx1,nx1)
        real dxm1(0:n,0:n),dxtm1(0:n,0:n)

        integer i,j,k, ny1,nz1

        ny1 = nx1
        nz1 = nx1

        write(*,'(2A, 1I10)') 'DXM1_&_DXtM1  ', 'pDOF= ', nx1
        do j=0,n
            do i=0,n
                write(*,'(2I10,2ES23.14)') i,j, dxm1(i,j), dxtm1(i,j)
            end do
        end do

        write(*,'(2A, 1I10)') 'Gauss Weights  ', 'pDOF= ', nx1
        do k=1,nz1
        do j=1,ny1
        do i=1,nx1
            write(*,'(3I10,6ES23.14)') i,j,k, g(1,i,j,k), g(2,i,j,k), g(3,i,j,k), g(4,i,j,k), g(5,i,j,k), g(6,i,j,k)
        enddo
        enddo
        enddo
        write(*,*) '-----'

      END SUBROUTINE
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
!--------------------------------------------------------------------
!
!     Generate NP GAUSS JACOBI points (Z) and weights (W)
!     associated with Jacobi polynomial P(N)(alpha>-1,beta>-1).
!     The polynomial degree N=NP-1.
!     Double precision version.
!
!--------------------------------------------------------------------
      SUBROUTINE ZWGJD (Z,W,NP,ALPHA,BETA)
      IMPLICIT REAL*8  (A-H,O-Z)

      REAL*8  Z(1),W(1),ALPHA,BETA

      integer k

      N     = NP-1
      DN    = ((N))
      ONE   = 1.
      TWO   = 2.
      APB   = ALPHA+BETA

      IF (NP.LE.0) THEN
         WRITE (6,*) 'ZWGJD: Minimum number of Gauss points is 1',np
         call exit(1)
      ENDIF
      IF ((ALPHA.LE.-ONE).OR.(BETA.LE.-ONE)) THEN
         WRITE (6,*) 'ZWGJD: Alpha and Beta must be greater than -1'
         call exit(2)
      ENDIF

      IF (NP.EQ.1) THEN
         Z(1) = (BETA-ALPHA)/(APB+TWO)
         W(1) = GAMMAF(ALPHA+ONE)*GAMMAF(BETA+ONE)/GAMMAF(APB+TWO) * TWO**(APB+ONE)
         RETURN
      ENDIF

      CALL JACG (Z,NP,ALPHA,BETA)

      !DBG> do k=1,NP
      !DBG>   write(*,'(A,1I10,1ES23.14)') "DBG>F> JACG> ", k, Z(k)
      !DBG> enddo

      NP1   = N+1
      NP2   = N+2
      DNP1  = ((NP1))
      DNP2  = ((NP2))
      FAC1  = DNP1+ALPHA+BETA+ONE
      FAC2  = FAC1+DNP1
      FAC3  = FAC2+ONE
      FNORM = PNORMJ(NP1,ALPHA,BETA)
      RCOEF = (FNORM*FAC2*FAC3)/(TWO*FAC1*DNP2)

      !DBG> write(*,'(A,2I10,5ES23.14)') "DBG>F> const> ", NP, NP1, FAC1, FAC2, FAC3, FNORM, RCOEF

      DO 123 I=1,NP
         CALL JACOBF (P,PD,PM1,PDM1,PM2,PDM2,NP2,ALPHA,BETA,Z(I))
         W(I) = -RCOEF/(P*PDM1)
 123  CONTINUE
      RETURN
      END
!--------------------------------------------------------------------
      REAL*8  FUNCTION ENDW1 (N,ALPHA,BETA)
      IMPLICIT REAL*8  (A-H,O-Z)
      REAL*8  ALPHA,BETA
      ZERO  = 0.
      ONE   = 1.
      TWO   = 2.
      THREE = 3.
      FOUR  = 4.
      APB   = ALPHA+BETA
      IF (N.EQ.0) THEN
         ENDW1 = ZERO
         RETURN
      ENDIF
      F1   = GAMMAF(ALPHA+TWO)*GAMMAF(BETA+ONE)/GAMMAF(APB+THREE)
      F1   = F1*(APB+TWO)*TWO**(APB+TWO)/TWO
      IF (N.EQ.1) THEN
         ENDW1 = F1
         RETURN
      ENDIF
      FINT1 = GAMMAF(ALPHA+TWO)*GAMMAF(BETA+ONE)/GAMMAF(APB+THREE)
      FINT1 = FINT1*TWO**(APB+TWO)
      FINT2 = GAMMAF(ALPHA+TWO)*GAMMAF(BETA+TWO)/GAMMAF(APB+FOUR)
      FINT2 = FINT2*TWO**(APB+THREE)
      F2    = (-TWO*(BETA+TWO)*FINT1 + (APB+FOUR)*FINT2) * (APB+THREE)/FOUR
      IF (N.EQ.2) THEN
         ENDW1 = F2
         RETURN
      ENDIF
      DO 100 I=3,N
         DI   = ((I-1))
         ABN  = ALPHA+BETA+DI
         ABNN = ABN+DI
         A1   = -(TWO*(DI+ALPHA)*(DI+BETA))/(ABN*ABNN*(ABNN+ONE))
         A2   =  (TWO*(ALPHA-BETA))/(ABNN*(ABNN+TWO))
         A3   =  (TWO*(ABN+ONE))/((ABNN+TWO)*(ABNN+ONE))
         F3   =  -(A2*F2+A1*F1)/A3
         F1   = F2
         F2   = F3
 100  CONTINUE
      ENDW1  = F3
      RETURN
      END
!--------------------------------------------------------------------
      REAL*8  FUNCTION ENDW2 (N,ALPHA,BETA)
      IMPLICIT REAL*8  (A-H,O-Z)
      REAL*8  ALPHA,BETA
      ZERO  = 0.
      ONE   = 1.
      TWO   = 2.
      THREE = 3.
      FOUR  = 4.
      APB   = ALPHA+BETA
      IF (N.EQ.0) THEN
         ENDW2 = ZERO
         RETURN
      ENDIF
      F1   = GAMMAF(ALPHA+ONE)*GAMMAF(BETA+TWO)/GAMMAF(APB+THREE)
      F1   = F1*(APB+TWO)*TWO**(APB+TWO)/TWO
      IF (N.EQ.1) THEN
         ENDW2 = F1
         RETURN
      ENDIF
      FINT1 = GAMMAF(ALPHA+ONE)*GAMMAF(BETA+TWO)/GAMMAF(APB+THREE)
      FINT1 = FINT1*TWO**(APB+TWO)
      FINT2 = GAMMAF(ALPHA+TWO)*GAMMAF(BETA+TWO)/GAMMAF(APB+FOUR)
      FINT2 = FINT2*TWO**(APB+THREE)
      F2    = (TWO*(ALPHA+TWO)*FINT1 - (APB+FOUR)*FINT2) * (APB+THREE)/FOUR
      IF (N.EQ.2) THEN
         ENDW2 = F2
         RETURN
      ENDIF
      DO 100 I=3,N
         DI   = ((I-1))
         ABN  = ALPHA+BETA+DI
         ABNN = ABN+DI
         A1   =  -(TWO*(DI+ALPHA)*(DI+BETA))/(ABN*ABNN*(ABNN+ONE))
         A2   =  (TWO*(ALPHA-BETA))/(ABNN*(ABNN+TWO))
         A3   =  (TWO*(ABN+ONE))/((ABNN+TWO)*(ABNN+ONE))
         F3   =  -(A2*F2+A1*F1)/A3
         F1   = F2
         F2   = F3
 100  CONTINUE
      ENDW2  = F3
      RETURN
      END
!--------------------------------------------------------------------
!
!     Generate NP GAUSS LOBATTO JACOBI points (Z) and weights (W)
!     associated with Jacobi polynomial P(N)(alpha>-1,beta>-1).
!     The polynomial degree N=NP-1.
!     Double precision version.
!
!--------------------------------------------------------------------
      SUBROUTINE ZWGLJD (Z,W,NP,ALPHA,BETA)
      IMPLICIT REAL*8  (A-H,O-Z)
      REAL*8  Z(NP),W(NP),ALPHA,BETA

      N     = NP-1
      NM1   = N-1
      ONE   = 1.
      TWO   = 2.

      IF (NP.LE.1) THEN
       WRITE (6,*) 'ZWGLJD: Minimum number of Gauss-Lobatto points is 2'
       WRITE (6,*) 'ZWGLJD: alpha,beta:',alpha,beta,np
       call exit(84)
      ENDIF
      IF ((ALPHA.LE.-ONE).OR.(BETA.LE.-ONE)) THEN
         WRITE (6,*) 'ZWGLJD: Alpha and Beta must be greater than -1'
         call exit(88)
      ENDIF

      IF (NM1.GT.0) THEN
         ALPG  = ALPHA+ONE
         BETG  = BETA+ONE
         CALL ZWGJD (Z(2),W(2),NM1,ALPG,BETG)
      ENDIF
      Z(1)  = -ONE
      Z(NP) =  ONE
      DO 234  I=2,NP-1
         W(I) = W(I)/(ONE-Z(I)**2)
 234  CONTINUE
      CALL JACOBF (P,PD,PM1,PDM1,PM2,PDM2,N,ALPHA,BETA,Z(1))
      W(1)  = ENDW1 (N,ALPHA,BETA)/(TWO*PD)
      CALL JACOBF (P,PD,PM1,PDM1,PM2,PDM2,N,ALPHA,BETA,Z(NP))
      W(NP) = ENDW2 (N,ALPHA,BETA)/(TWO*PD)

      RETURN
      END

!--------------------------------------------------------------------
!
!     Generate NP GAUSS LOBATTO JACOBI points (Z) and weights (W)
!     associated with Jacobi polynomial P(N)(alpha>-1,beta>-1).
!     The polynomial degree N=NP-1.
!     Single precision version.
!
!--------------------------------------------------------------------
      SUBROUTINE ZWGLJ (Z,W,NP,ALPHA,BETA)
      PARAMETER (NMAX=84)
      PARAMETER (NZD = NMAX)
      REAL*8  ZD(NZD),WD(NZD)
      REAL Z(1),W(1),ALPHA,BETA

      NPMAX = NZD
      IF (NP.GT.NPMAX) THEN
         WRITE (6,*) 'Too large polynomial degree in ZWGLJ'
         WRITE (6,*) 'Maximum polynomial degree is',NMAX
         WRITE (6,*) 'Here NP=',NP
         call exit(10)
      ENDIF
      ALPHAD = ALPHA
      BETAD  = BETA
      CALL ZWGLJD (ZD,WD,NP,ALPHAD,BETAD)
      DO 415 I=1,NP
         Z(I) = ZD(I)
         W(I) = WD(I)
 415  CONTINUE
      RETURN
      END
!--------------------------------------------------------------------
!
!     Generate NP Gauss-Lobatto Legendre points (Z) and weights (W)
!     associated with Jacobi polynomial P(N)(alpha=0,beta=0).
!     The polynomial degree N=NP-1.
!     Z and W are in single precision, but all the arithmetic
!     operations are done in double precision.
!
!--------------------------------------------------------------------
     SUBROUTINE ZWGLL (Z,W,NP)
      REAL Z(1),W(1)
      ALPHA = 0.
      BETA  = 0.
      CALL ZWGLJ (Z,W,NP,ALPHA,BETA)
      RETURN
      END

!-----------------------------------------------------------------------
!     This routine evaluates the derivative based on all points
!     in the stencils.  It is more memory efficient than "fd_weights"
!
!     This set of routines comes from the appendix of
!     A Practical Guide to Pseudospectral Methods, B. Fornberg
!     Cambridge Univ. Press, 1996.   (pff)
!
!     Input parameters:
!       xx -- point at wich the approximations are to be accurate
!       x  -- array of x-ordinates:   x(0:n)
!       n  -- polynomial degree of interpolant (# of points := n+1)
!       m  -- highest order of derivative to be approxxmated at xi
!
!     Output:
!       c  -- set of coefficients c(0:n,0:m).
!             c(j,k) is to be applied at x(j) when
!             the kth derivative is approxxmated by a
!             stencil extending over x(0),x(1),...x(n).

      subroutine fd_weights_full(xx,x,n,m,c)
      real x(0:n),c(0:n,0:m)

      c1       = 1.
      c4       = x(0) - xx

      do k=0,m
      do j=0,n
         c(j,k) = 0.
      enddo
      enddo
      c(0,0) = 1.

      do i=1,n
         mn = min(i,m)
         c2 = 1.
         c5 = c4
         c4 = x(i)-xx
         do j=0,i-1
            c3 = x(i)-x(j)
            c2 = c2*c3
            do k=mn,1,-1
               c(i,k) = c1*(k*c(i-1,k-1)-c5*c(i-1,k))/c2
            enddo
            c(i,0) = -c1*c5*c(i-1,0)/c2
            do k=mn,1,-1
               c(j,k) = (c4*c(j,k)-k*c(j,k-1))/c3
            enddo
            c(j,0) = c4*c(j,0)/c3
         enddo
         c1 = c2
      enddo
      return
      end
!-----------------------------------------------------------------------
      subroutine rzero(a,n)
      DIMENSION  A(1)

      DO I = 1, N
         A(I ) = 0.0
      enddo
      return
      END
!-----------------------------------------------------------------------
!     Generate matrices for single element, 1D operators:
!
!        a    = Laplacian
!        b    = diagonal mass matrix    (GLL weights)
!        c    = convection operator b*d
!        d    = derivative matrix
!        z    = GLL points
!
      subroutine semhat(a,b,c,d,z,w,n)

      real a(0:n,0:n),b(0:n),c(0:n,0:n),d(0:n,0:n),z(0:n)
      real w(0:2*n)

      np = n+1

      call zwgll (z,b,np)

!DBG>      do i=0,n
!        write(*,'(A, 1I10,2ES23.14)') 'DBG-ZB> ', i, z(i), b(i)
!      end do

      do i=0,n
         call fd_weights_full(z(i),z,n,1,w)
         do j=0,n
            d(i,j) = w(j+np)                   !  Derivative matrix
         enddo
      enddo

!DBG>      do i=0,n
!         do j=0,n
!            write(*,'(A, 2I10,1ES23.14)') 'DBG-D> ', i, j, d(i,j)
!         enddo
!      enddo

      call rzero(a,np*np)
      do j=0,n
      do i=0,n
         do k=0,n
            a(i,j) = a(i,j) + d(k,i)*b(k)*d(k,j)
         enddo
         c(i,j) = b(i)*d(i,j)
      enddo
      enddo

!DBG>      do i=0,n
!         do j=0,n
!            write(*,'(A, 2I10,2ES23.14)') 'DBG-AC> ', i, j, a(i,j), c(i,j)
!         enddo
!      enddo

      return
      end
!-----------------------------------------------------------------------
      subroutine copy(a,b,n)
      real a(1),b(1)

      do i=1,n
         a(i)=b(i)
      enddo

      return
      end
!-------------------------------------------------------------------------
      subroutine transpose(a,lda,b,ldb)
      real a(lda,1),b(ldb,1)

      do j=1,ldb
         do i=1,lda
            a(i,j) = b(j,i)
         enddo
      enddo
      return
      end
!-------------------------------------------------------------------------
      subroutine setup_g(g, nx1, wxm1)
      integer nx1
      real g(6,nx1,nx1,nx1), wxm1(nx1)
      !not in use: integer e
      integer i,j,k, ny1, nz1

      ny1 = nx1
      nz1 = nx1

      !2016Oct21: Noting that all g(1:6,i,j,k,e) are all the same for any value of e,
      !           the simplification of calculating only once g for e=1
      !           was decided.  This is equivalent to what would happen if a proper
      !           memorization mechanism was used, as is the case in many actual
      !           application.
      !TODO: Remove memoization of Gaussian weights g.
!      do e=1,nelt
      do k=1,nz1
      do j=1,ny1
      do i=1,nx1
         call rzero(g(1,i,j,k),6)
         g(1,i,j,k) = wxm1(i)*wxm1(j)*wxm1(k)
         g(4,i,j,k) = wxm1(i)*wxm1(j)*wxm1(k)
         g(6,i,j,k) = wxm1(i)*wxm1(j)*wxm1(k)
         g(6,i,j,k) = wxm1(i)*wxm1(j)*wxm1(k)
      enddo
      enddo
      enddo
!      enddo

      return
      end
!-----------------------------------------------------------------------
      subroutine proxy_setup(a,b,c,d,z,w, nx1, o_g, o_dxm1, o_dxtm1)

      integer nx1,n2

      ! See memoization note in setup_g()> real g(6,nx1*nx1*nx1*lelt)
      real o_dxm1(nx1*nx1),o_dxtm1(nx1*nx1), o_g(6,nx1,nx1,nx1)

      real a(nx1*nx1),b(nx1),c(nx1*nx1),d(nx1*nx1),z(nx1)
      real w(nx1*2), wxm1(nx1)

      call semhat(a,b,c,d,z,w,nx1-1)

      n2 = nx1*nx1
      call copy(o_dxm1,d,n2)
      call transpose(o_dxtm1,nx1,o_dxm1,nx1)

      !2016OCt21: Although important in normal usage, the Gll points zgm1 are
      !           not used anywhere in the baseline NEkbone code.
      !           So they will be omitted for now.
      !TODO: add zgm1 even if they are still not used.
      !Not in use--> call copy(zgm1,z,nx1)   ! GLL points

      call copy(wxm1,b,nx1)   ! GLL weights

      call setup_g(o_g, nx1, wxm1)

      return
      end
