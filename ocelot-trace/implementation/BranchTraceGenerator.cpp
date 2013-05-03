/*!
	\file BranchTraceGenerator.cpp
	\author Gregory Diamos
	\date Monday April 13, 2009
	\brief The source file for the BranchTraceGenerator class
*/

#ifndef BRANCH_TRACE_GENERATOR_CPP_INCLUDED
#define BRANCH_TRACE_GENERATOR_CPP_INCLUDED

#include <ocelot-trace/interface/BranchTraceGenerator.h>

#include <ocelot/trace/interface/TraceEvent.h>
#include <ocelot/executive/interface/EmulatedKernel.h>
#include <ocelot/ir/interface/Module.h>
#include <hydrazine/implementation/Exception.h>
#include <fstream>
#include <ocelot/executive/interface/Device.h>

// Eiger includes
#include <api/eiger.h>

namespace trace
{
		
	unsigned int BranchTraceGenerator::BranchTraceGenerator::_counter = 0;

	BranchTraceGenerator::BranchTraceGenerator(trace::TraceConfiguration *config) : _config(config), trialCounter(0)
	{
	}
	
	BranchTraceGenerator::~BranchTraceGenerator()
	{
		
	}

	void BranchTraceGenerator::initialize(
		const executive::ExecutableKernel& kernel)
	{
		/*
		_entry.name = kernel.name;
		_entry.module = kernel.module->path();
		_entry.format = BranchTraceFormat;

		std::string name = kernel.name;
		
		if( name.size() > 20 )
		{
			name.resize( 20 );
		}

		std::stringstream stream;
		stream << _entry.format << "_" << _counter++;

		boost::filesystem::path path( database );
		path = path.parent_path();
		path /= _entry.program + "_" + name + "_" + stream.str() 
			+ ".header";
		path = boost::filesystem::system_complete( path );
		
		_entry.header = path.string();
		*/
		
		_header.format = BranchTraceFormat;
		_header.instructions = 0;
		_header.branches = 0;
		_header.divergent = 0;
		_header.threads =
			kernel.blockDim().x*kernel.blockDim().y*kernel.blockDim().z;
		_header.activeThreads = 0;
		_header.maxContextStackSize = 0;
		_header.totalContextStackSize = 0;

		// get device properties
		executive::Device::Properties prop = kernel.device->properties();
		machineName = prop.name;

		_kernelName = kernel.name;
		_kernelModule = kernel.module->path();

	}

	void BranchTraceGenerator::event( const TraceEvent& event )
	{
		if (event.contextStackSize > _header.maxContextStackSize) 
		{
			_header.maxContextStackSize = event.contextStackSize;
		}
		
		_header.totalContextStackSize += event.contextStackSize;

		if( event.instruction->opcode == ir::PTXInstruction::Bra )
		{
			_header.divergent = ( event.taken.count() != 0 
				&& event.fallthrough.count() != 0 ) 
				? _header.divergent + 1 : _header.divergent;
			++_header.branches;
		}
		++_header.instructions;
		_header.activeThreads += event.active.count();
	}
	
	void BranchTraceGenerator::finish()
	{
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

		 eiger::TrialID trialID;
		 trialID  = _config->startTrial + trialCounter;
		 /*
		// 7) construct new trial
		eiger::Trial trial(dc.getID(), machine.getID(), application.getID(), dataset.getID(), NULL);
		trial.commit();
		*/

		eiger::Execution exec(trialID, machine.getID());
		exec.commit();

		eiger::Metric branches(eiger::NONDETERMINISTIC, "branches", "branch count");
		branches.commit();
		eiger::NondeterministicMetric bcount(exec.getID(), branches.getID(), _header.branches);
		bcount.commit();

		eiger::Metric divergent(eiger::NONDETERMINISTIC, "divergent", "divergent branch count");
		divergent.commit();
		eiger::NondeterministicMetric dcount(exec.getID(), divergent.getID(), _header.divergent);
		dcount.commit();

		eiger::Metric maxcallstack(eiger::NONDETERMINISTIC, "maxContextStackSize", "maximum number of elements in the call stack");
		maxcallstack.commit();
		eiger::NondeterministicMetric mcallcount(exec.getID(), maxcallstack.getID(), _header.maxContextStackSize);
		mcallcount.commit();

		eiger::Metric totalcallstack(eiger::NONDETERMINISTIC, "totalContextStackSize", "total number of elements in call stack");
		totalcallstack.commit();
		eiger::NondeterministicMetric tcallcount(exec.getID(), totalcallstack.getID(), _header.totalContextStackSize);
		tcallcount.commit();


		// 8) disconnect from eiger
		eiger::Disconnect();
		trialCounter++;

		/*
		_entry.updateDatabase( database );

		std::ofstream hfile( _entry.header.c_str() );
		boost::archive::text_oarchive harchive( hfile );
	
		if( !hfile.is_open() )
		{
			throw hydrazine::Exception(
				"Failed to open BranchTraceGenerator header file " 
				+ _entry.header );
		}
		
		harchive << _header;
		
		hfile.close();
		*/
	}
	
}

#endif

