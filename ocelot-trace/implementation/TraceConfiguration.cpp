/*! \file   TraceConfiguration.cpp
	\author Andrew Kerr <arkerr@gatech.edu>, Gregory Diamos <gregory.diamos@gatech.edu>
	\date   September 7, 2011;  July 31, 2010
	\brief  The source file for the TraceConfiguration class.
*/

// Trace includes
#include <ocelot-trace/interface/TraceConfiguration.h>
#include <ocelot-trace/interface/InstructionTraceGenerator.h>
#include <ocelot-trace/interface/KernelDimensionsGenerator.h>
#include <ocelot-trace/interface/KernelParametersGenerator.h>
#include <ocelot-trace/interface/KernelTimingGenerator.h>
#include <ocelot-trace/interface/MachineAttributesGenerator.h>
#include <ocelot-trace/interface/MemoryEfficiencyGenerator.h>
#include <ocelot-trace/interface/BranchTraceGenerator.h>
#include <ocelot-trace/interface/ParallelismTraceGenerator.h>

// Ocelot includes
#include <ocelot/api/interface/ocelot.h>
#include <ocelot/api/interface/OcelotConfiguration.h>

// Hydrazine includes
#include <hydrazine/implementation/Exception.h>
#include <hydrazine/implementation/debug.h>
#include <hydrazine/implementation/json.h>

#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 0

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace trace
{

TraceConfiguration *TraceConfiguration::Singleton;



/*!
	\brief constructs and initializes trace generators
*/
TraceConfiguration::TraceConfiguration() {
	hydrazine::json::Parser parser;
	hydrazine::json::Object* object = 0;

	// initialize
	instruction = false;
	kernelDimensions = false;
	kernelParameters= false;
	machineAttributes = false;
	memoryEfficiency = false;
	branch = false;
	parallelism = false;
	timing = false;

	try
	{
		/*
		std::ifstream stream("configure.ocelot");
		object = parser.parse_object(stream);

		hydrazine::json::Visitor mainConfig(object);
		hydrazine::json::Visitor traceConfig = mainConfig["eiger"];
		*/
		hydrazine::json::Object *configureOcelot = 
			static_cast<hydrazine::json::Object *>(api::OcelotConfiguration::get().configuration());

		hydrazine::json::Visitor main(configureOcelot);
		if(main.find("eiger")){
			hydrazine::json::Visitor eiger(main["eiger"]);
			/*
			hydrazine::json::Visitor eiger(main["trace"]);
		if (!traceConfig.is_null()) {
			hydrazine::json::Visitor eiger(traceConfig);
			*/

			// parse DB connection parameters
			dbLocation = eiger.parse<std::string>("databaseLocation", "localhost");
			dbName = eiger.parse<std::string>("databaseName", "eiger_development");
			dbUsername = eiger.parse<std::string>("databaseUsername", "eiger");
			dbPassword = eiger.parse<std::string>("databasePassword", "eigerdreams");

			// parse datacollection parameters
			dataCollection = eiger.parse<std::string>("dataCollection", "default");
			dataCollectionDesc = eiger.parse<std::string>("dataCollectionDescription", "");

			// parse dataset parameters
			datasetName = eiger.parse<std::string>("datasetName", "default");
			datasetDesc = eiger.parse<std::string>("datasetDesc", "");
			datasetURL = eiger.parse<std::string>("datasetURL", "");

			// parse start trial
			startTrial = eiger.parse<int>("startTrial", 1);

			//
			// add enabled trace generators
			//
			instruction = eiger.parse<bool>("instruction", false);
			kernelDimensions = eiger.parse<bool>("kernelDimensions", false);
			kernelParameters= eiger.parse<bool>("kernelParameters", false);
			machineAttributes = eiger.parse<bool>("machineAttributes", false);
			memoryEfficiency = eiger.parse<bool>("memoryEfficiency", false);
			branch = eiger.parse<bool>("branch", false);
			parallelism = eiger.parse<bool>("parallelism", false);
			timing = eiger.parse<bool>("timing", false);

			if(instruction) {
				report("Creating instruction trace generator");
				_instructionTraceGenerator = new InstructionTraceGenerator(this);
				ocelot::addTraceGenerator(*_instructionTraceGenerator, true);
			}
			
			if(kernelDimensions) {
				report("Creating kernel dimensions trace generator");
				_kernelDimensionsGenerator = new KernelDimensionsGenerator(this);
				ocelot::addTraceGenerator(*_kernelDimensionsGenerator, true);
			}

			if(kernelParameters) {
				report("Creating kernel parameters trace generator");
				_kernelParametersGenerator = new KernelParametersGenerator(this);
				ocelot::addTraceGenerator(*_kernelParametersGenerator, true);
			}

			if(machineAttributes) {
				report("Creating machine attributes trace generator");
				_machineAttributesGenerator = new MachineAttributesGenerator(this);
				ocelot::addTraceGenerator(*_machineAttributesGenerator, true);
			}

			if(memoryEfficiency) {
				report("Creating memory efficiency trace generator");
				_memoryEfficiencyGenerator = new MemoryEfficiencyGenerator(this);
				ocelot::addTraceGenerator(*_memoryEfficiencyGenerator, true);
			}

			if(branch) {
				report("Creating branch trace generator");
				_branchTraceGenerator = new BranchTraceGenerator(this);
				ocelot::addTraceGenerator(*_branchTraceGenerator, true);
			}

			if(parallelism) {
				report("Creating parallelism trace generator");
				_parallelismTraceGenerator = new ParallelismTraceGenerator(this);
				ocelot::addTraceGenerator(*_parallelismTraceGenerator, true);
			}

			if(timing) {
				report("Creating kernel timing generator");
				_kernelTimingGenerator = new KernelTimingGenerator(this);
				ocelot::addTraceGenerator(*_kernelTimingGenerator, true);
			}

		}
	}
	catch(const hydrazine::Exception& exp) {
		// default configuration
	}
}

TraceConfiguration::~TraceConfiguration() {
	delete _instructionTraceGenerator;
	delete _kernelDimensionsGenerator;
	delete _kernelParametersGenerator;
	delete _machineAttributesGenerator;
	delete _memoryEfficiencyGenerator;
	delete _branchTraceGenerator;
	delete _kernelTimingGenerator;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

TraceConfiguration::ConfigureInstructionTrace::ConfigureInstructionTrace(): 
	enabled(false) {
}

void TraceConfiguration::ConfigureInstructionTrace::initialize(
	hydrazine::json::Visitor &configure) {
	enabled = configure.parse<bool>("enabled", false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

}

extern "C" void eigerInitialize() {
	trace::TraceConfiguration::Singleton = new trace::TraceConfiguration;
	report("eigerInitialize() called");
}

