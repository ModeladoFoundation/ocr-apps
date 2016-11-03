!-----------------------------------------------------------------------
! The code for the procedures "local_grad3" and "local_grad3t" were
! taken from
!   nekbone-2.3.4/src/cg.f
! as were all supporting subroutines.  The subroutine mxm was abridged.
!-----------------------------------------------------------------------
      program test_both_local_grad

      implicit none
      integer first_pDOF, last_pDOF
      integer dof

      first_pDOF = 2
      last_pDOF = 10

      open(unit=10,file='z_fortran.out')

      do dof=first_pDOF,last_pDOF
        call check_local_grad3(dof)
        call check_local_grad3t(dof)
      enddo

      close(10)

      call exit(0)
      end
!-----------------------------------------------------------------------
      subroutine build_data(pDOF, ur,us,ut, u, D,Dt)
        implicit none
        integer pDOF

        real ur(pDOF,pDOF,pDOF)
        real us(pDOF,pDOF,pDOF)
        real ut(pDOF,pDOF,pDOF)

        real u(pDOF,pDOF,pDOF)
        real D(pDOF,pDOF)
        real Dt(pDOF,pDOF)

        integer i,j,k

        do i=1,pDOF
            do j=1,pDOF
                D(i,j) =1 +(i-1) + (pDOF)*(j-1)
                Dt(j,i)=D(i,j)
            enddo
        enddo

        do i=1,pDOF
            do j=1,pDOF
                do k=1,pDOF
                    u(i,j,k) = i+5*j+7*k
                    ur(i,j,k) = 1+(i-1)
                    us(i,j,k) = 2+(j-1)
                    ut(i,j,k) = 3+(k-1)
                enddo
            enddo
        enddo

      end subroutine
!-----------------------------------------------------------------------
      subroutine check_local_grad3(pDOF)
        implicit none
        integer pDOF, pOrder

        real ur(pDOF,pDOF,pDOF)
        real us(pDOF,pDOF,pDOF)
        real ut(pDOF,pDOF,pDOF)

        real u(pDOF,pDOF,pDOF)
        real D(pDOF,pDOF)
        real Dt(pDOF,pDOF)

        integer i,j,k

        pOrder = pDOF -1

        call build_data(pDOF, ur,us,ut, u, D,Dt)

        call local_grad3(ur,us,ut, u, pOrder, D,Dt)

        write(10,'(A,1I10)') "LG_pDOF= ", pDOF
        do i=1,pDOF
            do j=1,pDOF
                do k=1,pDOF
                    write(10,'(3I10,4ES23.14)') i,j,k, ur(i,j,k), us(i,j,k), ut(i,j,k), u(i,j,k)
                enddo
            enddo
        enddo

      return
      end
!-----------------------------------------------------------------------
      subroutine check_local_grad3t(pDOF)
        implicit none
        integer pDOF, pOrder

        real ur(pDOF,pDOF,pDOF)
        real us(pDOF,pDOF,pDOF)
        real ut(pDOF,pDOF,pDOF)

        real u(pDOF,pDOF,pDOF)
        real D(pDOF,pDOF)
        real Dt(pDOF,pDOF)

        integer i,j,k

        pOrder = pDOF -1

        call build_data(pDOF, ur,us,ut, u, D,Dt)

        call local_grad3_t(u, ur,us,ut, pOrder, D,Dt)

        write(10,'(A,1I10)') "LGt_pDOF= ", pDOF
        do i=1,pDOF
            do j=1,pDOF
                do k=1,pDOF
                    write(10,'(3I10,4ES23.14)') i,j,k, ur(i,j,k), us(i,j,k), ut(i,j,k), u(i,j,k)
                enddo
            enddo
        enddo

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
