#
#	\file SConstruct
#	\author Eric Anger <eanger@gatech.edu>, Andrew Kerr <arkerr@gatech.edu>
# \date September 20, 2011; Nov 28, 2011
#	\brief SConstruct build script for Eiger C++ interface library and trace generators
#
import os

gpuocelotLocation	= '/home/eric/gpuocelot/install/include'
mysqlppLocation		= '/usr/include/mysql++'
mysqlLocation		= '/usr/include/mysql'

cppPath = ['.', mysqlLocation, mysqlppLocation, gpuocelotLocation]
cppFlags = ['-std=c++0x',]
libPath = ['.']

vars = Variables()
vars.Add(PathVariable('PREFIX', 'Directory to install under', '/usr/local', PathVariable.PathIsDir))

env = Environment(variables=vars, CPPPATH=cppPath , CXXFLAGS=cppFlags, LIBPATH=libPath)
env.VariantDir('.build_dir/ocelot-trace', 'ocelot-trace', duplicate=0)
env.VariantDir('.build_dir/api', 'api', duplicate=0)
Help(vars.GenerateHelpText(env))


####################################################################################################
#
# Eiger Database interface
#


# sources
eigerInterfaceSources = [".build_dir/api/eiger.cpp",]

# headers
eigerInterfaceIncludes = [".build_dir/api/eiger.h",]

libeigerInterface = env.SharedLibrary(target = ".build_dir/eigerInterface", source = eigerInterfaceSources, \
	includes = eigerInterfaceIncludes)
env.Alias('interface','libeigerInterface.so')


####################################################################################################
#
# fakeeiger and its loader
#
fakeeigerInterfaceSources = [".build_dir/api/fakeeiger.cpp",]
fakeeigerInterfaceIncludes = [".build_dir/api/fakeeiger.h",]
libfakeeigerInterface = env.SharedLibrary(target = ".build_dir/fakeeigerInterface", source = fakeeigerInterfaceSources, \
	includes = fakeeigerInterfaceIncludes)
env.Alias('fakeinterface','libfakeeigerInterface.so')

feloader= env.Program(".build_dir/api/feloader.cpp",LIBS=[libeigerInterface,'mysqlpp'], )

####################################################################################################
#
# Ocelot trace generators
# 


#
# source files
#
ocelotTraceSources = [".build_dir/ocelot-trace/implementation/" + x for x in
	["InstructionTraceGenerator.cpp",
		"BranchTraceGenerator.cpp",
		"KernelDimensionsGenerator.cpp",
		"KernelParametersGenerator.cpp",
		"KernelTimingGenerator.cpp",
		"MachineAttributesGenerator.cpp",
		"ParallelismTraceGenerator.cpp",
		"MemoryEfficiencyGenerator.cpp",
		"TraceConfiguration.cpp",]]

#
# header files
#
ocelotTraceIncludes = [".build_dir/ocelot-trace/interface/" + x for x in
	["InstructionTraceGenerator.h",
		"KernelDimensionsGenerator.h",
		"KernelParametersGenerator.h",
		"KernelTimingGenerator.h",
		"MachineAttributesGenerator.h",
		"MemoryEfficiencyGenerator.h",
		"BranchTraceGenerator.h",
		"ParallelismTraceGenerator.h",
		"TraceConfiguration.h"]]

libocelotTrace = env.SharedLibrary(target = ".build_dir/eigerOcelotTrace", source = ocelotTraceSources, \
	includes = ocelotTraceIncludes, LIBS = [libeigerInterface,])
env.Alias('trace','libeigerOcelotTrace.so')


#
# installing
#
env.Alias("install", env.Install(os.path.join(env['PREFIX'], "lib"), libocelotTrace))
env.Alias("install", env.Install(os.path.join(env['PREFIX'], "lib"), libeigerInterface))
env.Alias("install", env.Install(os.path.join(env['PREFIX'], "include/eiger"), eigerInterfaceIncludes))
env.Alias("install", env.Install(os.path.join(env['PREFIX'], "bin"), feloader))
env.Alias("install", env.Install(os.path.join(env['PREFIX'], "lib"), libfakeeigerInterface))
env.Alias("install", env.Install(os.path.join(env['PREFIX'], "include/eiger"), fakeeigerInterfaceIncludes))

#individual installs
env.Alias("install-ocelot-only", env.Install(os.path.join(env['PREFIX'], "lib"), libocelotTrace))

env.Alias("install-eiger-only", env.Install(os.path.join(env['PREFIX'], "lib"), libeigerInterface))
env.Alias("install-eiger-only", env.Install(os.path.join(env['PREFIX'], "include/eiger"), eigerInterfaceIncludes))
env.Alias("install-eiger-only", env.Install(os.path.join(env['PREFIX'], "bin"), feloader))
env.Alias("install-eiger-only", env.Install(os.path.join(env['PREFIX'], "lib"), libfakeeigerInterface))
env.Alias("install-eiger-only", env.Install(os.path.join(env['PREFIX'], "include/eiger"), fakeeigerInterfaceIncludes))
####################################################################################################
