#ifndef eigerformatter_h
#define eigerformatter_h

#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#ifdef _USE_FAKEEIGER 
#include "mpifakeeiger.h"
#else
#include "eiger.h"
#endif
#include "csvformatter.h"

class eigercontext {
public:
	eigercontext(std::string app, std::string machine_, std::string tools)
	{ 
		application = new eiger::Application(app, tools);
		application->commit();
		machine = new eiger::Machine(machine_, "nARCH");
		machine->commit();
	}

	~eigercontext()
	{
		delete application; application = 0;
		delete machine; machine = 0;
	}

	eiger::Machine *machine;
	eiger::Application *application;
};
	
/**
See eigerformatter.cc for example usage.
Compile -DTEST for simple test.
This provides eiger and text logging, with turning off the text and screen optional.
*/
class eigerformatter : public csvformatter
{
private:

	// base: std::vector<std::string> head;
	// base: std::vector<enum datakind> htype;
	// base: std::vector< union data > row;
	// base: bool screen;
	// base: FILE * f;
	// base: bool append;
	// base: double t0;
        // base: const int mpiRank;
        // base: const int mpiSize;

	eigercontext *ec;
	std::string sitename;
	std::vector< eiger::Metric *> erow;
	eiger::DataCollection *dc;
	// perf-counterlike stuff here
	// base: bool usetext;

public:
	// when an application knows it no longer needs headers, it can set
	// this to true. downstream calls to writeheaders can be wrapped in if !headersdone
	// set true before using any formatters if all formatters will write to existing files.
	// base: static int headersdone;

public:
	/** Options:
	@param filename name of logging file, or empty string ("") if no text log wanted.
		normally derived from sitename somehow.
	@param screen duplicate all output to stdout also?
	@param rank parallel (mpi) process rank
	@param size parallel (mpi) process size
	@param append write to existing text file or restart text file; no eiger effect.
	@param sitename name of profiling block for eiger; must be unique in app.
	*/
	eigerformatter(std::string filename, bool screen, int rank, int size, bool append, std::string sitename, eigercontext *ec) : csvformatter(filename, screen, rank, size, append, false), ec(ec), sitename(sitename)
	{
		usetext = (filename != "");
//std::cout << "eigerformatter: " << rank << "/" << size <<"\n";
		dc = new eiger::DataCollection(sitename,sitename);
		dc->commit();
		
		DEFAULT_PERFCTRS;

	}

	~eigerformatter() { 
		delete dc; dc=0; 
		if (f) fclose(f); f=0;
	}
		
	// uniqueness is enforced. strings given will become name and desc
	void addCol(std::string label, enum datakind k) {
		std::string slabel = sitename + "_"+label;
		csvformatter::addCol(label,k);
		int j = erow.size();
//std::cout << "addCol j="<< j<< " label= " << label << std::endl;
		switch (htype.at(j)) {
		case DR:
		case IR:
			erow.push_back(new eiger::Metric(eiger::RESULT, slabel, slabel));
			break;
		case DD:
		case ID:
			erow.push_back(new eiger::Metric(eiger::DETERMINISTIC, slabel, slabel));
			break;
		case DN:
		case IN:
			erow.push_back(new eiger::Metric(eiger::NONDETERMINISTIC, slabel, slabel));
			break;
		default:
			throw "addCol unhandled datakind";
		} 
		(erow.at(j))->commit();
	}

	// dump headers to a line
	// base: void writeheaders() {

	// accumulate data until row complete
	// base: void put(double d) 

	// accumulate data until row complete
	// base: void put(int i) ;

	// accumulate data until row complete
	// base: void put(unsigned u) ;

	// dump existing row data if complete
	void nextrow() {
		// declare a trial, exec, etc and do all commits, then 
		// do text since text handles row.clear().
		std::ostringstream dsbuf; 
		std::ostringstream ddbuf; 
		dsbuf << sitename;
		ddbuf << sitename;
//		std::cout << "LOGGING @ " << sitename <<"\n";
		std::vector<std::string>::size_type len = head.size();
		assert(row.size() == len && 0 != "eigerformatter::nextrow called with incomplete data");
		assert(htype.size() == len && 0 != "eigerformatter::nextrow called with confused htype");
		for (std::vector<std::string>::size_type j = 0; j < len; j++) {
			switch (htype.at(j)) {
			case DR: // fallthru
			case DN: // fallthru
			case IR: // fallthru
			case IN: // fallthru
				break;
			case ID:
				dsbuf << "_" << row.at(j).i;
				ddbuf << " " << head.at(j) << "=" <<row.at(j).i;
				break;
			case DD:
				dsbuf << "_" << row.at(j).d;
				ddbuf << " " << head.at(j)<<  "=" <<row.at(j).d;
				break;
			default:
				throw "unhandled htype in nextrow";
			} 
		} 
		std::string dsname = dsbuf.str();
		std::string dsdesc = ddbuf.str();

		eiger::ApplicationID aid = ec->application->getID();
		eiger::Dataset ds(aid, dsname, dsdesc, "nURL");
		ds.commit();
		eiger::DatasetID dsid = ds.getID();

		eiger::MachineID machid = ec->machine->getID();
		eiger::Trial trial(dc->getID(), machid, aid, dsid, eiger::PropertiesID(0,0));
		trial.commit();
		eiger::Execution exec(trial.getID(),machid);
		exec.commit();
		eiger::ExecutionID eid = exec.getID();
		for (std::vector<std::string>::size_type j = 0; j < len; j++) {
			eiger::MetricID mid = (erow.at(j))->getID();
			switch (htype.at(j)) {
			case DD: {
				eiger::DeterministicMetric dmD(dsid, mid, row.at(j).d);
				dmD.commit();
				break;
				}
			case DN: {
				eiger::NondeterministicMetric ndmD(eid, mid, row.at(j).d);
				ndmD.commit();
				break;
				}
			case DR: {
				eiger::NondeterministicMetric rdmD(eid, mid, row.at(j).d);
				rdmD.commit();
				break;
				}
			case IN:
				{
				eiger::NondeterministicMetric ndmI(eid, mid, (double)row.at(j).i);
				ndmI.commit();
				break;
				}
			case ID:
				{
				eiger::DeterministicMetric dmI(dsid, mid, (double)row.at(j).i);
				dmI.commit();
				break;
				}
			case IR: {
				eiger::NondeterministicMetric rdmI(eid, mid, (double)row.at(j).d);
				rdmI.commit();
				break;
				}
			default:
				throw "unhandled htype in nextrow";
			} 
		}
		csvformatter::nextrow();
	}

	// set a zero time reference
	// base: void start();
	// return seconds since last start
	// base: void stop() {

};

#endif // eigerformatter_h
