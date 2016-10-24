!-----------------------------------------------------------------------
! The code for the procedure "fd_weights_full" was taken from
!   nekbone-2.3.4/src/semhat.f
!-----------------------------------------------------------------------
      program test_fd_weights_full

      implicit none

      !Porder and Ntry have to be the same as in their C counterpart.

      integer, parameter :: Porder = 10 ! Porder = (Nb of DOF for given polynomial) - 1

      REAL z(0:Porder)
      REAL w(0:Porder, 0:Porder)
      integer m

      integer i,j,k, Ntry, Nstep
      REAL po

      Ntry = 5

      m = 1 ! That is all that the Nekbone Fortran code uses.
      po = Porder

      open(unit=10,file='z_fortran.out')

      do k=1,Ntry   ! 1 to 5 chosen quasi-randomly, just to stress the function
        do i=0,Porder
            z(i) = (i+k)/(po)
            do j=0,Porder
                w(j,i) = 0
            enddo
        enddo
        do i=0,Porder
          call fd_weights_full(z(i),z,Porder,m,w)
          write(10,'(2I10,1ES23.14)') k, i, z(i)
          do j=0,Porder
            write(10,'(3I10,1ES23.14)') k, i, j, w(i,j)
          end do
        enddo

      enddo

      close(10)

      call exit(0)
      end

!-----------------------------------------------------------------------
      subroutine fd_weights_full(xx,x,n,m,c)
!
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
!
!
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
