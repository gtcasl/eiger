
#include "sperf.h"
#include <cassert>
#include <cstdlib>
#include <sstream>

namespace SkeletonPerf {
	int mpirank;
	int mpisize;

	void init() {}
	void mpiArgs(int rank, int size){mpirank = rank; mpisize=size;}
	void fileOptions(std::string prefix, std::string suffix) {}
	void stringOptions(std::string host, std::string tools, std::string application, std::string database, std::string prefix, std::string suffix) {}
	void finalize() {}
	// The rest of the perf user interface is the
  // PERFDECL, PERFLOG, PERFSTOP, and optional PERFSTART at bottom.
}

