/*!
	\file KernelDimensionsGenerator.cpp
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
#include <ocelot-trace/interface/KernelDimensionsGenerator.h>
#include <ocelot/executive/interface/Device.h>

// Hydrazine includes
#include <hydrazine/implementation/Exception.h>

// Eiger includes
#include <api/eiger.h>

//////////////////////////////////////////////////////////////////////////////////////////////////

trace::KernelDimensionsGenerator::Header::Header() {

}

//////////////////////////////////////////////////////////////////////////////////////////////////
		
unsigned int trace::KernelDimensionsGenerator::KernelDimensionsGenerator::_counter = 0;

trace::KernelDimensionsGenerator::KernelDimensionsGenerator(trace::TraceConfiguration *config) : _config(config) {
}
	
trace::KernelDimensionsGenerator::~KernelDimensionsGenerator() {
}

void trace::KernelDimensionsGenerator::initialize(const executive::ExecutableKernel& kernel) {
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
	
	_header.format = KernelDimensionsFormat;
	
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

	eiger::Metric blockdimx(eiger::DETERMINISTIC, "blockDim.x" , "block dimension (x)");
	blockdimx.commit();
	eiger::Metric blockdimy(eiger::DETERMINISTIC, "blockDim.y" , "block dimension (y)");
	blockdimy.commit();
	eiger::Metric blockdimz(eiger::DETERMINISTIC, "blockDim.z" , "block dimension (z)");
	blockdimz.commit();

	eiger::DeterministicMetric bdx(dataset.getID(), blockdimx.getID(), _header.block.x);
	bdx.commit();
	eiger::DeterministicMetric bdy(dataset.getID(), blockdimy.getID(), _header.block.y);
	bdy.commit();
	eiger::DeterministicMetric bdz(dataset.getID(), blockdimz.getID(), _header.block.z);
	bdz.commit();

	eiger::Metric griddimx(eiger::DETERMINISTIC, "gridDim.x" , "grid dimension (x)");
	griddimx.commit();
	eiger::Metric griddimy(eiger::DETERMINISTIC, "gridDim.y" , "grid dimension (y)");
	griddimy.commit();
	eiger::Metric griddimz(eiger::DETERMINISTIC, "gridDim.z" , "grid dimension (z)");
	griddimz.commit();

	eiger::DeterministicMetric gdx(dataset.getID(), griddimx.getID(), _header.grid.x);
	gdx.commit();
	eiger::DeterministicMetric gdy(dataset.getID(), griddimy.getID(), _header.grid.y);
	gdy.commit();
	eiger::DeterministicMetric gdz(dataset.getID(), griddimz.getID(), _header.grid.z);
	gdz.commit();

	// 8) disconnect from eiger
  eiger::Disconnect();


}

void trace::KernelDimensionsGenerator::event( const TraceEvent& event ) {
}
	
void trace::KernelDimensionsGenerator::finish() {
/*
	_entry.updateDatabase( database );

	std::ofstream hfile( _entry.header.c_str() );
	boost::archive::text_oarchive harchive( hfile );

	if( !hfile.is_open() )
	{
		throw hydrazine::Exception(
			"Failed to open KernelDimensionsGenerator header file " 
			+ _entry.header );
	}
	
	harchive << _header;
	
	hfile.close();
	*/

}

