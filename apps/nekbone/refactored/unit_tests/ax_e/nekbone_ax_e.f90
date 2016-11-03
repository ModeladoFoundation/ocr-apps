!-----------------------------------------------------------------------
! The code for the procedures "ax_e" was taken from
!   nekbone-2.3.4/src/cg.f
! as were all supporting subroutines.  The subroutine mxm was abridged.
!-----------------------------------------------------------------------
      program test_ax_e

      implicit none

      !100 is a very big number, which should never be reached
      real w(100*100*100), u(100*100*100), g(6,100*100*100)
      real dxm1(100*100),dxtm1(100*100)
      integer dof, dof3D, first_pDOF, last_pDOF
      integer i

      first_pDOF = 2
      last_pDOF = 20

      open(unit=10,file='z_fortran.out')

      do dof=first_pDOF,last_pDOF
        dof3D = dof*dof*dof

        write(10,'(A,1I10)') "pDOF= ", dof
        call build_variables(dof,dof3D, w,u,g,dxm1,dxtm1)

        call ax_e(w,u,g, dof,dof3D, dxm1,dxtm1)

        do i=1,dof3D
            write(10,'(1I10,2ES23.14)') i, w(i), u(i)
        end do
      enddo

      close(10)

      call exit(0)
      end
!-----------------------------------------------------------------------
      subroutine build_variables(pDOF,pDOF3D, w,u,g,D,Dt)
        implicit none
        integer pDOF, pDOF3D
        real w(pDOF3D), u(pDOF3D), g(6,pDOF3D)
        real D(pDOF,pDOF), Dt(pDOF,pDOF)
        integer i,j
        real x

        do i=1,pDOF
            do j=1,pDOF
                x = 1 + (i-1)
                x = x / pDOF
                x = x + (j-1)
                D(i,j) = x
                Dt(j,i) = D(i,j)
            enddo
        enddo

        do i=1,6
            do j=1,pDOF3D
                x = j
                x = x / pDOF3D
                x = x + (i-1)
                if( (i .EQ. 1) .OR. (i .EQ. 4) .OR. (i .EQ. 6)) then
                    g(i,j) = x
                else
                    g(i,j) = 0
                endif
            enddo
        enddo

        do i=1,pDOF3D
            x = i
            x = x / pDOF3D
            w(i) = x
            x = (pDOF3D - i +1)
            x = x / pDOF3D
            u(i) = x
        end do

      end subroutine
!-------------------------------------------------------------------------
      subroutine ax_e(w,u,g, pDOF,pDOF3D, dxm1,dxtm1) ! Local matrix-vector product

      integer pDOF, pDOF3D
      real w(pDOF3D),u(pDOF3D),g(6,pDOF3D)
      real dxm1(pDOF,pDOF), dxtm1(pDOF,pDOF)

      real ur(pDOF3D),us(pDOF3D),ut(pDOF3D)

      nxyz = pDOF3D
      n    = pDOF - 1

      call local_grad3(ur,us,ut,u,n,dxm1,dxtm1)

      do i=1,nxyz
         wr = g(1,i)*ur(i) + g(2,i)*us(i) + g(3,i)*ut(i)
         ws = g(2,i)*ur(i) + g(4,i)*us(i) + g(5,i)*ut(i)
         wt = g(3,i)*ur(i) + g(5,i)*us(i) + g(6,i)*ut(i)
         ur(i) = wr
         us(i) = ws
         ut(i) = wt
      enddo

      call local_grad3_t(w,ur,us,ut,n,dxm1,dxtm1)

      return
      end
!-----------------------------------------------------------------------
      subroutine mxm(a,n1,b,n2,c,n3)
        implicit none
        integer n1,n2,n3
        real a(n1,n2),b(n2,n3),c(n1,n3)
        integer i,j,k
        real som

        do i=1,n1
            do j=1,n3
                som=0
                do k=1,n2
                   som = som + a(i,k)*b(k,j)
                enddo
                c(i,j)=som
            enddo
        enddo
      return
      end
!-----------------------------------------------------------------------
      subroutine local_grad3(ur,us,ut,u,n,D,Dt)
!     Output: ur,us,ut         Input:u,n,D,Dt
      implicit none
      real ur(0:n,0:n,0:n),us(0:n,0:n,0:n),ut(0:n,0:n,0:n)
      real u (0:n,0:n,0:n)
      real D (0:n,0:n),Dt(0:n,0:n)
      integer m1, m2, k, n

      m1 = n+1
      m2 = m1*m1

      call mxm(D ,m1,u,m1,ur,m2)
      do k=0,n
         call mxm(u(0,0,k),m1,Dt,m1,us(0,0,k),m1)
      enddo
      call mxm(u,m2,Dt,m1,ut,m1)

      return
      end
!-----------------------------------------------------------------------
      subroutine prettyPrint_matrix2D(nbRows,nbCols,M)
        implicit none
        integer nbRows, nbCols
        real M(nbRows, nbCols)
        integer i,j

        do i=1,nbRows
            do j=1,nbCols
                write(*,*) "  M2(", i,", ", j,")=",M(i,j)
            enddo
        enddo

      return
      end

!-----------------------------------------------------------------------
      subroutine prettyPrint_matrix3D(nbRows,nbCols,nbDepth, M)
        implicit none
        integer nbRows, nbCols, nbDepth
        real M(nbRows, nbCols, nbDepth)
        integer i,j,k

        do i=1,nbRows
            do j=1,nbCols
                do k=1,nbDepth
                    write(*,*) "  M3(", i,", ", j,", ", k,")=",M(i,j,k)
                enddo
            enddo
        enddo

      return
      end
!-----------------------------------------------------------------------
      subroutine local_grad3_t(u,ur,us,ut,N,D,Dt)

!     Output: ur,us,ut         Input:u,N,D,Dt
      real u (0:N,0:N,0:N)
      real ur(0:N,0:N,0:N),us(0:N,0:N,0:N),ut(0:N,0:N,0:N)
      real D (0:N,0:N),Dt(0:N,0:N)
      real w (0:N,0:N,0:N)
      integer k
      integer m1,m2,m3

      m1 = N+1
      m2 = m1*m1
      m3 = m1*m1*m1

      call mxm(Dt,m1,ur,m1,u,m2)

      do k=0,N
         call mxm(us(0,0,k),m1,D ,m1,w(0,0,k),m1)
      enddo
      call add2(u,w,m3)

      call mxm(ut,m2,D ,m1,w,m1)
      call add2(u,w,m3)

      return
      end

!-----------------------------------------------------------------------
      subroutine add2(a,b,n)
      real a(1),b(1)
      do i=1,n
         a(i)=a(i)+b(i)
      enddo
      return
      end
