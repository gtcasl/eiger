/*!
	\file KernelParametersGenerator.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\date Jan 6, 2010
	\brief records parameters of launched kernels - note: this assumes trace generators are only
		applied to EmulatedKernel instances
*/

#ifndef OCELOT_KERNELPAR_TRACE_GENERATOR_H_INCLUDED
#define OCELOT_KERNELPAR_TRACE_GENERATOR_H_INCLUDED

// Ocelot includes
#include <ocelot/ir/interface/Dim3.h>
#include <ocelot/ir/interface/PTXInstruction.h>
#include <ocelot/trace/interface/TraceGenerator.h>
#include <ocelot-trace/interface/TraceConfiguration.h>

//////////////////////////////////////////////////////////////////////////////////////////////////

namespace trace {

	/*!
		\brief kernel dimensions trace generator
	*/
	class KernelParametersGenerator : public TraceGenerator {
	public:
	
		/*!
			header for InstructionTraceGenerator
		*/
		class Header {
		public:
			Header();
			
		public:
		
			TraceFormat format;

			ir::Dim3 block;
			
			ir::Dim3 grid;
			
		};	
	
	public:

		/*!
			default constructor
		*/
		KernelParametersGenerator(TraceConfiguration *config);

		/*!
			\brief destructs instruction trace generator
		*/
		virtual ~KernelParametersGenerator();

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

		static unsigned int _counter;	

	private:
		trace::TraceConfiguration *_config;

	};
}

#endif

