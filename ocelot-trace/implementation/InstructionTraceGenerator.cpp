/*!
	\file InstructionTraceGenerator.cpp
	\author Andrew Kerr <arkerr@gatech.edu>
	\date date Jan 5, 2010
	\brief captures static and dynamic instruction counts
*/

// C++ stdlib includes
#include <fstream>

// Ocelot includes
#include <ocelot/executive/interface/ExecutableKernel.h>
#include <ocelot/ir/interface/Module.h>
#include <ocelot/executive/interface/EmulatedKernel.h>
#include <ocelot/trace/interface/TraceEvent.h>
#include <ocelot-trace/interface/InstructionTraceGenerator.h>
#include <ocelot/executive/interface/Device.h>

// Hydrazine includes
#include <hydrazine/implementation/Exception.h>
#include <hydrazine/implementation/debug.h>
#include <hydrazine/implementation/macros.h>

// Eiger includes
#include <api/eiger.h>

#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 0

//////////////////////////////////////////////////////////////////////////////////////////////////

trace::InstructionTraceGenerator::InstructionCounter::InstructionCounter():
	dynamic_count(0), static_count(0), activity(0) {
}

void trace::InstructionTraceGenerator::InstructionCounter::count(
	const ir::PTXInstruction &instr, size_t active) {

	++dynamic_count;
	activity += (double)active;
}

trace::InstructionTraceGenerator::InstructionCounter & 
	trace::InstructionTraceGenerator::InstructionCounter::operator+=( const InstructionCounter &counter) {

	dynamic_count += counter.dynamic_count;
	static_count += counter.static_count;
	activity += counter.activity;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

const char * trace::InstructionTraceGenerator::toString(FunctionalUnit fu) {
	switch (fu) {
		case Integer_arithmetic:	//! integer arithmetic
			return "Integer_arithmetic";
		case Integer_logical:		//! itneger logical
			return "Integer_logical";
		case Integer_comparison:	//! comparison
			return "Integer_comparison";
		case Float_single:				//! floating-point single-precision
			return "Float_single";
		case Float_double:				//! floating-point, double-precision
			return "Float_double";
		case Float_comparison:		//! floating-point comparison
			return "Float_comparison";
		case Memory_offchip:			//! off-chip: {global, local}
			return "Memory_offchip";
		case Memory_onchip:			//! cached or scratchpad: {texture, shared, constant}
			return "Memory_onchip";
		case Control:						//! control-flow instructions
			return "Control";
		case Parallelism:				//! parallelism: sync, reduction, vote
			return "Parallelism";
		case Special:						//! transcendental and special functions
			return "Special";
		case Other:							//! not categorized
			return "Other";
		default:
			return "FunctionalUnit_invalid";
	}
}

/*!
	\brief gets the functional unit associated with a particular PTXInstruction
	\param instr PTXInstruction instance
	\return functional unit
*/
trace::InstructionTraceGenerator::FunctionalUnit trace::InstructionTraceGenerator::getFunctionalUnit(
	const ir::PTXInstruction &instr) {

	// switch warning: many fall-through cases follow - code blocks not listed in braces are suspect
	switch (instr.opcode) {

	// Arithmetic
	case ir::PTXInstruction::Abs:
	case ir::PTXInstruction::Add:
	case ir::PTXInstruction::AddC:
	case ir::PTXInstruction::Clz:
	case ir::PTXInstruction::Cvt:
	case ir::PTXInstruction::Cvta:
	case ir::PTXInstruction::Div:
  case ir::PTXInstruction::Fma:
	case ir::PTXInstruction::Mad24:
	case ir::PTXInstruction::Mad:
	case ir::PTXInstruction::Max:
	case ir::PTXInstruction::Min:
	case ir::PTXInstruction::Mov:
	case ir::PTXInstruction::Mul24:
	case ir::PTXInstruction::Mul:
	case ir::PTXInstruction::Neg:
	case ir::PTXInstruction::Rcp:
	case ir::PTXInstruction::Rem:
	case ir::PTXInstruction::Sad:
	case ir::PTXInstruction::Sub:
		{
			switch (instr.type) {
				case ir::PTXOperand::b8:
				case ir::PTXOperand::b16:
				case ir::PTXOperand::b32:
				case ir::PTXOperand::b64:
				case ir::PTXOperand::s8:
				case ir::PTXOperand::s16:
				case ir::PTXOperand::s32:
				case ir::PTXOperand::s64:
				case ir::PTXOperand::u8:
				case ir::PTXOperand::u16:
				case ir::PTXOperand::u32:
				case ir::PTXOperand::u64:
				case ir::PTXOperand::pred:
					{
						return Integer_arithmetic;
					}
				case ir::PTXOperand::f32:
					{
						return Float_single;
					}
				case ir::PTXOperand::f64:
					{
						return Float_double;
					}
				default:
					{
						//std::cout << "invalid arithmetic op: " << ir::PTXInstruction::toString(instr.opcode) << std::endl;
					//return FunctionalUnit_invalid;
					}
			}
		}
		break;

	// Logical
	case ir::PTXInstruction::And:
	case ir::PTXInstruction::CNot:
	case ir::PTXInstruction::Not:
	case ir::PTXInstruction::Or:
	case ir::PTXInstruction::Shl:
	case ir::PTXInstruction::Shr:
	case ir::PTXInstruction::Xor:
		{
			switch (instr.type) {
				case ir::PTXOperand::b8:
				case ir::PTXOperand::b16:
				case ir::PTXOperand::b32:
				case ir::PTXOperand::b64:
				case ir::PTXOperand::s8:
				case ir::PTXOperand::s16:
				case ir::PTXOperand::s32:
				case ir::PTXOperand::s64:
				case ir::PTXOperand::u8:
				case ir::PTXOperand::u16:
				case ir::PTXOperand::u32:
				case ir::PTXOperand::u64:
				case ir::PTXOperand::pred:
					{
						return Integer_logical;
					}
				default:
					{
						//std::cout << "invalid logical op: " << ir::PTXInstruction::toString(instr.opcode) << " " << ir::PTXOperand::toString(instr.type) << std::endl;
					//return FunctionalUnit_invalid;
					}
			}
		}
		break;

	// Comparison
	case ir::PTXInstruction::SetP:
	case ir::PTXInstruction::Set:
	case ir::PTXInstruction::SlCt:
	case ir::PTXInstruction::SelP:
		{
			switch (instr.type) {
				case ir::PTXOperand::f32:
				case ir::PTXOperand::f64:
					{
						return Float_comparison;
					}

				case ir::PTXOperand::b8:
				case ir::PTXOperand::b16:
				case ir::PTXOperand::b32:
				case ir::PTXOperand::b64:
				case ir::PTXOperand::s8:
				case ir::PTXOperand::s16:
				case ir::PTXOperand::s32:
				case ir::PTXOperand::s64:
				case ir::PTXOperand::u8:
				case ir::PTXOperand::u16:
				case ir::PTXOperand::u32:
				case ir::PTXOperand::u64:
					{
						return Integer_comparison;
					}
				default:
					{
						//std::cout << "invalid comparison op: " << ir::PTXInstruction::toString(instr.opcode) << std::endl;
					//break;
					}
			}
		}
		break;

	// Transcendental and other special
	case ir::PTXInstruction::Cos:
	case ir::PTXInstruction::Ex2:
	case ir::PTXInstruction::Lg2:
	case ir::PTXInstruction::Rsqrt:
	case ir::PTXInstruction::Sqrt:
	case ir::PTXInstruction::Sin:
		{
			switch (instr.type) {
				case ir::PTXOperand::f32:
				case ir::PTXOperand::f64:
					{
						return Special;
					}
				default:
					{
						//std::cout << "invalid transcendental op: " << ir::PTXInstruction::toString(instr.opcode) << std::endl;
					//return FunctionalUnit_invalid;
					}
			}
		}
		break;

	// Memory
	case ir::PTXInstruction::Atom:
	case ir::PTXInstruction::Ld:
	case ir::PTXInstruction::Ldu:
	case ir::PTXInstruction::Membar:
	case ir::PTXInstruction::Tex:
	case ir::PTXInstruction::St:
		{
			switch (instr.addressSpace) {
				case ir::PTXInstruction::Global:
				case ir::PTXInstruction::Local:
				{
					return Memory_offchip;
				}

				default:
				{
					return Memory_onchip;
				}
			}
		}
		break;

	// Parallelism
	case ir::PTXInstruction::Bar:
	case ir::PTXInstruction::Red:
	case ir::PTXInstruction::Vote:
		{
			return Parallelism;
		}
		break;

	// Control flow
	case ir::PTXInstruction::Bra:
	case ir::PTXInstruction::Call:
	case ir::PTXInstruction::Exit:
	case ir::PTXInstruction::Reconverge:
	case ir::PTXInstruction::Ret:
		{
			return Control;
		}
		break;

	// this should be an exhaustive list!
	default:
		{
			//std::cout << "invalid op (thats it!): " << ir::PTXInstruction::toString(instr.opcode) << std::endl;
		break;
		}
	}

	std::stringstream ss;
	ss << "InstructionTraceGenerator - PTX instruction not supported: " 
		<< ir::PTXInstruction::toString(instr.opcode);
	throw hydrazine::Exception(ss.str());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

trace::InstructionTraceGenerator::Header::Header():
	format(trace::TraceGenerator::InstructionTraceFormat), total_dynamic(0), total_static(0) {
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int trace::InstructionTraceGenerator::_counter = 0;

/*!
	default constructor
*/
trace::InstructionTraceGenerator::InstructionTraceGenerator(trace::TraceConfiguration *config) : _config(config), trialCounter(0){

}

/*!
	\brief destructs instruction trace generator
*/
trace::InstructionTraceGenerator::~InstructionTraceGenerator() {

}

/*!
	\brief called when a traced kernel is launched to retrieve some 
		parameters from the kernel
*/
void trace::InstructionTraceGenerator::initialize(
	const executive::ExecutableKernel& kernel) {
	//
	// initialize kernel entry
	//
	_kernelName = kernel.name;
	_kernelModule = kernel.module->path();
	
	// get device properties
	executive::Device::Properties prop = kernel.device->properties();
	_machineName = prop.name;

/*
	std::string name = kernel.name;

	if( name.size() > 20 ) name.resize( 20 );
	std::stringstream stream;
	stream << _entry.format << "_" << _counter++;

	boost::filesystem::path path( database );
	
	path = path.parent_path();
	path /= _entry.program + "_" + name + "_" + stream.str() + ".header";
	path = boost::filesystem::system_complete( path );
	
	_entry.header = path.string();
*/
	
	_header.format = InstructionTraceFormat;
		
	// static counts
	const executive::EmulatedKernel &emuKernel = 
		static_cast<const executive::EmulatedKernel &>(kernel);
		
	_header.blockDim = emuKernel.blockDim();
	_header.gridDim = emuKernel.gridDim();

	
	threadCount = _header.blockDim.x * _header.blockDim.y * _header.blockDim.z;

	// make new opmap and counter for every type
	InstructionCounter ic1;
	instructionCounter[Integer_arithmetic] = ic1;

	InstructionCounter ic2;
	instructionCounter[Integer_logical] = ic2;

	InstructionCounter ic3;
	instructionCounter[Integer_comparison] = ic3;

	InstructionCounter ic4;
	instructionCounter[Float_single] = ic4;

	InstructionCounter ic5;
	instructionCounter[Float_double] = ic5;

	InstructionCounter ic6;
	instructionCounter[Float_comparison] = ic6;

	InstructionCounter ic7;
	instructionCounter[Memory_offchip] = ic7;

	InstructionCounter ic8;
	instructionCounter[Memory_onchip] = ic8;

	InstructionCounter ic9;
	instructionCounter[Control] = ic9;

	InstructionCounter ic10;
	instructionCounter[Parallelism] = ic10;

	InstructionCounter ic11;
	instructionCounter[Special] = ic11;

	InstructionCounter ic12;
	instructionCounter[Other] = ic12;

	InstructionCounter ic13;
	instructionCounter[FunctionalUnit_invalid] = ic13;
	
	
	for (executive::EmulatedKernel::PTXInstructionVector::const_iterator instr_it = 
		emuKernel.instructions.begin();
		instr_it != emuKernel.instructions.end(); ++instr_it ) {
	
		const ir::PTXInstruction & instr = *instr_it;
		
		FunctionalUnit fu = getFunctionalUnit(instr);
		/*if (instructionCounter.find(fu) == instructionCounter.end()) {
			OpcodeCountMap opMap;
			instructionCounter[fu] = opMap;
		}
		if (instructionCounter[fu].find(instr.opcode) == instructionCounter[fu].end()) {
			InstructionCounter counter;
			instructionCounter[fu][instr.opcode] = counter;
		}
		*/
		
		++instructionCounter[fu].static_count;
	}
}

/*!
	\brief Called whenever an event takes place.

	Note, the const reference 'event' is only valid until event() 
	returns
*/
void trace::InstructionTraceGenerator::event(const TraceEvent & event) {
	// every instruction we encounter is guaranteed to be mapped to in the instructionCounter
	// data structure
	FunctionalUnit fu = getFunctionalUnit(*event.instruction);
	instructionCounter[fu].count(*event.instruction, 
		event.active.count());
}

/*! 
	\brief Called when a kernel is finished. There will be no more 
		events for this kernel.
*/
void trace::InstructionTraceGenerator::finish() {

  // visit every instruction counter and divide activity by thread count
	if (threadCount) {
		for (FunctionalUnitCountMap::iterator fu_it = instructionCounter.begin(); 
			fu_it != instructionCounter.end(); ++fu_it) {
	
			if (threadCount != 0 && fu_it->second.dynamic_count != 0) {
				fu_it->second.activity /= (double)threadCount * (double)fu_it->second.dynamic_count;
			}
			else {
				fu_it->second.activity = 0;
			}
		}
	}


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
  // 7) construct new trial
  //eiger::Trial trial(dc.getID(), 1, application.getID(), dataset.getID(), NULL);
	//trial.commit();

		eiger::Execution exec(trialID, machine.getID());
		exec.commit();

  // 8) for each dynamic metric:
  //    - find/construct this metric
  //    - construct new dynamic metric
  for(FunctionalUnitCountMap::iterator fu_it = instructionCounter.begin();
      fu_it != instructionCounter.end(); ++fu_it) {
    
		std::string name = InstructionTraceGenerator::toString((FunctionalUnit) fu_it->first);
		name += " instruction count";

    eiger::Metric dyncount(eiger::NONDETERMINISTIC, "dynamic " + name, "dynamic count");
		dyncount.commit();
		eiger::Metric statcount(eiger::DETERMINISTIC, "static " + name, "static count");
		statcount.commit();
		eiger::Metric act(eiger::NONDETERMINISTIC, "activity " + name, "activity factor");
		act.commit();

    eiger::NondeterministicMetric dm(exec.getID(), dyncount.getID(), fu_it->second.dynamic_count);
		dm.commit();
		eiger::DeterministicMetric sm(dataset.getID(), statcount.getID(), fu_it->second.static_count);
		sm.commit();
		eiger::NondeterministicMetric am(exec.getID(), act.getID(), fu_it->second.activity);
		am.commit();
		

  }

	// 8) disconnect from eiger
  eiger::Disconnect();
	trialCounter++;


}

//////////////////////////////////////////////////////////////////////////////////////////////////

