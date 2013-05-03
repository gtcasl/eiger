// for case when application defines neither csv nor eiger
#if (!defined(_USE_CSV)&&!defined(_USE_EIGER))
#define _USE_CSV
#endif

#include "lwperf.h"

extern "C" {

void lwperf_init()
{
// wrapper
  PERF::init();
}

void lwperf_finalize() {
// wrapper
  PERF::finalize();
}

void lwperf_mpiArgs(int me, int csize) {
  PERF::mpiArgs(me, csize);
}

void lwperf_fileOptions( const char *host, const char *tools, const char *app,
                        const char *dbname, const char *prefix, const char *suffix)
{
// wrapper
  PERF::stringOptions(host, tools, app, dbname, prefix, suffix);
}

void lwperf_log(int site)
{
  switch(site) {
#include "cperf._log.h"
  default: 
    std::cout <<
    "perflog* macro called with unexpected integer site value " << site <<std::endl;
    throw "perflog* macro called with unexpected integer site value";
  }
}

void lwperf_start(int site)
{
  switch(site) {
#include "cperf._log.h"
  default: 
  std::cout << "perfstart macro called with unexpected integer site value " << site<<std::endl;
  throw "perfstart macro called with unexpected integer site value";
  }
}

void lwperf_stop(int site)
{
  switch(site) {
#include "cperf._stop.h"
  default:
    std::cout << "perflog* macro called with unexpected integer site value " << site <<std::endl;
    throw "perflog* macro called with unexpected integer site value";
  }
}

// generated bit
#include "cperf._save.body.h"

}
