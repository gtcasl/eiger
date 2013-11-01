#ifndef cperf_h_seen
#define cperf_h_seen

#ifndef __STDC__
#error "This file is for use with C compilers only"
#else
#if (__STDC__==0)
#error "This file is for use with C compilers only, not c++."
#endif
#endif

/* this file must match fperf.F90 bind c requirements also */
extern void lwperf_init(const char *machine, const char *app, const char *db, const char *prefix, const char *suffix);
extern void lwperf_finalize();
extern void lwperf_mpiArgs(int rank, int size);
extern void lwperf_log(int site);
extern void lwperf_start(int site);
extern void lwperf_stop(int site);

enum CLocation {
	X, // dummy for example
#ifndef PERF_DISABLE
#include "clocations.h"
#endif
};

#if (!defined(_USE_CSV) && !defined(_USE_EIGER))
// turn it all off
#define PERF Perf

#define PERF_INIT
#define PERF_MPI(a,b)
#define PERF_OPTS(...)
#define PERF_FINAL

#define PERFLOG(X, ... )
#define PERFSTOP(X, ... ) 

#define DR(v)
#define DD(v)
#define DN(v)
#define IR(v)
#define ID(v)
#define IN(v)

#else
// _USE_CSV defined
#include "cperf._save.h"

/// macro to wrap declarations used only for perf logging.
#define PERFDECL(...) __VA_ARGS__

#define PERF_INIT(machine,app,db,prefix,suffix) lwperf_init(machine,app,db,prefix,suffix)
#define PERF_MPI(rank,size) lwperf_mpiArgs(rank,size)
#define PERF_FINAL lwperf_finalize()

// which logger class are we using
#define PERF CPerf

/** This macro set is able to handle overlapping timers by different names. 
 E.g. PERFLOG(site1,IP(param1))
 @param X name of timer location; must be unique in application and follow
the rules of C enum member names.
@param ... I*(name) or D*(name) for int/double log data names .
*/
#define PERFLOG(X, ... ) lwperf_log(lwperf_ ## X)

/* Restart a timer declared earlier. */
#define PERFSTART(X) lwperf_start(lwperf_ ## X)

/* Compute perf counters and record values. USER values should already be computed.
 VARARGS list should be same as to PERFLOG, e.g. PERFSTOP(site1,IP(param1_value),...)
 but here the values are given to I*()/D*() rather than their names.
 example: PERFSTOP(SomeLoopNest,IP(nk),ID(nj),DR(max_relative_error))
*/
#define PERFSTOP(X, ... ) lwperf_stop(lwperf_ ## X); lwperf_save_##X( __VA_ARGS__ ) 

/*
 D* & I* macros are used at PERFSTOP to store values.
 Their appearance in PERFLOG (which must match PERFSTOP)
 is preprocessed by means other than the preprocessor into 
 properly typed initialization calls as needed.
 Typically, application-level code uses only I* to name integer inputs
 conceivably, a stopping tolerance could be used with D*, but such use
 is just a poor proxy for some integer iteration count(s) that ought 
 to be logged directly.
*/
#define DR(v) v
#define DD(v) v
#define DN(v) v
#define IR(v) v
#define ID(v) v
#define IN(v) v

#endif // not PERF_DISABLE

#endif /*cperf_h_seen*/
