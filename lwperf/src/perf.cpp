#include <cassert>
#include <cstdlib>
#include <sstream>

#include "perf.h"

static Perf *perf_singleton = 0;
void Perf::init(std::string machine, std::string app, std::string dbname, 
                std::string prefix, std::string suffix) {
	//assert(perf_singleton==0);
  if(perf_singleton != 0) return;
        const char *esalt = getenv("PERF_APPEND");
        bool append = (esalt != 0);
	perf_singleton = new Perf(machine, app, prefix, suffix, append );

  std::string dbfile = "127.0.0.1";
  std::string uname = "root";
  std::string pw = "root";
	// in mpi-parallel, we will pound the database
#if defined(_USE_EIGER) || defined(_USE_FAKEEIGER)
	eiger::Connect(dbfile, dbname, uname, pw );
#endif
}

void
Perf::mpiArgs(int rank, int size){
	assert(perf_singleton!=0);
	if (!perf_singleton) return;
	perf_singleton->mpirank = rank;
	perf_singleton->mpisize = size;
	perf_singleton->mpiused = true;
  getInvariants()["MPIsize"] = size;
}

void 
Perf::finalize() {
  if(perf_singleton==0) return;
	std::map<enum Location, formatter<PERFBACKEND> *>::iterator it = perf_singleton->log.begin();
	while (it != perf_singleton->log.end() ) {
		delete it->second;
		 perf_singleton->log[it->first] = 0;
		it++;
	}
#if defined(_USE_EIGER) || defined(_USE_FAKEEIGER)
	eiger::Disconnect();
#endif
	assert(perf_singleton!=0);
	delete perf_singleton; 
	perf_singleton = 0;
}

formatter<PERFBACKEND> *Perf::Log(enum Location l, std::string filename)
{
	assert(perf_singleton!=0);
	if (!perf_singleton) return 0; 
	return perf_singleton->getLog(l, filename);
}

Perf::~Perf() {
  for (std::map<enum Location, formatter<PERFBACKEND> *>::iterator i = log.begin(); 
      i != log.end(); i++) {
    delete i->second;
  }
}

formatter<PERFBACKEND> *Perf::getLog(enum Location l, std::string filename) {
  formatter<PERFBACKEND> *cf = log[l];
  if (!cf) {
    formatter<PERFBACKEND> * ncf = new formatter<PERFBACKEND>(filename, machine, app, getInvariants());
    switch (l) {
    case X: initX(ncf); break;
#ifndef PERF_DISABLE
#include "InitSwitchElements.h"
#endif
    default:
      assert(0 == "unexpected Location given to PERFLOG");
      break;
    }
    if(!append) { ncf->writeheaders(); }
    log[l] = ncf;
  } else {
    return cf;
  }
  cf = log[l];
  return cf;
}

