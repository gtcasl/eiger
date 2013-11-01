#ifndef fperf_h
#define fperf_h

!
! header for fortran source inclusion.
! user can define up to 9 logged parameters at each
! logging site. (This limit is arbitrary and could be
! changed by editing the macro sets here and the generator script.


#if (!defined(_USE_CSV) && !defined(_USE_EIGER))
!// turn it all off
#define PERF ErRoR

#define PERF_USE(a,b,c,d,e,f)
#define PERF_INIT
#define PERF_MPI(a,b)
#define PERF_FINAL

#define PERFDECL(x)
#define PERFLOG1(X, a )
#define PERFLOG2(X, a,b )
#define PERFLOG3(X, a,b,c )
#define PERFLOG4(X, a,b,c,d )
#define PERFLOG5(X, a,b,c,d,e )
#define PERFLOG6(X, a,b,c,d,e,f )
#define PERFLOG7(X, a,b,c,d,e,f,g )
#define PERFLOG8(X, a,b,c,d,e,f,g,h )
#define PERFLOG9(X, a,b,c,d,e,f,g,h,i )
#define PERFSTOP1(X, a ) 
#define PERFSTOP2(X, a,b )
#define PERFSTOP3(X, a,b,c )
#define PERFSTOP4(X, a,b,c,d )
#define PERFSTOP5(X, a,b,c,d,e )
#define PERFSTOP6(X, a,b,c,d,e,f )
#define PERFSTOP7(X, a,b,c,d,e,f,g )
#define PERFSTOP8(X, a,b,c,d,e,f,g,h )
#define PERFSTOP9(X, a,b,c,d,e,f,g,h,i )

#define DR(v)
#define DD(v)
#define DN(v)
#define IR(v)
#define ID(v)
#define IN(v)

#else
!! // no PERF_DISABLE defined

#define PERF_USE use lwperf
#define PERF_INIT(m,a,d,prefix,suffix) call lwperf_init(m,a,d,prefix,suffix)
#define PERF_MPI(rank,size) call lwperf_mpiArgs(rank,size)
#define PERF_FINAL call lwperf_finalize()

!! cheat to get around lack of ## in gnu fpp/tradcpp
#define	PerfXPaste(s)	s
#define	PerfPaste(a, b)	PerfXPaste(a)b
!// which logger class are we using
#define PERF FPerf

#define PERFDECL(x) x

!/** This macro set is able to handle overlapping timers by different names. 
! The non-X arguments are used by the file generators.
! E.g. PERFLOG(site1,IP(param1))
! @param X name of timer location; must be unique in application and follow
!the rules of C enum members. For sanity, should be < 32 char long.
!@param ... I*(name) or D*(name) for int/double log data names .
! example: PERFLOG3(SomeLoopNest,IP(maxk),IP(maxj),DP(rtol))
! where maxk, maxj, and rtol are the names to appear in the log.
! the values (the variables they come from) are given in the same order
! and I* or D* macro in the PERFSTOP# call.
!
! These macros are numbered by arg count because fortran preprocessing
! in some compilers does not support macro __VA_ARGS__.
!*/
#define PERFLOG1(X, a ) call lwperf_log(PerfPaste(lwperf_,X))
#define PERFLOG2(X, a,b ) call lwperf_log(PerfPaste(lwperf_,X))
#define PERFLOG3(X, a,b,c ) call lwperf_log(PerfPaste(lwperf_,X))
#define PERFLOG4(X, a,b,c,d ) call lwperf_log(PerfPaste(lwperf_,X))
#define PERFLOG5(X, a,b,c,d,e ) call lwperf_log(PerfPaste(lwperf_,X))
#define PERFLOG6(X, a,b,c,d,e,f ) call lwperf_log(PerfPaste(lwperf_,X))
#define PERFLOG7(X, a,b,c,d,e,f,g ) call lwperf_log(PerfPaste(lwperf_,X))
#define PERFLOG8(X, a,b,c,d,e,f,g,h ) call lwperf_log(PerfPaste(lwperf_,X))
#define PERFLOG9(X, a,b,c,d,e,f,g,h,i ) call lwperf_log(PerfPaste(lwperf_,X))

!/* restart a timer started earlier. rarely needed in fortran*/
#define PERFSTART(X) call lwperf_start(PerfPaste(lwperf_,X))

!/* Compute perf counters and record values. USER values should already be computed.
! VARARGS list should be same as to PERFLOG, e.g. PERFSTOP(site1,IP(param1_value),...)
! but here the values are given to I*()/D*() rather than their names.
! example: PERFSTOP3(SomeLoopNest,IP(nk),IP(nj),DP(max_relative_error))
!*/
#define PERFSTOP1(X, a ) call lwperf_stop(PerfPaste(lwperf_,X)); call PerfPaste(lwperf_save_,X)( a ) 
#define PERFSTOP2(X, a,b ) call lwperf_stop(PerfPaste(lwperf_,X)); call PerfPaste(lwperf_save_,X)( a,b ) 
#define PERFSTOP3(X, a,b,c ) call lwperf_stop(PerfPaste(lwperf_,X)); call PerfPaste(lwperf_save_,X)( a,b,c ) 
#define PERFSTOP4(X, a,b,c,d ) call lwperf_stop(PerfPaste(lwperf_,X)); call PerfPaste(lwperf_save_,X)( a,b,c,d ) 
#define PERFSTOP5(X, a,b,c,d,e ) call lwperf_stop(PerfPaste(lwperf_,X)); call PerfPaste(lwperf_save_,X)( a,b,c,d,e ) 
#define PERFSTOP6(X, a,b,c,d,e,f ) call lwperf_stop(PerfPaste(lwperf_,X)); call PerfPaste(lwperf_save_,X)( a,b,c,d,e,f ) 
#define PERFSTOP7(X, a,b,c,d,e,f,g ) call lwperf_stop(PerfPaste(lwperf_,X)); call PerfPaste(lwperf_save_,X)( a,b,c,d,e,f,g ) 
#define PERFSTOP8(X, a,b,c,d,e,f,g,h ) call PERF_stop(PerfPaste(lwperf_,X)); call PerfPaste(lwperf_save_,X)( a,b,c,d,e,f,g,h ) 
#define PERFSTOP9(X, a,b,c,d,e,f,g,h,i ) call lwperf_stop(PerfPaste(lwperf_,X)); call PerfPaste(lwperf_save_,X)( a,b,c,d,e,f,g,h,i ) 

! D* & I* macros are used at PERFSTOP to store values.
! Their appearance in PERFLOG (which must match PERFSTOP)
! is preprocessed by means other than the preprocessor into 
! properly typed initialization calls as needed.
! Typically, application-level code uses only IP to name integer inputs;
! conceivably, a stopping tolerance could be used with DP, but such use
! is just a poor proxy for some integer iteration count(s) that ought 
! to be logged directly.

#define DR(v) v
#define DD(v) v
#define DN(v) v
#define IR(v) v
#define ID(v) v
#define IN(v) v

#endif
 ! // use csv or eiger

#endif 
 !// fperf_h
