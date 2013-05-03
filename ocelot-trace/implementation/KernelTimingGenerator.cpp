/*!
	\file KernelTimingGenerator.cpp
	\author Andrew Kerr <arkerr@gatech.edu>
	\date Jan 6, 2010; Nov 2011
	\brief records dimensions of launched kernels - note: this assumes trace generators are only
		applied to EmulatedKernel instances
*/

// C++ stdlib includes
#include <fstream>
#include <time.h>

// Ocelot includes
#include <ocelot/ir/interface/PTXOperand.h>
#include <ocelot/executive/interface/ExecutableKernel.h>
#include <ocelot/ir/interface/Module.h>
#include <ocelot/executive/interface/EmulatedKernel.h>
#include <ocelot/trace/interface/TraceEvent.h>
#include <ocelot-trace/interface/KernelTimingGenerator.h>
#include <ocelot/executive/interface/Device.h>

// Hydrazine includes
#include <hydrazine/implementation/Exception.h>

// Eiger includes
#include <api/eiger.h>

//////////////////////////////////////////////////////////////////////////////////////////////////
		
unsigned int trace::KernelTimingGenerator::KernelTimingGenerator::_counter = 0;

trace::KernelTimingGenerator::KernelTimingGenerator(trace::TraceConfiguration *config) : _config(config) {
}
	
trace::KernelTimingGenerator::~KernelTimingGenerator() {
}

void trace::KernelTimingGenerator::initialize(const executive::ExecutableKernel& kernel) {
	_kernelName = kernel.name;
	_kernelModule = kernel.module->path();

	// get device properties
	executive::Device *dev = kernel.device;
	executive::Device::Properties prop = dev->properties();
	machineName = prop.name;

	clock_gettime(CLOCK_REALTIME, &tstart);

}

void trace::KernelTimingGenerator::event( const TraceEvent& event ) {
}
	
void trace::KernelTimingGenerator::finish() {

	clock_gettime(CLOCK_REALTIME, &tend);

	//
  // send to eiger
  //

  // 1) set up DB connection
  eiger::Connect(_config->dbLocation, _config->dbName, _config->dbUsername, _config->dbPassword);
  
  // 2) find/make this data collection
	eiger::DataCollection dc(_config->dataCollection, _config->dataCollectionDesc);
	dc.commit();

  // 3) find/make this machine
  eiger::Machine machine(machineName, "");
	machine.commit();

  // 4) find/make this application
  eiger::Application application(_kernelName, _kernelModule);
	application.commit();

  // 5) find/make this dataset
   eiger::Dataset dataset(application.getID(), _config->datasetName, _config->datasetDesc, _config->datasetURL);
	 dataset.commit();

  // 6) find/make this properties
  // TODO

  // 7) construct new trial
	 eiger::PropertiesID propID;
  eiger::Trial trial(dc.getID(), machine.getID(), application.getID(), dataset.getID(), propID);
	trial.commit();

	// construct execution
	eiger::Execution exec(trial.getID(), machine.getID());
	exec.commit();

	eiger::Metric runtime(eiger::RESULT, "runtime" , "application runtime in seconds");
	runtime.commit();

	eiger::NondeterministicMetric rt(exec.getID(), runtime.getID(), timespec_diff(tstart, tend));
	rt.commit();

	// 8) disconnect from eiger
  eiger::Disconnect();

}

double trace::KernelTimingGenerator::timespec_diff(timespec start, timespec end){

	timespec temp;
	if ((end.tv_nsec - start.tv_nsec) < 0){
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	}
	else {
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}

	return (temp.tv_sec + 0.000000001 * temp.tv_nsec);

}

