/*!
	\file MemoryEfficiencyGenerator.cpp
	\author Andrew Kerr <arkerr@gatech.edu>
	\date October 28, 2011
	\brief measures memory efficiency and intensity
*/


// C++ stdlib includes
#include <fstream>

// Ocelot includes
#include <ocelot/ir/interface/PTXInstruction.h>
#include <ocelot/executive/interface/ExecutableKernel.h>
#include <ocelot/ir/interface/Module.h>
#include <ocelot/executive/interface/EmulatedKernel.h>
#include <ocelot/trace/interface/TraceEvent.h>
#include <ocelot/executive/interface/Device.h>


// Hydrazine includes
#include <hydrazine/implementation/Exception.h>
#include <hydrazine/implementation/debug.h>
#include <hydrazine/implementation/macros.h>

// Eiger includes
#include <api/eiger.h>
#include <ocelot-trace/interface/MemoryEfficiencyGenerator.h>

#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 0

//////////////////////////////////////////////////////////////////////////////////////////////////

/*!
	default constructor
*/
trace::MemoryEfficiencyGenerator::MemoryEfficiencyGenerator(trace::TraceConfiguration *config) : _config(config), trialCounter(0) {

}

/*!
	\brief destructs instruction trace generator
*/
trace::MemoryEfficiencyGenerator::~MemoryEfficiencyGenerator() {

}

/*!
	\brief called when a traced kernel is launched to retrieve some 
		parameters from the kernel
*/
void trace::MemoryEfficiencyGenerator::initialize(const executive::ExecutableKernel& _kernel) {

	// get device properties
	executive::Device::Properties prop = _kernel.device->properties();
	machineName = prop.name;
	warpSize = prop.SIMDWidth;

	_kernelName = _kernel.name;
	_kernelModule = _kernel.module->path();

	//kernel = static_cast<const executive::EmulatedKernel *>(&_kernel);
	clear();
}

/*!
	\brief Called whenever an event takes place.

	Note, the const reference 'event' is only valid until event() 
	returns
*/
void trace::MemoryEfficiencyGenerator::event(const TraceEvent & event) {

	warpInstructions++;

	switch (event.instruction->opcode) {
		case ir::PTXInstruction::St:
		case ir::PTXInstruction::Ld:
			accessCounterMap[event.instruction->addressSpace].access(warpSize, event);
			break;
		
		case ir::PTXInstruction::Tex:
			accessCounterMap[event.instruction->addressSpace].access(warpSize, event);
			break;
		default:
			break;
	}
}

/*! 
	\brief Called when a kernel is finished. There will be no more 
		events for this kernel.
*/
void trace::MemoryEfficiencyGenerator::finish() {


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
  
	eiger::TrialID trialID;
	trialID = _config->startTrial + trialCounter;
	/*
  // 7) construct new trial
  eiger::Trial trial(dc.getID(), machine.getID(), application.getID(), dataset.getID(), NULL);
	trial.commit();
	*/

		eiger::Execution exec(trialID, machine.getID());
		exec.commit();
  
	eiger::Metric efficiencyMetric(eiger::NONDETERMINISTIC, "MemoryEfficiency", "memory efficiency");
	efficiencyMetric.commit();
	eiger::Metric intensityMetric(eiger::NONDETERMINISTIC, "MemoryIntensity", "memory intensity");
	intensityMetric.commit();
	eiger::Metric transferMetric(eiger::NONDETERMINISTIC, "MemoryTransfer", "memory transfer");
	transferMetric.commit();
	
	// Consider global and texture accesses only
	AccessCounter totalOffchip = accessCounterMap[ir::PTXInstruction::Global];
	totalOffchip += accessCounterMap[ir::PTXInstruction::Texture];
	
	double res;
	res = (totalOffchip.transactions != 0) ? ((double)totalOffchip.accesses / (double)totalOffchip.transactions) : 0;
	eiger::NondeterministicMetric memoryEfficiency(exec.getID(), efficiencyMetric.getID(), res);
	memoryEfficiency.commit();
		
	res = (warpInstructions != 0) ? ((double)totalOffchip.accesses / (double)warpInstructions) : 0;
	eiger::NondeterministicMetric memoryIntensity(exec.getID(), intensityMetric.getID(), res);
	memoryIntensity.commit();
		
	eiger::NondeterministicMetric memoryTransfer(exec.getID(), transferMetric.getID(), totalOffchip.bytes);
	memoryTransfer.commit();

  eiger::Disconnect();
  trialCounter++;
  
}


void trace::MemoryEfficiencyGenerator::clear() {
	accessCounterMap.clear();
	
	ir::PTXInstruction::AddressSpace spaces[] = {
		ir::PTXInstruction::Const,
		ir::PTXInstruction::Global,
		ir::PTXInstruction::Local,
		ir::PTXInstruction::Param,
		ir::PTXInstruction::Shared,
		ir::PTXInstruction::Texture,
		ir::PTXInstruction::Generic,
		ir::PTXInstruction::AddressSpace_Invalid
	};
	size_t sizes[] = {
		64,
		64,
		64,
		64,
		64,
		64,
		64,
		0
	};
	for (int i = 0; spaces[i] != ir::PTXInstruction::AddressSpace_Invalid; i++) {
		accessCounterMap[spaces[i]] = AccessCounter(sizes[i]);
	}
	warpInstructions = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void trace::MemoryEfficiencyGenerator::AccessCounter::access(int warpSize, 
	const trace::TraceEvent &event) {
	
	int ctaSize = event.blockDim.size();
	
	if (event.instruction->addressSpace == ir::PTXInstruction::Texture) {
		this->accesses += 1;
		this->transactions += 1;
		this->bytes += event.memory_size;
	}
	else {
		trace::TraceEvent::U64Vector::const_iterator addressIterator = event.memory_addresses.begin();
	
		std::map< ir::PTXU64 , int > blockAccesses;
		ir::PTXU64 mask = ~(transactionSize - 1);
	
		for (int tid = 0; tid < ctaSize; tid++) {
			if (event.active[tid]) {
				ir::PTXU64 baseAddress = (*addressIterator & mask);
				blockAccesses[baseAddress] ++;
				++addressIterator;
			}
			if ((tid % warpSize) == ctaSize - 1) {
				// last warp
				this->accesses += 1;
				this->transactions += blockAccesses.size();
				this->bytes += blockAccesses.size() * transactionSize;
			
				blockAccesses.clear();
			}		
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

