#include <cassert>
#include <cstdlib>
#include <sstream>

#include "perf.h"

static Perf *perf_singleton = 0;
void Perf::init() {
	assert(perf_singleton==0);
        char *esalt = getenv("PERF_APPEND");
        bool append = (esalt != 0);
	perf_singleton = new Perf(append );
}

void
Perf::stringOptions(std::string host, std::string tools, std::string app, std::string dbname, std::string prefix, std::string suffix)
{
	assert(perf_singleton!=0);
	if (!perf_singleton) return;
	perf_singleton->prefix = prefix;
	perf_singleton->suffix = suffix;
	perf_singleton->machine = host;

  std::string dbfile = "127.0.0.1";
  std::string uname = "root";
  std::string pw = "root";
	// in mpi-parallel, we will pound the database
#if defined(_USE_EIGER) || defined(_USE_FAKEEIGER)
	eiger::Connect(dbfile, dbname, uname, pw );
#endif
}

void
Perf::fileOptions(std::string p, std::string s)
{
	assert(perf_singleton!=0);
	if (!perf_singleton) return;
	perf_singleton->prefix = p;
	perf_singleton->suffix = s;

        std::string machineName = p;
#ifdef FORTRAN
#define APP "app"
        std::string app = APP;
        std::string tools = "gcc";
        std::string dbname = "fortapp";
#endif
#ifdef CAPP
#define APP "capp"
        std::string app = APP;
        std::string tools = "gcc";
        std::string dbname = "capp";
#endif
#ifndef APP
        std::string app = "minimd-0.1";
        std::string tools = "intel";
        std::string dbname = "minimd";
#endif
        std::string dbfile = "127.0.0.1";
        std::string uname = "root";
        std::string pw = "eiger123";
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
  getInvariants()["MPIrank"] = rank;
  getInvariants()["MPIsize"] = size;
}

void 
Perf::finalize() {
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

std::string
Perf::makeFileName(std::string & filename) 
{
	std::stringstream s;
	s << prefix << filename << "_mpi_" << mpisize << "." << mpirank <<suffix;
	return s.str();
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
    formatter<PERFBACKEND> * ncf = new formatter<PERFBACKEND>(filename, machine, getInvariants());
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

