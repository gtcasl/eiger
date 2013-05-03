
#include "diffrusage.h"

double timeval_subtract ( struct timeval *x /*end*/ , struct timeval *y /*begin*/)
{
	struct timeval r, *result; double dt;
	result = &r;
       /* Perform the carry for the later subtraction by updating y. */
       if (x->tv_usec < y->tv_usec) {
         int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
         y->tv_usec -= 1000000 * nsec;
         y->tv_sec += nsec;
       }
       if (x->tv_usec - y->tv_usec > 1000000) {
         int nsec = (x->tv_usec - y->tv_usec) / 1000000;
         y->tv_usec += 1000000 * nsec;
         y->tv_sec -= nsec;
       }
     
       /* Compute the time remaining to wait.
          tv_usec is certainly positive. */
       result->tv_sec = x->tv_sec - y->tv_sec;
       result->tv_usec = x->tv_usec - y->tv_usec;
	dt=result->tv_sec+1e-6*result->tv_usec;
     
	return dt;
}

double tdiff(timespec start, timespec end)
{
        timespec temp;
        if ((end.tv_nsec-start.tv_nsec)<0) {
                temp.tv_sec = end.tv_sec-start.tv_sec-1;
                temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
        } else {
                temp.tv_sec = end.tv_sec-start.tv_sec;
                temp.tv_nsec = end.tv_nsec-start.tv_nsec;
        }
        return temp.tv_sec +1.0e-9*temp.tv_nsec;
}

struct diffrusage_t diffrusage(struct rusage *end, struct rusage *start)
{
	struct diffrusage_t d;
	d.utime = timeval_subtract(&(end->ru_utime), &(start->ru_utime));
	d.stime = timeval_subtract(&(end->ru_stime), &(start->ru_stime));
	d.maxrss = end->ru_maxrss - start->ru_maxrss;
	d.minflt = end->ru_minflt - start->ru_minflt;
	d.majflt = end->ru_majflt - start->ru_majflt;
	d.nvcsw = end->ru_nvcsw - start->ru_nvcsw;
	d.nivcsw = end->ru_nivcsw - start->ru_nivcsw;
	return d;
}

