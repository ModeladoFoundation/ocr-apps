!-----------------------------------------------------------------------
! The code for the procedure "cubic" was taken from
!   nekbone-2.3.4/src/driver.f
!-----------------------------------------------------------------------
      program test_init_mesh

!setup.h  Using bash to get the preprocessor macro in here

      implicit none
      integer maxValue
      integer x,y,z
      integer k

      integer if_brick
      logical ifbrick   !use a brick shape or not

      integer nelt
      integer nx1,ny1,nz1,ndim  !globals

      integer np    !MPI rank count
      integer nid   !MPI rank ID    0<= nid < np

      integer npx,npy,npz      ! rank distribution
      integer mx,my,mz         ! local element distribution in x,y,z
      integer nelx,nely,nelz

      integer lglel(G_IELN)
      integer*8 i

      integer*8 glo_num(G_NXN * G_NXN * G_NXN * G_IELN)

      open(unit=10,file=OUTPUT_FILE)
      ! Set mpx,npy,npz to bogu numbers
      npx=1
      npy=1
      npz=1
      ! Set mx,my,mz to bogu numbers
      mx=1
      my=1
      mz=1

      write(10,*) "#if_brick,nelt,np,nid, npx,npy,npz, mx,my,mz, nelx,nely,nelz"

!       nx1,ny1,nz1,ndim are all globals from SIZE
      do nx1 = G_NX0, G_NXN, G_NXD
        ny1 = nx1
        nz1 = nx1
        do nelt = G_IEL0, G_IELN, G_IELD
            do nid = G_MPI_RANK_ID_MIN, G_MPI_RANK_ID_MAX, G_MPI_RANK_ID_D
                np = G_MPI_RANK_ID_MAX + 1
                do if_brick=G_IFBRICK_MIN, G_IFBRICK_MAX, G_IFBRICK_D
                    if (if_brick.eq.0) then
                        ifbrick=.false.
                    else
                        ifbrick=.true.
                    end if
                    ! Set mpx,npy,npz to invalid numbers
                    npx=1
                    npy=1
                    npz=1
                    ! Set mx,my,mz to invalid numbers
                    mx=1
                    my=1
                    mz=1

                    call init_mesh(ifbrick, npx,npy,npz, mx,my,mz, nelt,np,nid, nelx,nely,nelz, lglel)

                    write(10,'(13I8)') if_brick,nelt,np,nid,  npx,npy,npz, mx,my,mz, nelx,nely,nelz

                    call set_vert_box(lglel, nx1,ny1,nz1, nelx,nely,nelz, nelt, glo_num)

                    do i=0,(G_NXN * G_NXN * G_NXN * G_IELN)
                        write(10,*) i, glo_num(i)
                    enddo

                enddo
            enddo
        enddo
      enddo

      close(10)

      call exit(0)

      end program
!-----------------------------------------------------------------------
      subroutine set_vert_box(in_lglel, in_nx1, in_ny1, in_nz1, in_nelx, in_nely, in_nelz, in_nelt, o_glo_num)

!     Set up global numbering for elements in a box

!      include 'SIZE'
!      include 'PARALLEL'

      integer*8 o_glo_num(1),ii,kg,jg,ig ! The latter 3 for proper promotion

      integer in_lglel(*)
      integer in_nx1, in_ny1, in_nz1, in_nelx, in_nely, in_nelz, in_nelt

      integer e,ex,ey,ez,eg

      nn = in_nx1-1  ! nn := polynomial order

      do e=1,in_nelt
        eg = in_lglel(e)
        call get_exyz(ex,ey,ez,eg, in_nelx, in_nely, in_nelz)
        do k=0,nn
        do j=0,nn
        do i=0,nn
           kg = nn*(ez-1) + k
           jg = nn*(ey-1) + j
           ig = nn*(ex-1) + i
           ii = 1 + ig + jg*(nn*in_nelx+1) + kg*(nn*in_nelx+1)*(nn*in_nely+1)
           ll = 1 + i + in_nx1*j + in_nx1*in_ny1*k + in_nx1*in_ny1*in_nz1*(e-1)
           o_glo_num(ll) = ii
        enddo
        enddo
        enddo
      enddo

      return
      end
!-----------------------------------------------------------------------
      subroutine get_exyz(ex,ey,ez,eg,nelx,nely,nelz)
      integer ex,ey,ez,eg

      nelxy = nelx*nely

      ez = 1 +  (eg-1)/nelxy
      ey = mod1 (eg,nelxy)
      ey = 1 +  (ey-1)/nelx
      ex = mod1 (eg,nelx)

      return
      end
!-----------------------------------------------------------------------
      function mod1(i,n)
!
!     Yields MOD(I,N) with the exception that if I=K*N, result is N.
!
      MOD1=0
      IF (I.EQ.0) THEN
         return
      ENDIF
      IF (N.EQ.0) THEN
         WRITE(6,*) 'WARNING:  Attempt to take MOD(I,0) in function mod1.'
         return
      ENDIF
      II = I+N-1
      MOD1 = MOD(II,N)+1
      return
      END
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
      subroutine init_mesh(in_ifbrick, npx,npy,npz, mx,my,mz, in_nelt,in_np,in_nid, nelx,nely,nelz, lglel)

        implicit none
        logical in_ifbrick
        integer npx,npy,npz
        integer mx,my,mz

        integer in_nelt, in_np,in_nid
        integer nelx,nely,nelz

        integer lglel(*)

        integer i,j,k

!       SIZE provides
!           nelx,nely,nelz,nelt
!           nelt is set to "do nelt = iel0, ielN, ielD" in driver.f::program nekbone
!           nid the MPI rank ID in SIZE & comm_mpi.f::iniproc
!       TOTAL::PARALLEL provides np:
!           np = Number of MPI ranks
!           lglel = of length nelt

!      include 'SIZE'
!      include 'TOTAL'

      integer e, eg, offs

      if(.not. in_ifbrick) then   ! A 1-D array of elements of length P*lelt
         nelx = in_nelt*in_np
         nely = 1
         nelz = 1

         do e=1,in_nelt
            eg = e + in_nid * in_nelt
            lglel(e) = eg
         enddo
      else              ! A 3-D block of elements
        if (npx*npy*npz .ne. in_np) then
          call cubic(npx,npy,npz,in_np)  !xyz distribution of total proc
        end if
        if (mx*my*mz .ne. in_nelt) then
          call cubic(mx,my,mz,in_nelt)   !xyz distribution of elements per proc
        end if

        nelx = mx*npx
        nely = my*npy
        nelz = mz*npz

        e = 1
        offs = (mod(in_nid,npx)*mx) + npx*(my*mx)*(mod(in_nid/npx,npy)) + (npx*npy)*(mx*my*mz)*(in_nid/(npx*npy))
        do k = 0,mz-1
        do j = 0,my-1
        do i = 0,mx-1
           eg = offs+i+(j*nelx)+(k*nelx*nely)+1
           lglel(e) = eg
           e        = e+1
        enddo
        enddo
        enddo
      endif

!      if (in_nid.eq.0) then
!        write(6,*) "Processes: npx= ", npx, " npy= ", npy, " npz= ", npz
!        write(6,*) "Local Elements: mx= ", mx, " my= ", my, " mz= ", mz
!        write(6,*) "Elements: nelx= ", nelx, " nely= ", nely,
!     &             " nelz= ", nelz
!      end if

      return
      end
!-----------------------------------------------------------------------
