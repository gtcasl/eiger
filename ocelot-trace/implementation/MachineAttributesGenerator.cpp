/*!
	\file MachineAttributesGenerator.cpp
	\author Gregory Diamos
	\date Monday April 13, 2009
	\brief The source file for the MachineAttributesGenerator class
*/

// C++ stdlib includes
#include <fstream>

// Ocelot includes
#include <ocelot/ir/interface/PTXOperand.h>
#include <ocelot/executive/interface/ExecutableKernel.h>
#include <ocelot/ir/interface/Module.h>
#include <ocelot/executive/interface/EmulatedKernel.h>
#include <ocelot/trace/interface/TraceEvent.h>
#include <ocelot-trace/interface/MachineAttributesGenerator.h>
#include <ocelot/executive/interface/Device.h>

// Hydrazine includes
#include <hydrazine/implementation/Exception.h>

// Eiger includes
#include <api/eiger.h>

unsigned int trace::MachineAttributesGenerator::MachineAttributesGenerator::_counter = 0;

trace::MachineAttributesGenerator::MachineAttributesGenerator(trace::TraceConfiguration *config) : _config(config) {
}

trace::MachineAttributesGenerator::~MachineAttributesGenerator() {
}

void trace::MachineAttributesGenerator::initialize(const executive::ExecutableKernel& kernel) {

	// get device properties
	executive::Device::Properties prop = kernel.device->properties();

	//
  // send to eiger
  //

  // 1) set up DB connection
  eiger::Connect(_config->dbLocation, _config->dbName, _config->dbUsername, _config->dbPassword);
  
  /*
  // 2) find/make this data collection
	eiger::DataCollection dc(_config->dataCollection, _config->dataCollectionDesc);
	dc.commit();
	*/

  // 3) find/make this machine
  eiger::Machine machine(prop.name, "");
	machine.commit();

	/*
  // 4) find/make this application
  eiger::Application application(kernel.name, kernel.module->path());
	application.commit();

  // 5) find/make this dataset
   eiger::Dataset dataset(application.getID(), "default", "default parboil dataset", "mpact.crhc.illinois.edu");
	 dataset.commit();

  // 6) find/make this properties
  // TODO

  // 7) construct new trial
  eiger::Trial trial(dc.getID(), machine.getID(), application.getID(), dataset.getID(), NULL);
	trial.commit();
	*/


	// add metrics - hardcoded now, change?
	eiger::Metric met1(eiger::MACHINE, "totalMemory" , "total global mem in bytes");
	met1.commit();
	eiger::MachineMetric mmet1(machine.getID(), met1.getID(), prop.totalMemory);
	mmet1.commit();

	eiger::Metric met2(eiger::MACHINE, "sharedMemPerBlock" , "max shared mem per thread block in bytes");
	met2.commit();
	eiger::MachineMetric mmet2(machine.getID(), met2.getID(), prop.sharedMemPerBlock);
	mmet2.commit();

	eiger::Metric met3(eiger::MACHINE, "regsPerBlock" , "max number of 32 bit registers per thread block");
	met3.commit();
	eiger::MachineMetric mmet3(machine.getID(), met3.getID(), prop.regsPerBlock);
	mmet3.commit();

	eiger::Metric met4(eiger::MACHINE, "SIMDWidth" , "warp size in threads");
	met4.commit();
	eiger::MachineMetric mmet4(machine.getID(), met4.getID(), prop.SIMDWidth);
	mmet4.commit();

	eiger::Metric met5(eiger::MACHINE, "memPitch" , "max pitch size for mem copy in bytes");
	met5.commit();
	eiger::MachineMetric mmet5(machine.getID(), met5.getID(), prop.memPitch);
	mmet5.commit();

	eiger::Metric met6(eiger::MACHINE, "maxThreadsPerBlock" , "max number of threads per block");
	met6.commit();
	eiger::MachineMetric mmet6(machine.getID(), met6.getID(), prop.maxThreadsPerBlock);
	mmet6.commit();

	eiger::Metric met7(eiger::MACHINE, "maxThreadsDim.x" , "max number of threads in x dimension");
	met7.commit();
	eiger::MachineMetric mmet7(machine.getID(), met7.getID(), prop.maxThreadsDim[0]);
	mmet7.commit();

	eiger::Metric met8(eiger::MACHINE, "maxThreadsDim.y" , "max number of threads in y dimension");
	met8.commit();
	eiger::MachineMetric mmet8(machine.getID(), met8.getID(), prop.maxThreadsDim[1]);
	mmet8.commit();

	eiger::Metric met9(eiger::MACHINE, "maxThreadsDim.z" , "max number of threads in z dimension");
	met9.commit();
	eiger::MachineMetric mmet9(machine.getID(), met9.getID(), prop.maxThreadsDim[2]);
	mmet9.commit();

	eiger::Metric met10(eiger::MACHINE, "maxGridSize.x" , "max number of blocks in the x dimension");
	met10.commit();
	eiger::MachineMetric mmet10(machine.getID(), met10.getID(), prop.maxGridSize[0]);
	mmet10.commit();

	eiger::Metric met11(eiger::MACHINE, "maxGridSize.y" , "max number of blocks in the y dimension");
	met11.commit();
	eiger::MachineMetric mmet11(machine.getID(), met11.getID(), prop.maxGridSize[1]);
	mmet11.commit();

	eiger::Metric met12(eiger::MACHINE, "maxGridSize.z" , "max number of blocks in the z dimension");
	met12.commit();
	eiger::MachineMetric mmet12(machine.getID(), met12.getID(), prop.maxGridSize[2]);
	mmet12.commit();

	eiger::Metric met13(eiger::MACHINE, "clockRate" , "clock frequency in kilohertz");
	met13.commit();
	eiger::MachineMetric mmet13(machine.getID(), met13.getID(), prop.clockRate);
	mmet13.commit();

	eiger::Metric met14(eiger::MACHINE, "totalConstantMemory" , "total amount of constant memory in bytes");
	met14.commit();
	eiger::MachineMetric mmet14(machine.getID(), met14.getID(), prop.totalConstantMemory);
	mmet14.commit();

	eiger::Metric met15(eiger::MACHINE, "major" , "major revision number for compute capability");
	met15.commit();
	eiger::MachineMetric mmet15(machine.getID(), met15.getID(), prop.major);
	mmet15.commit();

	eiger::Metric met16(eiger::MACHINE, "minor" , "minor revision number for compute capability");
	met16.commit();
	eiger::MachineMetric mmet16(machine.getID(), met16.getID(), prop.minor);
	mmet16.commit();

	eiger::Metric met17(eiger::MACHINE, "textureAlign" , "texture alignment requirement");
	met17.commit();
	eiger::MachineMetric mmet17(machine.getID(), met17.getID(), prop.textureAlign);
	mmet17.commit();

	eiger::Metric met18(eiger::MACHINE, "memcpyOverlap" , "1 if device can concurrently copy memory between host and device while executing a kernel, 0 otherwise");
	met18.commit();
	eiger::MachineMetric mmet18(machine.getID(), met18.getID(), prop.memcpyOverlap);
	mmet18.commit();

	eiger::Metric met19(eiger::MACHINE, "multiprocessorCount" , "number of multiprocessors");
	met19.commit();
	eiger::MachineMetric mmet19(machine.getID(), met19.getID(), prop.multiprocessorCount);
	mmet19.commit();

	eiger::Metric met20(eiger::MACHINE, "concurrentKernels" , "concurrent kernel execution");
	met20.commit();
	eiger::MachineMetric mmet20(machine.getID(), met20.getID(), prop.concurrentKernels);
	mmet20.commit();

	eiger::Metric met21(eiger::MACHINE, "integrated" , "1 if device is an integrated (motherboard) device, 0 if discrete (card)");
	met21.commit();
	eiger::MachineMetric mmet21(machine.getID(), met21.getID(), prop.integrated);
	mmet21.commit();

	eiger::Metric met22(eiger::MACHINE, "stackSize" , "stack size");
	met22.commit();
	eiger::MachineMetric mmet22(machine.getID(), met22.getID(), prop.stackSize);
	mmet22.commit();

	eiger::Metric met23(eiger::MACHINE, "printfFIFOSize" , "printfFIFOSize");
	met23.commit();
	eiger::MachineMetric mmet23(machine.getID(), met23.getID(), prop.printfFIFOSize);
	mmet23.commit();
	
	eiger::Metric met24(eiger::MACHINE, "unifiedAddressing" , "device shares a unified address with the host");
	met24.commit();
	eiger::MachineMetric mmet24(machine.getID(), met24.getID(), prop.unifiedAddressing);
	mmet24.commit();

	eiger::Metric met25(eiger::MACHINE, "memoryClockRate" , "peak memory clock frequency in kilohertz");
	met25.commit();
	eiger::MachineMetric mmet25(machine.getID(), met25.getID(), prop.memoryClockRate);
	mmet25.commit();

	eiger::Metric met26(eiger::MACHINE, "memoryBusWidth" , "global memory bus width in bits");
	met26.commit();
	eiger::MachineMetric mmet26(machine.getID(), met26.getID(), prop.memoryBusWidth);
	mmet26.commit();

	eiger::Metric met27(eiger::MACHINE, "l2CacheSize" , "size of l2 cache in bytes");
	met27.commit();
	eiger::MachineMetric mmet27(machine.getID(), met27.getID(), prop.l2CacheSize);
	mmet27.commit();

	eiger::Metric met28(eiger::MACHINE, "maxThreadsPerMultiProcessor" , "maximum resident threads per multiprocessor");
	met28.commit();
	eiger::MachineMetric mmet28(machine.getID(), met28.getID(), prop.maxThreadsPerMultiProcessor);
	mmet28.commit();


	/*
	_entry.name = kernel.name;
	_entry.module = kernel.module->path();
	_entry.format = KernelDimensionsFormat;

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
	
	_header.format = MachineAttributesFormat;
	*/
	
}

void trace::MachineAttributesGenerator::event( const TraceEvent& event ) {
}

void trace::MachineAttributesGenerator::finish() {

	/*
	_entry.updateDatabase( database );

	std::ofstream hfile( _entry.header.c_str() );
	boost::archive::text_oarchive harchive( hfile );

	if( !hfile.is_open() )
	{
		throw hydrazine::Exception(
			"Failed to open MachineAttributesGenerator header file " 
			+ _entry.header );
	}
	
	harchive << _header;
	
	hfile.close();
	*/
}

