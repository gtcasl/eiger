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
// normal usage fp types are measuremnents and int types are inputs
	D=0, // double nonparametric
	F=4, // float nonparametric
	I=1, // int parametric
	U=2, // uint parametric
	L=3, // long parametric
// the 'nonusual' case have 2 letter names; fp types are parametric and int types are non parametric
	PD=10, // parametric double;
	PF=14, // parametric float
	NI=11, // nonparametric int (not used in dataset name construction)
	NU=12, // nonparametric unsigned
	NL=13, // nonparametric long
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
		addCol("delmaxrss",NL); \
		addCol("minflt",NL); \
		addCol("majflt",NL); \
		addCol("nvcsw",NL); \
		addCol("nivcsw",NL); \
		addCol("appclock", D) ; \
		addCol("MPIrank", NI); \
		addCol("MPIsize", I); \
		addCol("stime",D); \
		addCol("utime",D); \
		addCol("wtime", D); \

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
		if (htype[row.size()] == D || htype[row.size()] == PD) {
			union data ud;
			ud.d = d;
			row.push_back(ud);
			return;
		}
		assert(0&&"csvformatter::put with double called on non-double column");
	}
	// accumulate data until row complete
	void put(float f) {
		// fprintf(stderr,"put: %g\n",d);
		if (htype[row.size()] == F || htype[row.size()] == PF) {
			union data ud;
			ud.f = f;
			row.push_back(ud);
			return;
		}
		assert(0&&"csvformatter::put with float called on non-float column");
	}

	// accumulate data until row complete
	void put(int i) {
		if (htype[row.size()] == I || htype[row.size()] == NI) {
			union data ud;
			ud.i = i;
			row.push_back(ud);
			return;
		}
		assert(0&&"csvformatter::put with int called on non-int column");
	}

	// accumulate data until row complete
	void put(long j) {
		if (htype[row.size()] == L || htype[row.size()] == NL) {
			union data ud;
			ud.j = j;
			row.push_back(ud);
			return;
		}
		assert(0&&"csvformatter::put with long called on non-long column");
	}

	// accumulate data until row complete
	void put(unsigned u) {
		// fprintf(stderr,"put: %u\n",u);
		if (htype[row.size()] == U || htype[row.size()] == NU) {
			union data ud;
			ud.u = u;
			row.push_back(ud);
			return;
		}
		assert(0&&"csvformatter::put with unsigned called on non-unsigned column");
	}

	// dump existing row data if complete
	void nextrow() {
		ckfile();
		std::vector<std::string>::size_type len = head.size();
		assert(row.size() == len && 0 != "csvformatter::nextrow called with incomplete data");
		for (std::vector<std::string>::size_type j = 0; j < len; j++) {
			switch (htype[j]) {
			case F: // fallthru
			case PF:
				if (f) fprintf(f,"%.9f",row[j].f);
				if (screen) {
					fprintf(stdout,"%.9f",row[j].f);
				}
				break;
			case D: // fallthru
			case PD:
				if (f) fprintf(f,"%.18g",row[j].d);
				if (screen) {
					fprintf(stdout,"%.18g",row[j].d);
				}
				break;
			case NI: // fallthru
			case I:
				if (f) fprintf(f,"%d",row[j].i);
				if (screen) {
					fprintf(stdout,"%d",row[j].i);
				}
				break;
			case L: // fallthru
			case NL:
				if (f) fprintf(f,"%ld",row[j].j);
				if (screen) {
					fprintf(stdout,"%ld",row[j].j);
				}
				break;
			case NU: // fallthru
			case U:
				if (f) fprintf(f,"%u",row[j].i);
				if (screen) {
					fprintf(stdout,"%u",row[j].i);
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
