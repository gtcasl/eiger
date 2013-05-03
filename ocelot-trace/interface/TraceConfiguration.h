/*!
	\file TraceConfiguration.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\date Sept 7, 2011
	\brief defines the TraceConfiguration class for libEigerOcelotTrace - configures
		and adds trace generators
*/

#ifndef EIGER_OCELOTTRACE_TRACECONFIGURATION_H_INCLUDED
#define EIGER_OCELOTTRACE_TRACECONFIGURATION_H_INCLUDED

#include <hydrazine/implementation/json.h>

namespace trace {
	class InstructionTraceGenerator;
	class KernelDimensionsGenerator;
	class KernelParametersGenerator;
	class KernelTimingGenerator;
	class MachineAttributesGenerator;
	class MemoryEfficiencyGenerator;
	class BranchTraceGenerator;
	class ParallelismTraceGenerator;

	class TraceConfiguration {
	public:
	
		//! \brief configuration object for InstructionTrace
		class ConfigureInstructionTrace {
		public:
			ConfigureInstructionTrace();
			void initialize(hydrazine::json::Visitor &configure);
			
		public:
			bool enabled;
		};
	
	public:
		TraceConfiguration();
		virtual ~TraceConfiguration();
		
	public:
		static TraceConfiguration *Singleton;
		
	public:
		ConfigureInstructionTrace configureInstructionTrace;

		bool instruction;
		bool kernelDimensions;
		bool kernelParameters;
		bool machineAttributes;
		bool memoryEfficiency;
		bool branch;
		bool parallelism;
		bool timing;

	private:
		trace::InstructionTraceGenerator *_instructionTraceGenerator;
		trace::KernelDimensionsGenerator *_kernelDimensionsGenerator;
		trace::KernelParametersGenerator *_kernelParametersGenerator;
		trace::KernelTimingGenerator *_kernelTimingGenerator;
		trace::MachineAttributesGenerator *_machineAttributesGenerator;
		trace::MemoryEfficiencyGenerator *_memoryEfficiencyGenerator;
		trace::BranchTraceGenerator *_branchTraceGenerator;
		trace::ParallelismTraceGenerator *_parallelismTraceGenerator;

	public:
		std::string dbLocation;
		std::string dbName;
		std::string dbUsername;
		std::string dbPassword;

		std::string dataCollection;
		std::string dataCollectionDesc;

		std::string datasetName;
		std::string datasetDesc;
		std::string datasetURL;

		int startTrial;

	};
}

#endif

