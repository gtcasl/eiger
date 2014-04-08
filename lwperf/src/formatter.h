#ifndef formatter_h
#define formatter_h

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <iostream> // debug only
#include <time.h>
#include <sys/resource.h>
#include <algorithm>
#include <map>
#include <string>

#include "datakind.h"

namespace {
const long kNsecsPerSec = 1000000000;
double get_elapsed(const timespec& start_time, const timespec& stop_time) {
  double res = 0.0;
  if (stop_time.tv_nsec < start_time.tv_nsec) {
    res += (kNsecsPerSec + stop_time.tv_nsec - start_time.tv_nsec) /
           (double)kNsecsPerSec;
    res += stop_time.tv_sec - start_time.tv_sec - 1;
  } else {
    res += (stop_time.tv_nsec - start_time.tv_nsec) / (double)kNsecsPerSec;
    res += stop_time.tv_sec - start_time.tv_sec;
  }
  return res;
}
}

template <typename Backend>
class formatter
{
protected:
  Backend backend_;
  std::vector<std::pair<std::string, enum datakind> > headers;
	std::vector<double> row;
  timespec start_time;

public:
  std::map<std::string,double> invariants;

	/** Options:
	@param filename output location; not opened until first write.
	@param screen duplicate all output to stdout also?
	@param append if not given true, any existing file is overwritten.
	@param defmetrics if true, constructor is used alone, not part of inheriting class
	*/
	formatter(std::string filename, std::string machine, 
            std::map<std::string,double> invariants, bool append=false)
    : backend_(filename, machine, append), invariants(invariants)
  {
    addCol("time", RESULT);
    for(std::map<std::string,double>::const_iterator it = invariants.begin();
        it != invariants.end(); ++it){
      addCol(it->first, DETERMINISTIC);
    }
  }

	// uniqueness is not enforced. strings given will be double-quoted in output.
	void addCol(std::string label, enum datakind k)
  {
    bool found = false;
    typedef std::vector<std::pair<std::string, enum datakind> > pair_vec;
    for(pair_vec::const_iterator it = headers.begin(); it != headers.end(); ++it){
      if(it->first == label){
        found = true;
      }
    }
    if(found){
      throw "formatter::addCol duplicate column label ";
    }
    headers.push_back(std::make_pair(label, k));
    backend_.addCol(label, k);
  }

	// dump headers to a line
	void writeheaders()
  {
    backend_.writeheaders(headers);
  }

	// accumulate data until row complete
	void put(double d)
  {
    row.push_back(d);
  }

	// dump existing row data if complete
	void nextrow()
  {
    backend_.nextrow(headers, row);
    row.clear();
  }

	// set a zero time reference
	void start()
  {
    double res = clock_gettime(CLOCK_MONOTONIC, &start_time);
    if(res != 0){
      std::cerr << "Unable to get current time." << std::endl;
      exit(-1);
    }
  }
	// compute/store seconds and any other perf counters since last start
	void stop()
  {
    timespec stop_time;
    clock_gettime(CLOCK_MONOTONIC, &stop_time);
    double elapsed = get_elapsed(start_time, stop_time);
    put(elapsed);
    for(std::map<std::string,double>::const_iterator it = invariants.begin();
        it != invariants.end(); ++it){
      put(it->second);
    }
  }
};

#endif // formatter_h
