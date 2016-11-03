!-----------------------------------------------------------------------
! The code for the procedure "set_f" was taken from
!   nekbone-2.3.4/src/driver.f
!-----------------------------------------------------------------------
      program test_set_f

      implicit none

      integer pDOF, elementCount, N
      integer i

      !Assuming that pDOF can never be >= 25
      !Assuming that elementCount can never be >= 1000
      real f(25*25*25*1000)

      !If any of these values changes, make also the change in nekbone_set_f.f90
      pDOF = 25
      elementCount = 10

      open(unit=10,file='z_fortran.out')

      N = pDOF * pDOF * pDOF * elementCount
      call set_f(f,N)

      do i=1,N
        write(10,'(3I10,1ES23.14)') pDOF, elementCount, i, f(i)
      enddo

      close(10)

      call exit(0)
      end

!--------------------------------------------------------------------
! The variable c, found in the original set_f(), is not used in this unit test.
      subroutine set_f(f,n)
      real f(n)

      do i=1,n
         arg  = 1.e9*(i*i)  ! implicitly arg is of type REAL
         arg  = 1.e9*cos(arg)
         f(i) = sin(arg)
      enddo

      return
      end
