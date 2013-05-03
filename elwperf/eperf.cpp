
#include "eperf.h"
#include <cassert>
#include <cstdlib>
#include <sstream>

static EigerPerf *perf_singleton = 0;
void EigerPerf::init() {
	assert(perf_singleton==0);
        char *esalt = getenv("PERF_APPEND");
        bool append = (esalt != 0);
	perf_singleton = new EigerPerf(append );
}

void
EigerPerf::stringOptions(std::string host, std::string tools, std::string app, std::string dbname, std::string prefix, std::string suffix)
{
	assert(perf_singleton!=0);
	if (!perf_singleton) return;
	perf_singleton->prefix = prefix;
	perf_singleton->suffix = suffix;

        std::string machineName = host;

        std::string dbfile = "127.0.0.1";
        std::string uname = "root";
        std::string pw = "eiger123";

	// in mpi-parallel, we will pound the database
	eiger::Connect(dbfile, dbname, uname, pw );

	perf_singleton->epec = new eigercontext(app, machineName, tools);

}

void
EigerPerf::fileOptions(std::string p, std::string s)
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
	eiger::Connect(dbfile, dbname, uname, pw );

	perf_singleton->epec = new eigercontext(app, machineName, tools);

}
void
EigerPerf::mpiArgs(int rank, int size){
	assert(perf_singleton!=0);
	if (!perf_singleton) return;
	perf_singleton->mpirank = rank;
	perf_singleton->mpisize = size;
	perf_singleton->mpiused = true;
}

void 
EigerPerf::finalize() {
	std::map<enum Location, eigerformatter *>::iterator it = perf_singleton->log.begin();
	while (it != perf_singleton->log.end() ) {
		delete it->second;
		 perf_singleton->log[it->first] = 0;
		it++;
	}
	eiger::Disconnect();
	assert(perf_singleton!=0);
	delete perf_singleton; 
	perf_singleton = 0;
}

std::string
EigerPerf::makeFileName(std::string & filename) 
{
	std::stringstream s;
	s << prefix << filename << "_mpi_" << mpisize << "." << mpirank <<suffix;
	return s.str();
}

eigerformatter *EigerPerf::Log(enum Location l, std::string filename, bool screen)
{
	assert(perf_singleton!=0);
	if (!perf_singleton) return 0; 
	return perf_singleton->getLog(l, filename, screen);
}
