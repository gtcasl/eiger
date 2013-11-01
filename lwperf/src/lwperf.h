#ifndef lwperf_h
#define lwperf_h

#ifndef __cplusplus
#error "this file is for inclusion in C++ sources only"
#endif

#if !defined(PERF_DISABLE) || defined(_USE_EIGER) || defined(_USE_CSV) || \
    defined(_USE_EIGER_MODEL) || defined(_USE_FAKEEIGER)

#include "perf.h"

#else // PERF_DISABLE is set
#define PERF ErRoR
#define PERFFORMATTER NO_LWPERF_ENABLED
#define PERFDECL(...)
#define PERFLOG(X, ...)
#define PERFLOGKEEP(X, ...)
#define PERFSTART(X)
#define PERFSTOP(X, ...) 
#define PERFSTOPKEEP(X, ...) 
#define REGISTERINVARIANT(name,value)
#define DR(v)
#define DD(v)
#define DN(v)
#define IR(v)
#define ID(v)
#define IN(v)
#endif // PERF_DISABLE

#endif // lwperf_h
