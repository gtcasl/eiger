/*!
	\file InstructionTraceGenerator.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\date date Jan 5, 2010
	\brief captures static and dynamic instruction counts
*/

#ifndef OCELOT_INSTRUCTION_TRACE_GENERATOR_H_INCLUDED
#define OCELOT_INSTRUCTION_TRACE_GENERATOR_H_INCLUDED

// C++ stdlib includes
#include <unordered_map>

// Ocelot includes
#include <ocelot/ir/interface/Dim3.h>
#include <ocelot/ir/interface/PTXInstruction.h>
#include <ocelot/trace/interface/TraceGenerator.h>
#include <ocelot-trace/interface/TraceConfiguration.h>

//////////////////////////////////////////////////////////////////////////////////////////////////

namespace trace {

	class InstructionTraceGenerator : public TraceGenerator {
	public:

		/*!
			Indicates which functional unit a particular instruction utilizes
		*/
		enum FunctionalUnit {
			Integer_arithmetic,	//! integer arithmetic
			Integer_logical,		//! itneger logical
			Integer_comparison,	//! comparison
			Float_single,				//! floating-point single-precision
			Float_double,				//! floating-point, double-precision
			Float_comparison,		//! floating-point comparison
			Memory_offchip,			//! off-chip: {global, local}
			Memory_onchip,			//! cached or scratchpad: {texture, shared, constant}
			Control,						//! control-flow instructions
			Parallelism,				//! parallelism: sync, reduction, vote
			Special,						//! transcendental and special functions
			Other,							//! not categorized
			FunctionalUnit_invalid		//! invalid
		};
		
		static const char * toString(FunctionalUnit fu);

		/*!
			\brief gets the functional unit associated with a particular PTXInstruction
			\param instr PTXInstruction instance
			\return functional unit
		*/
		static FunctionalUnit getFunctionalUnit(const ir::PTXInstruction &instr);

	public:

		/*!
			\brief 
		*/
		class InstructionCounter {
		public:
			InstructionCounter();

			void count(const ir::PTXInstruction &instr, size_t active);

			InstructionCounter & operator+=( const InstructionCounter &counter);

		public:

			size_t dynamic_count;

			size_t static_count;
			
			double activity;
		};

		/*!
			header for InstructionTraceGenerator
		*/
		class Header {
		public:
			Header();
			
		public:
		
			TraceFormat format;

			size_t total_dynamic;
			
			size_t total_static;
			
			ir::Dim3 blockDim;
			
			ir::Dim3 gridDim;
		};

		/*!
			\brief maps a PTXInstruction::Opcode onto an instruction counter
		*/
		//typedef std::unordered_map<int, InstructionCounter > OpcodeCountMap;

		/*!
			\brief maps a functional unit onto an InstructionCounter (only count per FU, not per inst)
		*/
		typedef std::unordered_map<int, InstructionCounter > FunctionalUnitCountMap;

	public:

		/*!
			default constructor
		*/
		InstructionTraceGenerator(TraceConfiguration *config);

		/*!
			\brief destructs instruction trace generator
		*/
		virtual ~InstructionTraceGenerator();

		/*!
			\brief called when a traced kernel is launched to retrieve some 
				parameters from the kernel
		*/
		virtual void initialize(const executive::ExecutableKernel& kernel);

		/*!
			\brief Called whenever an event takes place.

			Note, the const reference 'event' is only valid until event() 
			returns
		*/
		virtual void event(const TraceEvent & event);
		
		/*! 
			\brief Called when a kernel is finished. There will be no more 
				events for this kernel.
		*/
		virtual void finish();
	
	public:
	
		Header _header;

		std::string _kernelName;
		std::string _kernelModule;
		std::string _machineName;

		int trialCounter;
		
		FunctionalUnitCountMap instructionCounter;
		
		size_t threadCount;
		
	private:
	
		static unsigned int _counter;
		trace::TraceConfiguration *_config;
		
	};
}

#endif

