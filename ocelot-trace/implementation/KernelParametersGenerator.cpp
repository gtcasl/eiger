/*!
	\file KernelParametersGenerator.cpp
	\author Andrew Kerr <arkerr@gatech.edu>
	\date Jan 6, 2010
	\brief records dimensions of launched kernels - note: this assumes trace generators are only
		applied to EmulatedKernel instances
*/

// C++ stdlib includes
#include <fstream>

// Ocelot includes
#include <ocelot/ir/interface/PTXOperand.h>
#include <ocelot/executive/interface/ExecutableKernel.h>
#include <ocelot/ir/interface/Module.h>
#include <ocelot/executive/interface/EmulatedKernel.h>
#include <ocelot/trace/interface/TraceEvent.h>
#include <ocelot-trace/interface/KernelParametersGenerator.h>
#include <ocelot/executive/interface/Device.h>

// Hydrazine includes
#include <hydrazine/implementation/Exception.h>

// Eiger includes
#include <api/eiger.h>

//////////////////////////////////////////////////////////////////////////////////////////////////

trace::KernelParametersGenerator::Header::Header() {

}

//////////////////////////////////////////////////////////////////////////////////////////////////
		
unsigned int trace::KernelParametersGenerator::KernelParametersGenerator::_counter = 0;

trace::KernelParametersGenerator::KernelParametersGenerator(trace::TraceConfiguration *config) : _config(config) {
}
	
trace::KernelParametersGenerator::~KernelParametersGenerator() {
}

void trace::KernelParametersGenerator::initialize(const executive::ExecutableKernel& kernel) {
	_kernelName = kernel.name;
	_kernelModule = kernel.module->path();

	// get device properties
	executive::Device::Properties prop = kernel.device->properties();

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
		+ ".header";
	path = boost::filesystem::system_complete( path );
	
	_entry.header = path.string();
	*/
	
	//_header.format = KernelParametersFormat;
	
	//
	// this may not be strictly safe - a better solution is to improve the interface to
	// Executable kernel
	//
	const executive::EmulatedKernel &emuKernel = static_cast<const executive::EmulatedKernel & >(kernel);
	
	_header.block = emuKernel.blockDim();
	_header.grid = emuKernel.gridDim();

	//
  // send to eiger
  //

  // 1) set up DB connection
  eiger::Connect(_config->dbLocation, _config->dbName, _config->dbUsername, _config->dbPassword);
  
  /*
  // 2) find/make this data collection
	eiger::DataCollection dc(_config->dataCollection, _config->dataCollectionDesc);
	dc.commit();

  // 3) find/make this machine
  eiger::Machine machine(prop.name, "");
	machine.commit();
	*/

  // 4) find/make this application
  eiger::Application application(_kernelName, _kernelModule);
	application.commit();

  // 5) find/make this dataset
   eiger::Dataset dataset(application.getID(), _config->datasetName, _config->datasetDesc, _config->datasetURL);
	 dataset.commit();

  // 6) find/make this properties
  // TODO

	 /*
  // 7) construct new trial
  eiger::Trial trial(dc.getID(), machine.getID(), application.getID(), dataset.getID(), NULL);
	trial.commit();
	*/

	// collect kernel parameters
	for(int i = 0; i < emuKernel.arguments.size(); i++){
		ir::Parameter param = emuKernel.arguments[i];
		if(param.argument){
			std::stringstream stream;
			std::string svalue = ir::Parameter::value(param);
			bool done = false;
			double value;
			switch(param.type){
				case ir::PTXOperand::u8:
				case ir::PTXOperand::s8:
				case ir::PTXOperand::u16:
				case ir::PTXOperand::s16:
				case ir::PTXOperand::u32:
				case ir::PTXOperand::s32:
				case ir::PTXOperand::s64:
					stream << ir::Parameter::value(param);
					stream >> value;
					break;
				case ir::PTXOperand::b8:
				case ir::PTXOperand::b16:
				case ir::PTXOperand::b32:
				case ir::PTXOperand::b64:
				case ir::PTXOperand::u64:
					stream << std::hex << ir::Parameter::value(param);
					long vtemp;
					stream >> vtemp;
					value = (double) vtemp;	
					break;
				default: done = true; break;
			}
			if(done) continue;

			eiger::Metric arg(eiger::DETERMINISTIC, param.name, "kernel parameter");
			arg.commit();
			eiger::DeterministicMetric val(dataset.getID(), arg.getID(), value);
			val.commit();
		}
	}

	// 8) disconnect from eiger
  eiger::Disconnect();


}

void trace::KernelParametersGenerator::event( const TraceEvent& event ) {
}
	
void trace::KernelParametersGenerator::finish() {
/*
	_entry.updateDatabase( database );

	std::ofstream hfile( _entry.header.c_str() );
	boost::archive::text_oarchive harchive( hfile );

	if( !hfile.is_open() )
	{
		throw hydrazine::Exception(
			"Failed to open KernelParametersGenerator header file " 
			+ _entry.header );
	}
	
	harchive << _header;
	
	hfile.close();
	*/

}

