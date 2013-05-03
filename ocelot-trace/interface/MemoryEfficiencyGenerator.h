/*!
	\file MemoryEfficiencyGenerator.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\date October 28, 2011
	\brief measures memory efficiency and intensity
*/

#ifndef OCELOT_MEMORY_EFFICIENCY_GENERATOR_H_INCLUDED
#define OCELOT_MEMORY_EFFICIENCY_GENERATOR_H_INCLUDED

// C++ stdlib includes
#include <unordered_map>

// Ocelot includes
#include <ocelot/ir/interface/Dim3.h>
#include <ocelot/ir/interface/PTXInstruction.h>
#include <ocelot/trace/interface/TraceGenerator.h>
#include <ocelot-trace/interface/TraceConfiguration.h>

// Boost headers
#include <boost/serialization/split_free.hpp>

//////////////////////////////////////////////////////////////////////////////////////////////////

namespace executive {
	class EmulatedKernel;
}

namespace trace {

	class MemoryEfficiencyGenerator : public TraceGenerator {
	public:
	
		class AccessCounter {
		public:
			AccessCounter(size_t ts = 64): transactionSize(ts), transactions(0), accesses(0), bytes(0), 
				bytesDemanded(0) { }
			
			void access(int warpSize, const trace::TraceEvent &event);
			
			AccessCounter &operator+=(const AccessCounter &counter) {
				transactions += counter.transactions;
				accesses += counter.accesses;
				bytes += counter.bytes;
				bytesDemanded += counter.bytesDemanded;
				return *this;
			}
			
		public:
		
			size_t transactionSize;
			
		public:
			
			//! \brief number of transactions needed to satisfy these transactions
			size_t transactions;
			
			//! \brief number of dynamic warp-level memory instructions
			size_t accesses;
						
			//! \brief number of bytes transferred
			size_t bytes;
			
			//! \brief 
			size_t bytesDemanded;
		};
		
		//! \brief maps address space onto access counters
		typedef std::map< ir::PTXInstruction::AddressSpace, AccessCounter > AccessCounterMap;

	public:
	
		/*!
			default constructor
		*/
		MemoryEfficiencyGenerator(TraceConfiguration *config);

		/*!
			\brief destructs instruction trace generator
		*/
		virtual ~MemoryEfficiencyGenerator();

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
		
	private:
	
		AccessCounterMap accessCounterMap;
		
		int warpSize;
		
		size_t warpInstructions;

	public:
		trace::TraceConfiguration *_config;
		std::string machineName;
		std::string _kernelName;
		std::string _kernelModule;
		int trialCounter;
		
	private:
	
		void clear();
		
	};
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////
#endif

