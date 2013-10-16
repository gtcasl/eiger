#ifndef perf_h
#define perf_h

#ifndef __cplusplus
#error "this file is for inclusion in C++ sources only"
#endif

#ifdef _USE_EIGER
#include "eperf.h"
#endif

#ifdef _USE_CSV
#include "aperf.h"
#endif

#ifndef PERF
#define PERF ErRoR
#define PERFFORMATTER NO_LWPERF_ENABLED
#define PERFDECL(...)
#define PERFLOG(X, Y, ...)
#define PERFSTART(X)
#define PERFSTOP(X, ...) 
#define DR(v)
#define DD(v)
#define DN(v)
#define IR(v)
#define ID(v)
#define IN(v)
#endif

#endif // perf_h