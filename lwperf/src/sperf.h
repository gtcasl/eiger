#ifndef sperf_h
#define sperf_h
#include <map>
#include <cassert>
#include <sstmac/software/api/eiger/sstmac_compute_eiger.h>

namespace SkeletonPerf{
	extern int mpirank;
	extern int mpisize;
  void init();
	void mpiArgs(int rank, int size);
	void fileOptions(std::string prefix, std::string suffix);
	void stringOptions(std::string host, std::string tools, std::string application, std::string database, std::string prefix, std::string suffix);
	void finalize();
}

#ifdef PERF_DISABLE
// turn it all off
#define PERF ErRoR
#define PERFDECL(...)
#define PERFLOG(X,  ...)
#define PERFLOGKEEP(X,  ...)
#define PERFSTART(X)
#define PERFSTOP(X, ...) 
#define PERFSTOPKEEP(X, ...) 
#define DR(v)
#define DD(v)
#define DN(v)
#define IR(v)
#define ID(v)
#define IN(v)

#else // no PERF_DISABLE defined

// which logger class are we using
#define PERF SkeletonPerf

/** Wraps statements that should be suppressed if PERF_DISABLE is defined.
// The other PERF macros are automatically suppressed if PERF_DISABLE is defined.
*/
#define PERFDECL(...) __VA_ARGS__
/** This macro is able to handle overlapping timers by different names. 
 It defines a local variable logX. X is a Location enum member, a timer name.
 The variable are used only by the include file generator,
 E.g. PERFLOG(site1,IP(param1))
 @param X name of timer location; must be unique in application and follow
the rules of C enum members.
@param ... I*(name) or D*(name) for int/double log data names .
*/
#define PERFLOGKEEP(X, ...) \
  {std::map<std::string, double> eigerparams; \
  std::string sitename = #X; \
  __VA_ARGS__; \
  eigerparams[sitename + "_" "MPIsize"] = SkeletonPerf::mpisize; \
  eigerparams[sitename + "_" "MPIrank"] = SkeletonPerf::mpirank; \
  SSTMAC_compute_eiger(eigerparams, #X ".model"); }

#define PERFLOG(X, ...) \
  PERFLOGKEEP(X,__VA_ARGS__) \
  if(0){

/** Compute perf counters and record values. USER values should already be computed.
 VARARGS list should be same as to PERFLOG, e.g. PERFSTOP(site1,IP(param1_value),...)
but here the values are given to I*()/D*() rather than their names.
*/
#define PERFSTOPKEEP(X, ...)
#define PERFSTOP(X, ...) }

#define PERFSTART(X)

// D* & I* macros are used at PERFSTOP to store values.
// Their appearance in PERFLOG (which must match PERFSTOP)
// is preprocessed by means other than C preprocessor into initialization calls as needed.
// Typically, application code uses only IP to name integer inputs

#define DR(v) eigerparams[sitename + "_" #v] = v
#define DD(v) eigerparams[sitename + "_" #v] = v
#define DN(v) eigerparams[sitename + "_" #v] = v
#define IR(v) eigerparams[sitename + "_" #v] = v
#define ID(v) eigerparams[sitename + "_" #v] = v
#define IN(v) eigerparams[sitename + "_" #v] = v

#endif // PERF_DISABLE

#endif // sperf_h
