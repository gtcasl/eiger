 module lwperf

 use iso_c_binding
 implicit none

 interface

 subroutine lwperf_init(machine,app,db, p,s)  bind(C, name="lwperf_init")
 ! wrapper expecting c null-terminated strings w/out trailing whitespace
 use iso_c_binding
   character(kind=c_char) :: machine(*),app(*),db(*), p(*), s(*)
 end subroutine

 subroutine lwperf_finalize() bind(C, name="lwperf_finalize")
 end subroutine

 subroutine lwperf_mpiArgs(commrank,commsize) bind(C,name="lwperf_mpiArgs")
 use iso_c_binding
  integer(c_int), value, INTENT(IN) :: commrank,commsize
  ! print *, "r,s=",commrank, commsize
 end subroutine

 subroutine lwperf_log(n) bind(C,name="lwperf_log")
 use iso_c_binding
  INTEGER(c_int), value, INTENT(IN) :: n 
 end subroutine

 subroutine lwperf_start(n) bind(C,name="lwperf_start")
 use iso_c_binding
  INTEGER(c_int), value, INTENT(IN) :: n 
 end subroutine

 subroutine lwperf_stop(n) bind(C,name="lwperf_stop")
 use iso_c_binding
  INTEGER(c_int), value, INTENT(IN) :: n 
 end subroutine

#include "fperf._save.h"

 end interface

  INTEGER, PARAMETER ::  lwperf_double = kind(1d0)
#include "flocations.h"

 contains

 end module 
  
