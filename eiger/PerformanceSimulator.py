#
# \file PerformanceSimulator.py
#
# \author Andrew Kerr <arkerr@gatech.edu>
# \date August 12, 2011
# \brief simulates performance of an application thorugh a simple analytical model plus noise
#


import numpy as np
import DataCube as dc

class PerformanceSimulator:
	
	def __init__(self):
		""" """
		pass
		
	def simulate(self, descriptor=[]):
		"""
		return a DataCollection() of the run
		"""
		
		applicationMetrics = ("N", "M", "FlOps", "MemOps", "Runtime")
		machineMetrics = ("Clock", "IPC", "MemBandwidth")
		
		machine = [1.0e6, 2.0, 1000]
		def appResult(m, n, machine, runtime = False):
			rt = m * n * n / machine[0] / machine[1]
			return [rt,] if runtime else [m, n, m * n * n, m * n] 
		
		tests = ((64, 64), (64, 128), (128, 128), (256, 128), (256,256), (256, 512), (512, 256), (512, 512))

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
		profile = np.matrix([appResult(r[0], r[1], machine) for r in tests])
		performance = np.matrix([appResult(r[0], r[1], machine, True) for r in tests])
		machine = np.matrix([machine, machine])
		applications = ["MatrixMultiply",]
		trials = np.matrix([[0, 0, 0] for r in tests])
		identifier = np.matrix([ [0,0,0] ])
		
		print "Profile =\n", profile
		print "Machine =\n", machine
		print "Performance =\n", performance
		print "Applications =\n", applications
		print "Trials =\n", trials
		
		return dc.DataCollection(profile, machine, performance, applications, trials, identifier)
		

if __name__ == "__main__":
	def Test():
		
		import PerformanceModel
		
		simulator = PerformanceSimulator()
		
		trainingData = simulator.simulate()
		
		performance = PerformanceModel.PerformanceModel(trainingData)
		performance.selectModel()
		
		print "Orders = ", performance.indices
		print "Beta = ", performance.model.beta
		
		from pylab import bar, plot, show, xlabel, ylabel, xticks, legend, figure, title
		
		trainingPrediction = performance.predict(trainingData)
		figure()
		xx = range(0, trainingPrediction.shape[0])
		rects0 = bar([x for x in xx], trainingPrediction.flat, 0.4, color='blue')
		rects1 = bar([x + 0.4 for x in xx], trainingData.performance.flat, 0.4, color='black')
		xticks([x + 0.4 for x in xx], trainingData.applications, rotation='90')
		legend((rects0[0], rects1[0]), ('Predicted', '8800GTX'), loc='upper right')
		title('Performance of training applications versus prediction')	

		"""		
		testData = experimentalData
		prediction = performance.predict(testData)
		
		figure()
		xx = range(0, prediction.shape[0])
		rects0 = bar([x for x in xx], prediction.flat, 0.4, color='blue')
		rects1 = bar([x + 0.4 for x in xx], testData.performance.flat, 0.4, color='black')
		xticks([x + 0.4 for x in xx], testData.applications, rotation='90')
		#legend((rects0[0], rects1[0]), ('Predicted', '8800GTX'), loc='upper right')
		title('Predicted performance for new applications')
		"""
		show()
		pass
	Test()

