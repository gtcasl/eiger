
       module test1
       integer :: n
       contains

       recursive subroutine sub1(x)
       integer,intent(inout):: x
       if (x < n) then
         x = x + 1
         print *, 'x = ', x
         call sub1(x)
       end if
       end subroutine sub1

       end module test1


#include "lwperf_f.h"

       program main

       use test1
       PERF_USE
       implicit none

#include "mpif.h"


       integer :: nx =0
       integer :: x = 0
       integer :: ierr,sz,rank = 0
       PERFDECL(real(kind=lwperf_double) :: dp =0 )

       call MPI_INIT(ierr)
       call MPI_Comm_rank(MPI_COMM_WORLD,rank,ierr)
       call MPI_Comm_size(MPI_COMM_WORLD,sz,ierr)
       PERF_INIT;
       PERF_MPI(rank,sz);
       PERF_OPTS("x5550","gcc","app.F90","fortapp","tesla.",".log") ;

       if (rank == 0) then
         print *, 'Enter number of repeats'
         read (*,*) n
       end if
       call MPI_Bcast(n,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr);

       PERFLOG1(sub1,IP(n)) ;
       PERFLOG2(sub2,IP(nx),DP(dp));
       call sub1(x)
       PERFSTOP2(sub2,IP(nx),DP(dp));
       PERFSTOP1(sub1,IP(n)) ;
       PERF_FINAL
       call MPI_FINALIZE()
       end program main

