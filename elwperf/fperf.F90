 module lwperf

 use iso_c_binding
 implicit none

 interface

 subroutine lwperf_init()  bind(C, name="lwperf_init")
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

 subroutine fileOptions(host,tools,app,db, p,s) bind(C, name="lwperf_fileOptions")
 ! wrapper expecting c null-terminated strings w/out trailing whitespace
 use iso_c_binding
   character(kind=c_char) :: host(*),tools(*),app(*),db(*), p(*), s(*)
 end subroutine fileOptions 

#include "fperf._save.h"

 end interface

  INTEGER, PARAMETER ::  lwperf_double = kind(1d0)
#include "flocations.h"

 contains

 subroutine lwperf_fileOptionsWrapper(host,tools,app,db, prefix, suffix)
 use iso_c_binding
  character(len=*), INTENT(IN) :: prefix, suffix, host,tools,app,db
  character(len=len_trim(host)+1) :: c_host
  character(len=len_trim(tools)+1) :: c_tools
  character(len=len_trim(app)+1) :: c_app
  character(len=len_trim(db)+1) :: c_db
  character(len=len_trim(prefix)+1) :: c_p
  character(len=len_trim(suffix)+1) :: c_s
  c_host = trim(host) // C_NULL_CHAR
  c_tools = trim(tools) // C_NULL_CHAR
  c_app = trim(app) // C_NULL_CHAR
  c_db = trim(db) // C_NULL_CHAR
  c_p = trim(prefix) // C_NULL_CHAR
  c_s = trim(suffix) // C_NULL_CHAR
  ! write(*,*) "prefix", trim(prefix)
  !print *, "suffix", trim(suffix)
  call fileOptions(c_host, c_tools, c_app, c_db, c_p,c_s)
 end subroutine
 
 end module 
  
