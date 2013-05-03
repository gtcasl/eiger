/*!
	\file ParallelismTraceGenerator.h
	
	\author Gregory Diamos
	\date Wednesday April 15, 2009
	
	\brief The header file for the ParallelismTraceGenerator class
*/

#ifndef PARALLELISM_TRACE_GENERATOR_H_INCLUDED
#define PARALLELISM_TRACE_GENERATOR_H_INCLUDED

#include <ocelot/trace/interface/TraceGenerator.h>
#include <ocelot-trace/interface/TraceConfiguration.h>

namespace trace
{
	
	class ParallelismTraceAnalyzer;
	
	/*!
		\brief A class for creating a trace file containing activity factor,
			and number of dynamic instructions for each CTA in a kernel.
			
		This class should create a database file as well as a trace
		file for each kernel.  The database should contain references
		and statistics about all of the trace files.
	*/
	class ParallelismTraceGenerator : public TraceGenerator
	{
		
		private:
		
			friend class ParallelismTraceAnalyzer;
			
			/*!
				\brief Header for a kernel trace
			*/
			class Header
			{
				public:
					TraceFormat format; //! The trace format stored
					unsigned int dimensions; //! The total number of CTAs
					unsigned int threads; //! Threads in the CTA
			};
			
			/*!
				\brief Statistics associated with a specific CTA
			*/
			class Event
			{
				public:
					unsigned int ctaid; //! The specific CTA
					long long unsigned int instructions; //! Instruction count
					double activity; //! Average number of on threads
			};
		
		private:
			
			/*!
				\brief Counter for creating unique file names.
			*/
			static unsigned int _counter;
		
		private:
		
			/*!
				\brief Header for the current kernel
			*/
			Header _header;
			
			/*!
				\brief The current event
			*/
			Event _event;
			std::vector<Event> _events;
					
		public:
		
			/*!
				\brief Initialize the file pointer to 0
			*/
			ParallelismTraceGenerator(TraceConfiguration *config);
			
			/*!
				\brief Finalize the trace and dump the results to disk.
				
				Add a databse entry for the trace as well.
			*/
			~ParallelismTraceGenerator();
		
			/*!
				\brief Initializes the trace generator when a new kernel is 
					about to be launched.
				\param kernel The kernel used to initialize the generator
			*/
			void initialize( const executive::ExecutableKernel& kernel );

			/*!
				\brief Called whenever an event takes place.

				\param even The TraceEvent that just occurred
				Note, the const reference 'event' is only valid until event() 
				returns
			*/
			void event( const TraceEvent& event );
			
			void finish();

		public:

			std::string _kernelName;
			std::string _kernelModule;
			std::string _machineName;

			int trialCounter;

		private:
			TraceConfiguration *_config;
			int activeCount;
			int instCount;
	
	};
	
}

#endif

