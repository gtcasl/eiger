/*!
	\file KernelTimingGenerator.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\date Jan 6, 2010
	\brief records timing of kernels
*/

#ifndef OCELOT_KERNELTIMING_TRACE_GENERATOR_H_INCLUDED
#define OCELOT_KERNELTIMING_TRACE_GENERATOR_H_INCLUDED

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
	class KernelTimingGenerator : public TraceGenerator {
	public:

		/*!
			default constructor
		*/
		KernelTimingGenerator(TraceConfiguration *config);

		/*!
			\brief destructs instruction trace generator
		*/
		virtual ~KernelTimingGenerator();

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
			
		std::string _kernelName;
		std::string _kernelModule;
		std::string machineName;

		static unsigned int _counter;	

		timespec tstart;
		timespec tend;

	private:
		trace::TraceConfiguration *_config;

		double timespec_diff(struct timespec start, struct timespec end);

	};
}

#endif

