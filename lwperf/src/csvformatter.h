#ifndef csvformatter_h
#define csvformatter_h

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <iostream> // debug only
#include <sys/time.h>
#include <sys/resource.h>
#include <algorithm>

/// only int and double data kinds are supported currently.
enum datakind {
	DR=0, // double result
	DD=4, // double deterministic
	DN=1, // double nondeterministic
	IR=2, // int result
	ID=3, // int deterministic
	IN=10, // int nondeterministic;
};

union data {
	double d;
	long j;
	int i;
	unsigned int u;
	float f;
};
	
/**
See csvformatter.cc for example usage.
Compile -DTEST for simple test.
*/
class csvformatter
{
protected:
	std::string filename;
	std::vector<std::string> head;
	std::vector<enum datakind> htype;
	std::vector<union data> row;
	bool screen;
	FILE *f;
	bool append;
	double t0;
	struct rusage u0;
	struct rusage u1;
	const int mpiRank;
	const int mpiSize;
	bool usetext;

public:
	// when an application knows it no longer needs headers, it can set
	// this to true. downstream calls to writeheaders can be wrapped in if !headersdone
	// set true before using any formatters if all formatters will write to existing files.
	static int headersdone;

protected:
	void ckfile() {
		if (!f && usetext)  {
			f = fopen(filename.c_str(),(append?"a+":"w+"));
		}
	}

public:
	/** Options:
	@param filename output location; not opened until first write.
	@param screen duplicate all output to stdout also?
	@param append if not given true, any existing file is overwritten.
	@param defmetrics if true, constructor is used alone, not part of inheriting class
	*/
	csvformatter(std::string filename, bool screen, int rank, int size, bool append=false, bool defmetrics=true) : filename(filename), screen(screen), f(0), append(append), mpiRank(rank), mpiSize(size)
	{
// perf-counterlike stuff here; each needs matching put in stop and init in start;
// notes: maxrss logged  is the change in maxrss, which is normally expected to be 
// 0 in scientific apps running multiple iterations over the same data.
// nvcsw is how many time we yielded to the kernel, normally 0.
// nivcsw is how many time we were interrupted by scheduler, normally 0.
#define DEFAULT_PERFCTRS \
		addCol("MPIrank", IN); \
		addCol("MPIsize", ID); \
		addCol("stime",DR); \
		addCol("utime",DR); \
		addCol("wtime", DR); \

		usetext = (filename != "");
		if (defmetrics) { 
			DEFAULT_PERFCTRS;
		}
//std::cout << "csvformatter: " << mpiRank << "/" << mpiSize << "arg" << rank<< "/" << size << "\n";
	}

	~csvformatter() { if (f) fclose(f); f=0; }
		
	// uniqueness is not enforced. strings given will be double-quoted in output.
	void addCol(std::string label, enum datakind k) {
//std::cout << "csv addCol " << label << " kind " << k << "\n";
		std::vector< std::string >::iterator i = find( head.begin(), head.end(), label);
		if ( i != head.end()) {
			throw "csvformatter::addCol duplicate column label ";
		}
		head.push_back(label);
		htype.push_back(k);
	}

	// dump headers to a line
	void writeheaders() {
		ckfile();
		if (f) fprintf(f,"\"%s\"\n",filename.c_str());
		std::vector<std::string>::size_type len = head.size();
		for (std::vector<std::string>::size_type j = 0; j < len; j++) {
			if (f) fprintf(f,"\"%s\",",head[j].c_str());
			if (screen) {
				fprintf(stdout,"\"%s\",",head[j].c_str());
			}
		}
		if (f) fprintf(f,"\n");
		if (screen) {
			fprintf(stdout,"\n");
		}
	}

	// accumulate data until row complete
	void put(double d) {
		// fprintf(stderr,"put: %g\n",d);
		if (htype[row.size()] == DR || htype[row.size()] == DD || htype[row.size()] == DN) {
			union data ud;
			ud.d = d;
			row.push_back(ud);
			return;
		}
		assert(0&&"csvformatter::put with double called on non-double column");
	}

	// accumulate data until row complete
	void put(int i) {
		if (htype[row.size()] == IR || htype[row.size()] == ID || htype[row.size()] == IN) {
			union data ud;
			ud.i = i;
			row.push_back(ud);
			return;
		}
		assert(0&&"csvformatter::put with int called on non-int column");
	}

	// dump existing row data if complete
	void nextrow() {
		ckfile();
		std::vector<std::string>::size_type len = head.size();
		assert(row.size() == len && 0 != "csvformatter::nextrow called with incomplete data");
		for (std::vector<std::string>::size_type j = 0; j < len; j++) {
			switch (htype[j]) {
			case DR: // fallthru
			case DD: // fallthru
			case DN:
				if (f) fprintf(f,"%.18g",row[j].d);
				if (screen) {
					fprintf(stdout,"%.18g",row[j].d);
				}
				break;
			case IR: // fallthru
			case ID: // fallthru
			case IN:
				if (f) fprintf(f,"%d",row[j].i);
				if (screen) {
					fprintf(stdout,"%d",row[j].i);
				}
				break;
			default:
				throw "nextrow unhandled datakind";
			} 
			if (f) fprintf(f,", ");
			if (screen) {
				fprintf(stdout,", ");
			}
		}
		if (f) fprintf(f,"\n");
		if (screen) {
			fprintf(stdout,"\n");
		}
		row.clear();
	}

	// set a zero time reference
	void start();
	// compute/store seconds and any other perf counters since last start
	void stop();

	// get underlying file pointer if adding noncsv data
	FILE *file() { return f;}
		
};

#endif // csvformatter_h
