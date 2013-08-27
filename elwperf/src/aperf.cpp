
#include "aperf.h"
#include <cassert>
#include <cstdlib>
#include <sstream>

static Perf *perf_singleton = 0;
void Perf::init() {
	assert(perf_singleton==0);
	char *esalt = getenv("PERF_APPEND");
	bool append = (esalt != 0);
	perf_singleton = new Perf( append);
}
void
Perf::fileOptions(std::string p, std::string s)
{
	assert(perf_singleton!=0);
	if (!perf_singleton) return;
	perf_singleton->prefix = p;
	perf_singleton->suffix = s;
}

void
Perf::stringOptions(std::string host, std::string tools, std::string app, std::string dbname, std::string prefix, std::string suffix)
{
        assert(perf_singleton!=0);
        if (!perf_singleton) return;
        perf_singleton->prefix = prefix;
        perf_singleton->suffix = suffix;

	// FIXME: ensure existence of directory
	// $dbname-$host-$tools/ and then compute and store full prefix=
	// $dbname-$host-$tools/$prefix
	(void)host;
	(void)tools;
	(void)dbname;
	(void)app;
}

void
Perf::mpiArgs(int rank, int size){
	assert(perf_singleton!=0);
	if (!perf_singleton) return;
	perf_singleton->mpirank = rank;
	perf_singleton->mpisize = size;
	perf_singleton->mpiused = true;
}

void 
Perf::finalize() {
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

csvformatter *Perf::Log(enum Location l, std::string filename, bool screen)
{
	assert(perf_singleton!=0);
	if (!perf_singleton) return 0; 
	return perf_singleton->getLog(l, filename, screen);
}
