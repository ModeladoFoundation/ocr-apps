!-----------------------------------------------------------------------
! The code for the procedure "cubic" was taken from
!   nekbone-2.3.4/src/driver.f
!-----------------------------------------------------------------------
      program test_cubic

      implicit none
      integer maxValue
      integer x,y,z
      integer k

      call read_param(maxValue)
      !write(*,*) maxValue

      open(unit=10,file='fortran.out')

      do k=1,maxValue,1
        call cubic(x,y,z, k)
        write(10,'(4I10)') k,x,y,z
      enddo

      close(10)

      call exit(0)
      end
!-----------------------------------------------------------------------
      subroutine cubic(mx,my,mz,np)

      mx = np
      my = 1
      mz = 1
      ratio = np

      iroot3 = np**(1./3.) + 0.000001
      do i= iroot3,1,-1
        iz = i
        myx = np/iz
        nrem = np-myx*iz

        if (nrem.eq.0) then
          iroot2 = myx**(1./2.) + 0.000001
          do j=iroot2,1,-1
            iy = j
            ix = myx/iy
            nrem = myx-ix*iy
            if (nrem.eq.0) goto 20
          enddo
   20     continue

          if (ix < iy) then
            it = ix
            ix = iy
            iy = it
          end if

          if (ix < iz) then
            it = ix
            ix = iz
            iz = it
          end if

          if (iy < iz) then
            it = iy
            iy = iz
            iz = it
          end if

          if ( REAL(ix)/iz < ratio) then
            ratio = REAL(ix)/iz
            mx = ix
            my = iy
            mz = iz
          end if

        end if
      enddo

      return
      end

!-----------------------------------------------------------------------
      subroutine read_param(o_maxval)

      integer o_maxval

      open(unit=9,file='inputparams.dat',status='old')
      read(9,*,err=100) o_maxval
      close(9)

      if(o_maxval <=0) goto 200
      return

  100 continue
      write(6,*) "ERROR READING inputparams.dat"
      call exit(1)

  200 continue
      write(6,*) "ERROR The maximum value must be a non-negative non-zero integer."
      call exit(2)

      return
      end
!-----------------------------------------------------------------------
