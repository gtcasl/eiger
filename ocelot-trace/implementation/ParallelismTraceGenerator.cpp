/*!
	\file ParallelismTraceGenerator.cpp
	
	\author Gregory Diamos
	\date Wednesday April 15, 2009
	
	\brief The source file for the ParallelismTraceGenerator class
*/

#ifndef PARALLELISM_TRACE_GENERATOR_CPP_INCLUDED
#define PARALLELISM_TRACE_GENERATOR_CPP_INCLUDED

#include <fstream>
#include <cfloat>

#include <ocelot/executive/interface/Device.h>
#include <ocelot/executive/interface/EmulatedKernel.h>
#include <ocelot/ir/interface/Module.h>
#include <ocelot/trace/interface/TraceEvent.h>
#include <ocelot-trace/interface/ParallelismTraceGenerator.h>

#include <hydrazine/implementation/debug.h>
#include <hydrazine/implementation/Exception.h>

#include <api/eiger.h>

#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 0

namespace trace
{
	
	unsigned int ParallelismTraceGenerator::ParallelismTraceGenerator::_counter 
		= 0;
	
	ParallelismTraceGenerator::ParallelismTraceGenerator(trace::TraceConfiguration *config) : _config(config), trialCounter(0)
	{
	}
	
	ParallelismTraceGenerator::~ParallelismTraceGenerator()
	{
	}

	void ParallelismTraceGenerator::initialize( 
		const executive::ExecutableKernel& kernel )
	{
		_kernelName = kernel.name;
		_kernelModule = kernel.module->path();
		
		// get device properties
		executive::Device::Properties prop = kernel.device->properties();
		_machineName = prop.name;

		/*
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
			+ ".trace";
		path = boost::filesystem::system_complete( path );
		
		_entry.path = path.string();
		
		path = path.parent_path();
		path /= _entry.program + "_" + name + "_" + stream.str() 
			+ ".header";
		path = boost::filesystem::system_complete( path );
		
		_entry.header = path.string();
		
		if( _file != 0 )
		{
			delete _archive; delete _file;
		}
		
		_file = new std::ofstream( _entry.path.c_str() );
		
		if( !_file->is_open() )
		{
			throw hydrazine::Exception(
				"Failed to open ParallelismTraceGenerator kernel trace file " 
				+ _entry.path );
		}
		
		_archive = new boost::archive::text_oarchive( *_file );
		*/
		
		_header.format = ParallelismTraceFormat;
		_header.dimensions = kernel.gridDim().x * kernel.gridDim().y 
			* kernel.gridDim().z;
		report("CTA Dimensions are " << _header.dimensions);
		_header.threads = kernel.maxThreadsPerBlock();
		
		_event.ctaid = 0;
		_event.instructions = 0;
		_event.activity = 0;
	}

	void ParallelismTraceGenerator::event( const TraceEvent& event )
	{
		if( event.instruction->opcode == ir::PTXInstruction::Exit )
		{
			_event.activity /= ( _event.instructions + DBL_EPSILON );
			_events.push_back(_event);
			++_event.ctaid;
			_event.instructions = 0;
			_event.activity = 0;
		}
		else
		{
			_event.activity += event.active.count();
			++_event.instructions;
		}		
	}	
	
	void ParallelismTraceGenerator::finish()
	{

		_event.activity /= ( _event.instructions + DBL_EPSILON );
		_events.push_back(_event);
		++_event.ctaid;
		_event.instructions = 0;
		_event.activity = 0;

		double MIMDspeedup = 0.0;
		long long unsigned int totalInstructions = 0;
		double activity = 0.0;

		for(std::vector<Event>::iterator it = _events.begin();
				it != _events.end(); it++){

			totalInstructions += it->instructions;
			activity += it->activity * it->instructions;
		}

		long long unsigned int maxInstructions = 0;
		for(std::vector<Event>::iterator it = _events.begin();
				it != _events.end(); it++){

			if(it->instructions > maxInstructions){
				maxInstructions = it->instructions;
			}
		}

		MIMDspeedup = totalInstructions / (maxInstructions + DBL_EPSILON);

		activity /= totalInstructions + DBL_EPSILON;

		//
		// send to eiger
		//

		// 1) set up DB connection
		eiger::Connect(_config->dbLocation, _config->dbName, _config->dbUsername, _config->dbPassword);
		
		// 2) find/make this data collection
		eiger::DataCollection dc(_config->dataCollection, _config->dataCollectionDesc);
		dc.commit();

		// 3) find/make this machine
		eiger::Machine machine(_machineName, "");
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
		 trialID = _config->startTrial + trialCounter;
		 /*
		// 7) construct new trial
		eiger::Trial trial(dc.getID(), machine.getID(), application.getID(), dataset.getID(), NULL);
		trial.commit();
		*/

		eiger::Execution exec(trialID, machine.getID());
		exec.commit();

		eiger::Metric mimd(eiger::NONDETERMINISTIC, "MIMD utilization", "average number of CTAs launched per dynamic instruction");
		mimd.commit();

		eiger::Metric simd(eiger::NONDETERMINISTIC, "SIMD utilization", "average active threads per dynamic instruction");
		simd.commit();

		eiger::NondeterministicMetric dysimd(exec.getID(), simd.getID(), activity);
		dysimd.commit();

		eiger::NondeterministicMetric dymimd(exec.getID(), mimd.getID(), MIMDspeedup);
		dymimd.commit();


		// 8) disconnect from eiger
		eiger::Disconnect();
		trialCounter++;

		/*
		if( _file != 0 )
		{
			_entry.updateDatabase( database );
			delete _archive;

			_file->close();	
			delete _file;
			_file = 0;
			
			std::ofstream hfile( _entry.header.c_str() );
			boost::archive::text_oarchive harchive( hfile );
		
			if( !hfile.is_open() )
			{
				throw hydrazine::Exception(
					"Failed to open ParallelismTraceGenerator header file " 
					+ _entry.header );
			}
			
			harchive << _header;
			
			hfile.close();
			
			assert( _event.instructions == 0 );
			assert( _event.activity == 0 );
			assert( _event.ctaid == _header.dimensions );
		}	
		*/
	}
}

#endif

