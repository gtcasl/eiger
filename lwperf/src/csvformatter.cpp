#ifndef _USE_EIGER_MODELS
#include <mpi.h>
#include "csvformatter.h"
#include "diffrusage.h"

int csvformatter::headersdone=0;

void
csvformatter::start() {
	memset(&u0,0,sizeof(struct rusage ));
	memset(&u1,0,sizeof(struct rusage )); // don't include in timed region
	t0 = MPI_Wtime(); 
	getrusage(RUSAGE_SELF, &u0);
}

void
csvformatter::stop() {
	getrusage(RUSAGE_SELF,&u1);
	double stamp = MPI_Wtime();
	double dt = stamp - t0;
	struct diffrusage_t d = diffrusage(&u1, &u0); // compute deltas from t0 to now.
	// keep these in order sync with DEFAAULT_PERFCTRS  macro
	put(mpiRank);
	put(mpiSize);
	put(d.stime); 
	put(d.utime); 
	put(dt);

//std::cout << "csvformatter:stop: " << mpiRank << "/" << mpiSize <<  "\n";
}

#ifdef TEST
void do2() {
	csvformatter log("formattest.log", false, true);
	log.addCol("apples",D);
	log.addCol("oranges",I);
	log.put(2.0);
	log.put(3);
	log.nextrow();
}
void do1() {
	csvformatter log("formattest.log", true);
	log.addCol("apples",D);
	log.addCol("oranges",I);
	log.writeheaders();
	log.put(2.0);
	log.put(3);
	log.nextrow();
	fprintf(log.file(),"oneliner\n");

}
int main() {
	do1();
	do2();
	return 0;
}
#endif // test
#endif // _USE_EIGER_MODELS
