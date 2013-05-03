#ifndef diffrusage_h
#define diffrusage_h

#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>


double timeval_subtract ( struct timeval *x /*end*/ , struct timeval *y /*begin*/);

double tdiff(timespec start, timespec end);

struct diffrusage_t {
	double utime;
	double stime;
	long int maxrss;
	long int minflt;
	long int majflt;
	long int nvcsw;
	long int nivcsw;
};

struct diffrusage_t diffrusage(struct rusage *end, struct rusage *start);

#endif // diffrusage_h
