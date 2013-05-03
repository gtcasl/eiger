# \file DataCube.py
# \author Andrew Kerr <arkerr@gatech.edu>
# \date 
# \brief models a data collection
#

import numpy as np

#################################################################################################
#
#
class DataCube:

	def __init__(self, metrics, profile, experimental, applications):
		"""
		"""
		self.profileMetrics, self.machineMetrics = metrics
		self.profileCube, self.profileMachine, self.profilePerformance, self.machineSelector = profile
		self.experimentalCube, self.experimentalMachine, self.experimentalPerformance = experimental
		self.applications = applications
	
	def selectMachine(self, machineProjected):
		machineColumns = np.zeros((self.profileCube.shape[0], machineProjected.shape[1]))
		for i in range(self.profileCube.shape[0]):
			machineColumns[i,:] = machineProjected[self.machineSelector[i],:]
		return machineColumns

##################################################################################################
#
#

	

class DataCollection:
	"""
	DataCollection represents the contents of a run consisting of
	
	profile - an N-by-M matrix (N: trials, M: metrics) of recorded metrics
	machine - a U-by-V matrix (U: each machine, V: each machine metric) describing the *collection* of machines
	performance - an N-by-1 vector of performance results
	applications - an array of application names
	trials - an N-by-3 matrix selecting (machine, application name, identifier)
		machine - indicates a particular row in self.machine
		application name - indicates a particular row in self.applications
		identifier - indicates a particular row in self.identifier
	identifier - an array uniquely identifying a particular trial
	"""
	
	#
	def __init__(self, profile = None, machine = None, performance = None, applications = None, trials = None, identifier = None):
		self.profile = profile
		self.machine = machine
		self.performance = performance
		self.applications = applications
		self.identifier = identifier
		self.trials = trials
		self.description = ""
		self.name = ""
		self.ID = 0

	def printDimensions(self):
		print "profile = ", self.profile.shape
		print "machine = ", self.machine.shape
		print "performance = ", self.performance.shape
		print "applications = ", len(self.applications)
		if self.identifier != None:
			print "identifier = ", self.identifier.shape
		print "trials = ", self.trials.shape

	#
	def getProfile(self):
		"""
		Getter for a profile of runs
		"""
		return self.profile
	
	#
	def _select(self, matrix, selector):
		"""
		Selection utility function denormalizes data.
		"""
		N = len(selector)
		M = matrix.shape[1]
		result = np.zeros((N,M))
		for i, s in enumerate(selector):
			result[i,:] = matrix[s,:]
		return result
	
	#
	def selectMachine(self, projection = None):
		"""
		If specified, projects the machine matrix via V-by-P matrix returned by PCA. 
		Uses self.trials[:,0] to construct N-by-P matrix containing machine parameters
		"""
		M = self.machine * projection if projection != None else self.machine		
		return self._select(M, list(self.trials[:,0].flat))
		
